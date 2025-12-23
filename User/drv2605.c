/******************************************************************************
 * 文件名   : drv2605.c
 * 描述     : DRV2605 震动驱动器操作库（仅包含写入/配置功能）。
 ******************************************************************************/
#include "drv2605.h"

#define I2C_TIMEOUT_CYC      0x4FFF
#define DRV2605_RATEDV_STEP_UV  21330UL
#define DRV2605_CLAMPV_STEP_UV  5600UL

static ErrorStatus DRV2605_I2C_WriteBytes(const u8 *data, u8 length);
static ErrorStatus DRV2605_I2C_ReadRegisters(DRV2605_Register reg, u8 *buffer, u8 length);
static ErrorStatus DRV2605_I2C_Start(u8 direction);
static ErrorStatus DRV2605_I2C_WaitEvent(uint32_t event, uint32_t timeout);
static ErrorStatus DRV2605_I2C_WaitIdle(void);
static void DRV2605_I2C_ClearErrors(void);
static ErrorStatus DRV2605_WaitGoClear(uint32_t timeoutMs);
static u8 s_continuousStrength = 0;
static u8 s_continuousConfigured = 0;
static FunctionalState s_freqAmpReady = DISABLE;
static u16 s_freqAmpBurstMs = 800;
static u16 s_freqAmpPauseMs = 300;
static u16 s_freqAmpVoltageMaxMv = 5000;

/* ========================= 公共 API 实现 ========================= */

/******************************************************************************
 * @brief  按照推荐值完成一次基础初始化（供 LRA 器件使用）。
 ******************************************************************************/
ErrorStatus DRV2605_InitDefaults(void) {
    if(DRV2605_SetMode(DRV2605_MODE_INT_TRIG) == NoREADY) return NoREADY;
    Delay_Ms(5);
    if(DRV2605_SelectLRA() == NoREADY) return NoREADY;
    if(DRV2605_SetLibrary(DRV2605_LIBRARY_LRA) == NoREADY) return NoREADY;
    if(DRV2605_SetWaveform(0, DRV2605_EFFECT_STRONG_CLICK_100) == NoREADY) return NoREADY;
    if(DRV2605_ClearWaveforms() == NoREADY) return NoREADY;
    if(DRV2605_SetOverdriveClamp(0x00) == NoREADY) return NoREADY;
    if(DRV2605_SetSustainLevel(0x00, 0x00) == NoREADY) return NoREADY;
    if(DRV2605_SetBrakeLevel(0x00) == NoREADY) return NoREADY;
    if(DRV2605_SetAudioMax(0x64) == NoREADY) return NoREADY;
    return DRV2605_SetMode(DRV2605_MODE_REALTIME);
}

/******************************************************************************
 * @brief  写单个寄存器（通用入口）。
 ******************************************************************************/
ErrorStatus DRV2605_WriteRegister(DRV2605_Register reg, u8 value) {
    u8 payload[2] = { (u8)reg, value };
    return DRV2605_I2C_WriteBytes(payload, sizeof(payload));
}

/******************************************************************************
 * @brief  读取单个寄存器（通用入口）。
 ******************************************************************************/
ErrorStatus DRV2605_ReadRegister(DRV2605_Register reg, u8 *value) {
    if(value == NULL) {
        return NoREADY;
    }
    return DRV2605_I2C_ReadRegisters(reg, value, 1);
}

/******************************************************************************
 * @brief  获取 STATUS 寄存器。
 ******************************************************************************/
ErrorStatus DRV2605_GetStatus(u8 *status) {
    return DRV2605_ReadRegister(DRV2605_REG_STATUS, status);
}

/******************************************************************************
 * @brief  设置模式寄存器。
 ******************************************************************************/
ErrorStatus DRV2605_SetMode(DRV2605_Mode mode) {
    return DRV2605_WriteRegister(DRV2605_REG_MODE, (u8)mode);
}

/******************************************************************************
 * @brief  读取模式寄存器。
 ******************************************************************************/
ErrorStatus DRV2605_GetMode(DRV2605_Mode *mode) {
    u8 value = 0;
    if(mode == NULL) {
        return NoREADY;
    }
    if(DRV2605_ReadRegister(DRV2605_REG_MODE, &value) == NoREADY) {
        return NoREADY;
    }
    *mode = (DRV2605_Mode)(value & 0x07);
    return READY;
}

