/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.1
 * Date               : 2025/07/06
 * Description        : DRV2605 real-time playback demo via I2C.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#include "debug.h"
#include "drv2605.h"

#define I2C_BUS_SPEED         100000
#define VIBE_FREQ_FAST_HZ     150
#define VIBE_FREQ_SLOW_HZ     200
#define VIBE_VOLTAGE_MAX_MV   5000
#define CONT_PULSE_COUNT      4
#define CONT_ON_TIME_MS       400
#define CONT_OFF_TIME_MS      300

typedef struct {
    u16 frequencyHz;
    u16 voltageMv;
} DRV2605_FreqVoltageTone;

static const DRV2605_FreqVoltageTone toneSequence[] = {
    { VIBE_FREQ_FAST_HZ, VIBE_VOLTAGE_MAX_MV },
    { VIBE_FREQ_SLOW_HZ, VIBE_VOLTAGE_MAX_MV }
};

static const DRV2605_FreqAmpTiming freqAmpTiming = {
    .burstDurationMs = 500,
    .pauseDurationMs = 0
};

static const DRV2605_ContinuousConfig continuousConfig = {
    .useLRA = ENABLE,
    .driveTime = 0x20,
    .strength = 0x50,
    .libraryId = DRV2605_LIBRARY_LRA
};

static const DRV2605_Effect romEffects[] = {
    DRV2605_EFFECT_STRONG_CLICK_100,
    DRV2605_EFFECT_SOFT_BUMP_60,
    DRV2605_EFFECT_BUZZ_3_60
};

#define TONE_COUNT      (sizeof(toneSequence) / sizeof(toneSequence[0]))
#define ROM_EFFECT_COUNT (sizeof(romEffects) / sizeof(romEffects[0]))

static void Demo_FreqVoltage(void);
static void Demo_ContinuousPulses(void);
static void Demo_RomWaveforms(void);

/*********************************************************************
 * @fn      IIC_Init
 *
 * @brief   Initializes the IIC peripheral.
 *
 * @return  none
 */
void IIC_Init (u32 bound, u16 address) {
     GPIO_InitTypeDef GPIO_InitStructure={0};
    I2C_InitTypeDef I2C_InitTSturcture={0};

    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE );
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_I2C1, ENABLE );

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_Init( GPIOC, &GPIO_InitStructure );

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_Init( GPIOC, &GPIO_InitStructure );
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_Init( GPIOC, &GPIO_InitStructure );
    GPIO_SetBits(GPIOC,GPIO_Pin_3);

    I2C_InitTSturcture.I2C_ClockSpeed = bound;
    I2C_InitTSturcture.I2C_Mode = I2C_Mode_I2C;
    I2C_InitTSturcture.I2C_DutyCycle = I2C_DutyCycle_16_9;
    I2C_InitTSturcture.I2C_OwnAddress1 = address;
    I2C_InitTSturcture.I2C_Ack = I2C_Ack_Enable;
    I2C_InitTSturcture.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init( I2C1, &I2C_InitTSturcture );

    I2C_Cmd( I2C1, ENABLE );
}

/*********************************************************************
 * @fn      main
 *
 * @brief   Application entry.
 *
 * @return  none
 */
int main (void) {
    SystemCoreClockUpdate();
    Delay_Init();
    USART_Printf_Init (460800);
    printf ("SystemClk:%d\r\n", SystemCoreClock);
    printf ("ChipID:%08x\r\n", DBGMCU_GetCHIPID());
    printf ("DRV2605 low-level freq/amplitude demo\r\n");

    IIC_Init (I2C_BUS_SPEED, 0x00);

    while (1) {
        Demo_FreqVoltage();
        Demo_ContinuousPulses();
        Demo_RomWaveforms();
    }
}

static void Demo_FreqVoltage(void) {
    printf ("\r\n[Demo] Frequency + Voltage sweep\r\n");
    DRV2605_SetFreqAmpVoltageRange (VIBE_VOLTAGE_MAX_MV);
    DRV2605_SetFreqAmpTiming (&freqAmpTiming);

    if (DRV2605_PrepareFreqAmpRealtime() != READY) {
        printf ("Realtime prepare failed\r\n");
        return;
    }

    for (u8 i = 0; i < TONE_COUNT; i++) {
        const DRV2605_FreqVoltageTone *tone = &toneSequence[i];
        printf ("Tone %u -> %u Hz / %u mV\r\n", i, tone->frequencyHz, tone->voltageMv);
        if (DRV2605_PlayFreqVoltage (tone->frequencyHz, tone->voltageMv) != READY) {
            printf ("Freq/Voltage drive failed\r\n");
        }
        Delay_Ms (200);
    }
}

static void Demo_ContinuousPulses(void) {
    printf ("\r\n[Demo] Continuous pulses\r\n");

    if (DRV2605_ConfigureContinuous (&continuousConfig) != READY) {
        printf ("Continuous config failed\r\n");
        return;
    }

    for (u8 pulse = 0; pulse < CONT_PULSE_COUNT; pulse++) {
        if (DRV2605_StartContinuous() != READY) {
            printf ("Start continuous failed\r\n");
            return;
        }
        Delay_Ms (CONT_ON_TIME_MS);
        if (DRV2605_StopContinuous() != READY) {
            printf ("Stop continuous failed\r\n");
            return;
        }
        Delay_Ms (CONT_OFF_TIME_MS);
    }
}

static void Demo_RomWaveforms(void) {
    printf ("\r\n[Demo] ROM sequence playback\r\n");

    if (DRV2605_SetMode (DRV2605_MODE_INT_TRIG) != READY) {
        printf ("Set mode failed\r\n");
        return;
    }
    Delay_Ms (5);
    if (DRV2605_SelectLRA() != READY) {
        printf ("Select LRA failed\r\n");
        return;
    }
    if (DRV2605_SetLibrary (DRV2605_LIBRARY_LRA) != READY) {
        printf ("Set library failed\r\n");
        return;
    }

    for (u8 idx = 0; idx < ROM_EFFECT_COUNT; idx++) {
        if (DRV2605_SetWaveform (0, romEffects[idx]) != READY) {
            printf ("Set waveform failed\r\n");
            return;
        }
        if (DRV2605_ClearWaveforms() != READY) {
            printf ("Clear waveform failed\r\n");
            return;
        }
        if (DRV2605_Start() != READY) {
            printf ("Start playback failed\r\n");
            return;
        }
        Delay_Ms (600);
        if (DRV2605_Stop() != READY) {
            printf ("Stop playback failed\r\n");
            return;
        }
        Delay_Ms (300);
    }
}