/******************************************************************************
 * @brief  选择内部波形库（0~7）。
 ******************************************************************************/
ErrorStatus DRV2605_SetLibrary(DRV2605_Library libraryId) {
    return DRV2605_WriteRegister(DRV2605_REG_LIBRARY, ((u8)libraryId) & 0x07);
}

/******************************************************************************
 * @brief  读取库选择寄存器。
 ******************************************************************************/
ErrorStatus DRV2605_GetLibrary(DRV2605_Library *libraryId) {
    u8 value = 0;
    if(libraryId == NULL) {
        return NoREADY;
    }
    if(DRV2605_ReadRegister(DRV2605_REG_LIBRARY, &value) == NoREADY) {
        return NoREADY;
    }
    *libraryId = (DRV2605_Library)(value & 0x07);
    return READY;
}

/******************************************************************************
 * @brief  写入 Waveform Sequencer 指定槽位。
 ******************************************************************************/
ErrorStatus DRV2605_SetWaveform(u8 slot, DRV2605_Effect effectId) {
    DRV2605_Register reg = (DRV2605_Register)(DRV2605_REG_WAVESEQ1 + slot);
    if(slot > 7) {
        return NoREADY;
    }
    return DRV2605_WriteRegister(reg, (u8)effectId);
}

/******************************************************************************
 * @brief  读取 Waveform Sequencer 指定槽位。
 ******************************************************************************/
ErrorStatus DRV2605_GetWaveform(u8 slot, DRV2605_Effect *effectId) {
    DRV2605_Register reg = (DRV2605_Register)(DRV2605_REG_WAVESEQ1 + slot);
    u8 value = 0;
    if(effectId == NULL || slot > 7) {
        return NoREADY;
    }
    if(DRV2605_ReadRegister(reg, &value) == NoREADY) {
        return NoREADY;
    }
    *effectId = (DRV2605_Effect)value;
    return READY;
}

/******************************************************************************
 * @brief  将剩余槽位清零（停止后续波形）。
 ******************************************************************************/
ErrorStatus DRV2605_ClearWaveforms(void) {
    u8 zero = 0x00;
    for(u8 slot = 1; slot < 8; slot++) {
        if(DRV2605_WriteRegister((DRV2605_Register)(DRV2605_REG_WAVESEQ1 + slot), zero) == NoREADY) {
            return NoREADY;
        }
    }
    return READY;
}

/******************************************************************************
 * @brief  播放实时动作组（内部维持索引）。
 ******************************************************************************/
ErrorStatus DRV2605_RunActionGroup(DRV2605_ActionGroup *group) {
    if(group == NULL || group->frames == NULL || group->frameCount == 0) {
        return NoREADY;
    }

    if(group->currentIndex < group->frameCount) {
        const DRV2605_RtpAction *frame = &group->frames[group->currentIndex++];
        if(DRV2605_SetRealtimeValue(frame->amplitude) == NoREADY) {
            return NoREADY;
        }
        Delay_Ms(frame->holdMs);
        return READY;
    }

    ErrorStatus status = DRV2605_SetRealtimeValue(0x00);
    Delay_Ms(group->pauseMs);
    group->currentIndex = 0;
    return status;
}

/******************************************************************************
 * @brief  写入官方推荐的自动校准默认值。
 ******************************************************************************/
void DRV2605_FillAutoCalDefaults(DRV2605_AutoCalConfig *cfg) {
    if(cfg == NULL) {
        return;
    }
    cfg->ratedVoltage = 0x50;
    cfg->clampVoltage = 0x90;
    cfg->control1 = 0x20;
    cfg->control2 = 0xF5;
    cfg->control3 = 0x80;
    cfg->control4 = 0x20;
    cfg->control5 = 0x80;
    cfg->timeoutMs = 2000;
}

/******************************************************************************
 * @brief  执行一次自动校准。
 ******************************************************************************/
ErrorStatus DRV2605_RunAutoCalibration(const DRV2605_AutoCalConfig *cfg,
                                       DRV2605_AutoCalResult *result) {
    if(cfg == NULL) {
        return NoREADY;
    }

    if(DRV2605_SetMode(DRV2605_MODE_INT_TRIG) == NoREADY) {
        return NoREADY;
    }
    Delay_Ms(5);

    if(DRV2605_SelectLRA() == NoREADY) return NoREADY;
    if(DRV2605_SetLibrary(DRV2605_LIBRARY_LRA) == NoREADY) return NoREADY;
    if(DRV2605_SetRatedVoltage(cfg->ratedVoltage) == NoREADY) return NoREADY;
    if(DRV2605_SetClampVoltage(cfg->clampVoltage) == NoREADY) return NoREADY;
    if(DRV2605_SetControl1(cfg->control1) == NoREADY) return NoREADY;
    if(DRV2605_SetControl2(cfg->control2) == NoREADY) return NoREADY;
    if(DRV2605_SetControl3(cfg->control3) == NoREADY) return NoREADY;
    if(DRV2605_SetControl4(cfg->control4) == NoREADY) return NoREADY;
    if(DRV2605_SetControl5(cfg->control5) == NoREADY) return NoREADY;

    if(DRV2605_SetMode(DRV2605_MODE_AUTOCAL) == NoREADY) return NoREADY;

    if(DRV2605_Start() == NoREADY) return NoREADY;

    if(DRV2605_WaitGoClear(cfg->timeoutMs ? cfg->timeoutMs : 2000) == NoREADY) {
        DRV2605_Stop();
        return NoREADY;
    }

    DRV2605_Stop();

    if(result) {
        if(DRV2605_GetStatus(&result->status) == NoREADY) return NoREADY;
        if(DRV2605_ReadRegister(DRV2605_REG_AUTOCALCOMP, &result->compensation) == NoREADY) return NoREADY;
        if(DRV2605_ReadRegister(DRV2605_REG_AUTOCALEMP, &result->backEMF) == NoREADY) return NoREADY;
        if(DRV2605_ReadRegister(DRV2605_REG_RATEDV, &result->ratedVoltage) == NoREADY) return NoREADY;
        if(DRV2605_ReadRegister(DRV2605_REG_CLAMPV, &result->clampVoltage) == NoREADY) return NoREADY;
        result->vbatRaw = 0;
        result->lraResonance = 0;
        DRV2605_ReadVbatRaw(&result->vbatRaw);
        DRV2605_ReadLraResonance(&result->lraResonance);
    }

    return READY;
}

/******************************************************************************
 * @brief  编码/解码辅助函数。
 ******************************************************************************/
u8 DRV2605_EncodeRatedVoltageMv(u16 millivolts) {
    uint32_t uv = (uint32_t)millivolts * 1000UL;
    uint32_t code = (uv + (DRV2605_RATEDV_STEP_UV / 2)) / DRV2605_RATEDV_STEP_UV;
    if(code > 0xFF) {
        code = 0xFF;
    }
    return (u8)code;
}

u8 DRV2605_EncodeClampVoltageMv(u16 millivolts) {
    uint32_t uv = (uint32_t)millivolts * 1000UL;
    uint32_t code = (uv + (DRV2605_CLAMPV_STEP_UV / 2)) / DRV2605_CLAMPV_STEP_UV;
    if(code > 0xFF) {
        code = 0xFF;
    }
    return (u8)code;
}

u16 DRV2605_DecodeRatedVoltageMv(u8 regValue) {
    uint32_t uv = (uint32_t)regValue * DRV2605_RATEDV_STEP_UV;
    return (u16)((uv + 500UL) / 1000UL);
}

u16 DRV2605_DecodeClampVoltageMv(u8 regValue) {
    uint32_t uv = (uint32_t)regValue * DRV2605_CLAMPV_STEP_UV;
    return (u16)((uv + 500UL) / 1000UL);
}

/******************************************************************************
 * @brief  持续震动配置与控制。
 ******************************************************************************/
ErrorStatus DRV2605_ConfigureContinuous(const DRV2605_ContinuousConfig *cfg) {
    if(cfg == NULL) {
        return NoREADY;
    }

    if(cfg->useLRA == ENABLE) {
        if(DRV2605_SelectLRA() == NoREADY) {
            return NoREADY;
        }
    } else {
        if(DRV2605_SelectERM() == NoREADY) {
            return NoREADY;
        }
    }

    if(DRV2605_SetLibrary(cfg->libraryId) == NoREADY) {
        return NoREADY;
    }

    u8 control2 = 0;
    if(DRV2605_ReadRegister(DRV2605_REG_CONTROL2, &control2) == NoREADY) {
        control2 = 0xF5; /* 回退到常用默认值 */
    }
    control2 = (control2 & 0xC0) | (cfg->driveTime & 0x3F);
    if(DRV2605_SetControl2(control2) == NoREADY) {
        return NoREADY;
    }

    s_continuousStrength = (cfg->strength > 0x7F) ? 0x7F : cfg->strength;
    s_continuousConfigured = 1;
    return READY;
}

ErrorStatus DRV2605_StartContinuous(void) {
    if(!s_continuousConfigured) {
        return NoREADY;
    }
    if(DRV2605_SetMode(DRV2605_MODE_REALTIME) == NoREADY) {
        return NoREADY;
    }
    if(DRV2605_SetRealtimeValue(s_continuousStrength) == NoREADY) {
        return NoREADY;
    }
    return DRV2605_Start();
}

ErrorStatus DRV2605_StopContinuous(void) {
    if(DRV2605_SetRealtimeValue(0x00) == NoREADY) {
        return NoREADY;
    }
    return DRV2605_Stop();
}

/* -------------------- 频率/幅值直驱控制 -------------------- */

void DRV2605_SetFreqAmpTiming(const DRV2605_FreqAmpTiming *timing) {
    if(timing == NULL) {
        return;
    }
    s_freqAmpBurstMs = (timing->burstDurationMs == 0) ? 1 : timing->burstDurationMs;
    s_freqAmpPauseMs = timing->pauseDurationMs;
}

void DRV2605_SetFreqAmpVoltageRange(u16 maxMillivolts) {
    if(maxMillivolts == 0) {
        return;
    }
    s_freqAmpVoltageMaxMv = maxMillivolts;
}

ErrorStatus DRV2605_PrepareFreqAmpRealtime(void) {
    if(s_freqAmpReady == ENABLE) {
        return READY;
    }
    if(DRV2605_SetMode(DRV2605_MODE_INT_TRIG) == NoREADY) {
        return NoREADY;
    }
    Delay_Ms(5);
    if(DRV2605_SelectLRA() == NoREADY) {
        return NoREADY;
    }
    if(DRV2605_SetLibrary(DRV2605_LIBRARY_LRA) == NoREADY) {
        return NoREADY;
    }
    if(DRV2605_SetMode(DRV2605_MODE_REALTIME) == NoREADY) {
        return NoREADY;
    }
    s_freqAmpReady = ENABLE;
    return READY;
}

ErrorStatus DRV2605_PlayFreqAmp(u16 frequencyHz, u8 amplitude) {
    if(frequencyHz == 0 || amplitude == 0) {
        return NoREADY;
    }

    if(s_freqAmpReady != ENABLE) {
        if(DRV2605_PrepareFreqAmpRealtime() == NoREADY) {
            return NoREADY;
        }
    }

    u32 periodUs = 1000000UL / frequencyHz;
    if(periodUs == 0) {
        periodUs = 1;
    }
    u32 halfPeriodUs = periodUs / 2UL;
    if(halfPeriodUs == 0) {
        halfPeriodUs = 1;
    }

    u32 totalUs = (u32)s_freqAmpBurstMs * 1000UL;
    u32 cycles = totalUs / periodUs;
    if(cycles == 0) {
        cycles = 1;
    }

    u8 driveValue = (amplitude > 0x7F) ? 0x7F : amplitude;

    for(u32 cycle = 0; cycle < cycles; cycle++) {
        if(DRV2605_SetRealtimeValue(driveValue) == NoREADY) {
            return NoREADY;
        }
        Delay_Us(halfPeriodUs);
        if(DRV2605_SetRealtimeValue(0x00) == NoREADY) {
            return NoREADY;
        }
        Delay_Us(halfPeriodUs);
    }

    if(s_freqAmpPauseMs) {
        Delay_Ms(s_freqAmpPauseMs);
    }

    return READY;
}

ErrorStatus DRV2605_PlayFreqVoltage(u16 frequencyHz, u16 voltageMv) {
    if(voltageMv == 0) {
        return NoREADY;
    }
    if(s_freqAmpVoltageMaxMv == 0) {
        s_freqAmpVoltageMaxMv = 1;
    }
    u32 clamped = (voltageMv > s_freqAmpVoltageMaxMv) ? s_freqAmpVoltageMaxMv : voltageMv;
    u32 amplitude = (clamped * 0x7FUL + (s_freqAmpVoltageMaxMv / 2UL)) / s_freqAmpVoltageMaxMv;
    if(amplitude == 0) {
        amplitude = 1;
    }
    return DRV2605_PlayFreqAmp(frequencyHz, (u8)amplitude);
}

/******************************************************************************
 * @brief  启动波形序列（GO = 1）。
 ******************************************************************************/
ErrorStatus DRV2605_Start(void) {
    return DRV2605_WriteRegister(DRV2605_REG_GO, 0x01);
}

/******************************************************************************
 * @brief  停止波形序列（GO = 0）。
 ******************************************************************************/
ErrorStatus DRV2605_Stop(void) {
    return DRV2605_WriteRegister(DRV2605_REG_GO, 0x00);
}

/******************************************************************************
 * @brief  设置实时播放值（Real-Time Playback Register）。
 ******************************************************************************/
ErrorStatus DRV2605_SetRealtimeValue(u8 value) {
    return DRV2605_WriteRegister(DRV2605_REG_RTPIN, value);
}

/******************************************************************************
 * @brief  读取实时播放寄存器。
 ******************************************************************************/
ErrorStatus DRV2605_GetRealtimeValue(u8 *value) {
    return DRV2605_ReadRegister(DRV2605_REG_RTPIN, value);
}

/******************************************************************************
 * @brief  设置过驱钳位（Overdrive Clamp）。
 ******************************************************************************/
ErrorStatus DRV2605_SetOverdriveClamp(u8 value) {
    return DRV2605_WriteRegister(DRV2605_REG_OVERDRIVE, value);
}

/******************************************************************************
 * @brief  设置正/负维持力度（Sustain）。
 ******************************************************************************/
ErrorStatus DRV2605_SetSustainLevel(u8 pos, u8 neg) {
    if(DRV2605_WriteRegister(DRV2605_REG_SUSTAINPOS, pos) == NoREADY) {
        return NoREADY;
    }
    return DRV2605_WriteRegister(DRV2605_REG_SUSTAINNEG, neg);
}

/******************************************************************************
 * @brief  设置制动时间（Brake Level）。
 ******************************************************************************/
ErrorStatus DRV2605_SetBrakeLevel(u8 value) {
    return DRV2605_WriteRegister(DRV2605_REG_BREAK, value);
}

/******************************************************************************
 * @brief  设置 Audio-to-Vibe 模式的幅度上限。
 ******************************************************************************/
ErrorStatus DRV2605_SetAudioMax(u8 value) {
    return DRV2605_WriteRegister(DRV2605_REG_AUDIOMAX, value);
}

/******************************************************************************
 * @brief  设置 Audio-to-Vibe 模式的最小幅度。
 ******************************************************************************/
ErrorStatus DRV2605_SetAudioMin(u8 value) {
    return DRV2605_WriteRegister(DRV2605_REG_AUDIOMIN, value);
}

/******************************************************************************
 * @brief  写 Audio Control 寄存器。
 ******************************************************************************/
ErrorStatus DRV2605_SetAudioControl(u8 value) {
    return DRV2605_WriteRegister(DRV2605_REG_AUDIOCTRL, value);
}

/******************************************************************************
 * @brief  设置额定电压寄存器。
 ******************************************************************************/
ErrorStatus DRV2605_SetRatedVoltage(u8 value) {
    return DRV2605_WriteRegister(DRV2605_REG_RATEDV, value);
}

/******************************************************************************
 * @brief  设置钳位电压寄存器。
 ******************************************************************************/
ErrorStatus DRV2605_SetClampVoltage(u8 value) {
    return DRV2605_WriteRegister(DRV2605_REG_CLAMPV, value);
}

/******************************************************************************
 * @brief  设置自动校准补偿值（AUTOCALCOMP）。
 ******************************************************************************/
ErrorStatus DRV2605_SetAutoCalComp(u8 value) {
    return DRV2605_WriteRegister(DRV2605_REG_AUTOCALCOMP, value);
}

/******************************************************************************
 * @brief  设置自动校准阻尼/EMF（AUTOCALEMP）。
 ******************************************************************************/
ErrorStatus DRV2605_SetAutoCalBackEMF(u8 value) {
    return DRV2605_WriteRegister(DRV2605_REG_AUTOCALEMP, value);
}

/******************************************************************************
 * @brief  设置 CONTROL1~CONTROL5 寄存器。
 ******************************************************************************/
ErrorStatus DRV2605_SetControl1(u8 ctrlValue) {
    return DRV2605_WriteRegister(DRV2605_REG_CONTROL1, ctrlValue);
}

ErrorStatus DRV2605_SetControl2(u8 ctrlValue) {
    return DRV2605_WriteRegister(DRV2605_REG_CONTROL2, ctrlValue);
}

ErrorStatus DRV2605_SetControl3(u8 ctrlValue) {
    return DRV2605_WriteRegister(DRV2605_REG_CONTROL3, ctrlValue);
}

ErrorStatus DRV2605_SetControl4(u8 ctrlValue) {
    return DRV2605_WriteRegister(DRV2605_REG_CONTROL4, ctrlValue);
}

ErrorStatus DRV2605_SetControl5(u8 ctrlValue) {
    return DRV2605_WriteRegister(DRV2605_REG_CONTROL5, ctrlValue);
}

/******************************************************************************
 * @brief  读取 VBAT 原始值。
 ******************************************************************************/
ErrorStatus DRV2605_ReadVbatRaw(u8 *value) {
    return DRV2605_ReadRegister(DRV2605_REG_VBAT, value);
}

/******************************************************************************
 * @brief  读取 LRA 共振频率寄存器。
 ******************************************************************************/
ErrorStatus DRV2605_ReadLraResonance(u8 *value) {
    return DRV2605_ReadRegister(DRV2605_REG_LRARESON, value);
}

/******************************************************************************
 * @brief  选择 LRA（bit7 = 1）。
 ******************************************************************************/
ErrorStatus DRV2605_SelectLRA(void) {
    return DRV2605_WriteRegister(DRV2605_REG_FEEDBACK, 0xB6);
}

/******************************************************************************
 * @brief  选择 ERM（bit7 = 0）。
 ******************************************************************************/
ErrorStatus DRV2605_SelectERM(void) {
    return DRV2605_WriteRegister(DRV2605_REG_FEEDBACK, 0x36);
}

/* -------------------- 以下为 I2C 私有工具函数 -------------------- */

static ErrorStatus DRV2605_I2C_WriteBytes(const u8 *data, u8 length) {
    u8 sent = 0;
    if(length == 0) {
        return READY;
    }

    if(DRV2605_I2C_Start(I2C_Direction_Transmitter) == NoREADY) {
        return NoREADY;
    }

    while(sent < length) {
        uint32_t timeout = I2C_TIMEOUT_CYC;
        while(I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE) == RESET) {
            if(timeout-- == 0) {
                I2C_GenerateSTOP(I2C1, ENABLE);
                DRV2605_I2C_ClearErrors();
                return NoREADY;
            }
        }
        I2C_SendData(I2C1, data[sent++]);
        if(DRV2605_I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED, I2C_TIMEOUT_CYC) == NoREADY) {
            I2C_GenerateSTOP(I2C1, ENABLE);
            DRV2605_I2C_ClearErrors();
            return NoREADY;
        }
    }

    I2C_GenerateSTOP(I2C1, ENABLE);
    return READY;
}

static ErrorStatus DRV2605_I2C_ReadRegisters(DRV2605_Register reg, u8 *buffer, u8 length) {
    uint8_t remaining = length;
    uint8_t stopIssued = 0;
    ErrorStatus result = NoREADY;

    if(remaining == 0) {
        return READY;
    }
    if(buffer == NULL) {
        return NoREADY;
    }

    if(DRV2605_I2C_Start(I2C_Direction_Transmitter) == NoREADY) {
        return NoREADY;
    }

    uint32_t timeout = I2C_TIMEOUT_CYC;
    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE) == RESET) {
        if(timeout-- == 0) {
            goto cleanup;
        }
    }

    I2C_SendData(I2C1, (u8)reg);

    if(DRV2605_I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED, I2C_TIMEOUT_CYC) == NoREADY) {
        goto cleanup;
    }

    I2C_GenerateSTART(I2C1, ENABLE);

    if(DRV2605_I2C_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT, I2C_TIMEOUT_CYC) == NoREADY) {
        goto cleanup;
    }

    I2C_Send7bitAddress(I2C1, DRV2605_I2C_ADDRESS << 1, I2C_Direction_Receiver);

    if(DRV2605_I2C_WaitEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED, I2C_TIMEOUT_CYC) == NoREADY) {
        goto cleanup;
    }

    while(remaining) {
        if(remaining == 1) {
            I2C_AcknowledgeConfig(I2C1, DISABLE);
            I2C_GenerateSTOP(I2C1, ENABLE);
            stopIssued = 1;
        }

        if(DRV2605_I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_RECEIVED, I2C_TIMEOUT_CYC) == NoREADY) {
            goto cleanup;
        }

        *buffer++ = I2C_ReceiveData(I2C1);
        remaining--;
    }

    result = READY;

cleanup:
    I2C_AcknowledgeConfig(I2C1, ENABLE);

    if(!stopIssued) {
        I2C_GenerateSTOP(I2C1, ENABLE);
    }

    DRV2605_I2C_WaitIdle();

    if(result == NoREADY) {
        DRV2605_I2C_ClearErrors();
    }

    return result;
}

static ErrorStatus DRV2605_I2C_Start(u8 direction) {
    uint32_t targetEvent = (direction == I2C_Direction_Transmitter) ?
            I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED :
            I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED;

    if(DRV2605_I2C_WaitIdle() == NoREADY) {
        return NoREADY;
    }

    I2C_GenerateSTART(I2C1, ENABLE);

    if(DRV2605_I2C_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT, I2C_TIMEOUT_CYC) == NoREADY) {
        I2C_GenerateSTOP(I2C1, ENABLE);
        DRV2605_I2C_ClearErrors();
        return NoREADY;
    }

    I2C_Send7bitAddress(I2C1, DRV2605_I2C_ADDRESS << 1, direction);

    if(DRV2605_I2C_WaitEvent(targetEvent, I2C_TIMEOUT_CYC) == NoREADY) {
        I2C_GenerateSTOP(I2C1, ENABLE);
        DRV2605_I2C_ClearErrors();
        return NoREADY;
    }

    return READY;
}

static ErrorStatus DRV2605_I2C_WaitEvent(uint32_t event, uint32_t timeout) {
    while(!I2C_CheckEvent(I2C1, event)) {
        if(I2C_GetFlagStatus(I2C1, I2C_FLAG_AF) != RESET ||
           I2C_GetFlagStatus(I2C1, I2C_FLAG_BERR) != RESET ||
           I2C_GetFlagStatus(I2C1, I2C_FLAG_ARLO) != RESET) {
            return NoREADY;
        }
        if(timeout-- == 0) {
            return NoREADY;
        }
    }
    return READY;
}

static ErrorStatus DRV2605_I2C_WaitIdle(void) {
    uint32_t timeout = I2C_TIMEOUT_CYC;
    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY) != RESET) {
        if(timeout-- == 0) {
            return NoREADY;
        }
    }
    return READY;
}

static void DRV2605_I2C_ClearErrors(void) {
    if(I2C_GetFlagStatus(I2C1, I2C_FLAG_AF) != RESET) {
        I2C_ClearFlag(I2C1, I2C_FLAG_AF);
    }
    if(I2C_GetFlagStatus(I2C1, I2C_FLAG_BERR) != RESET) {
        I2C_ClearFlag(I2C1, I2C_FLAG_BERR);
    }
    if(I2C_GetFlagStatus(I2C1, I2C_FLAG_ARLO) != RESET) {
        I2C_ClearFlag(I2C1, I2C_FLAG_ARLO);
    }
}

static ErrorStatus DRV2605_WaitGoClear(uint32_t timeoutMs) {
    u8 go = 0;
    uint32_t remaining = (timeoutMs == 0) ? 1 : timeoutMs;

    while(remaining--) {
        if(DRV2605_ReadRegister(DRV2605_REG_GO, &go) == READY) {
            if((go & 0x01) == 0) {
                return READY;
            }
        }
        Delay_Ms(1);
    }

    return NoREADY;
}
