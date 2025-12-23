/******************************************************************************
 * 文件名   : drv2605.h
 * 描述     : DRV2605 震动马达驱动器操作接口（基于 I2C1）。
 * 版权说明 : 仅供 CH32 项目内部使用。
 ******************************************************************************/
#ifndef __DRV2605_H
#define __DRV2605_H

#include "debug.h"

#define DRV2605_I2C_ADDRESS        0x5A

/* ========================= 寄存器/资源枚举 ========================= */

/* 常用寄存器地址定义（按官方寄存器映射整理） */
typedef enum {
	DRV2605_REG_STATUS      = 0x00,  /* 状态/错误标志 */
	DRV2605_REG_MODE        = 0x01,  /* 工作模式选择 */
	DRV2605_REG_RTPIN       = 0x02,  /* 实时播放输入 */
	DRV2605_REG_LIBRARY     = 0x03,  /* 触感库编号 */
	DRV2605_REG_WAVESEQ1    = 0x04,  /* 波形序列槽 1 */
	DRV2605_REG_WAVESEQ2    = 0x05,  /* 波形序列槽 2 */
	DRV2605_REG_WAVESEQ3    = 0x06,  /* 波形序列槽 3 */
	DRV2605_REG_WAVESEQ4    = 0x07,  /* 波形序列槽 4 */
	DRV2605_REG_WAVESEQ5    = 0x08,  /* 波形序列槽 5 */
	DRV2605_REG_WAVESEQ6    = 0x09,  /* 波形序列槽 6 */
	DRV2605_REG_WAVESEQ7    = 0x0A,  /* 波形序列槽 7 */
	DRV2605_REG_WAVESEQ8    = 0x0B,  /* 波形序列槽 8 */
	DRV2605_REG_GO          = 0x0C,  /* 启动/停止控制 */
	DRV2605_REG_OVERDRIVE   = 0x0D,  /* 过驱钳位设置 */
	DRV2605_REG_SUSTAINPOS  = 0x0E,  /* 正维持时间 */
	DRV2605_REG_SUSTAINNEG  = 0x0F,  /* 负维持时间 */
	DRV2605_REG_BREAK       = 0x10,  /* 制动时间 */
	DRV2605_REG_AUDIOCTRL   = 0x11,  /* Audio-to-Vibe 控制 */
	DRV2605_REG_AUDIOMIN    = 0x12,  /* 音频模式最小幅度 */
	DRV2605_REG_AUDIOMAX    = 0x13,  /* 音频模式最大幅度 */
	DRV2605_REG_RATEDV      = 0x16,  /* 额定电压设定 */
	DRV2605_REG_CLAMPV      = 0x17,  /* 钳位电压设定 */
	DRV2605_REG_AUTOCALCOMP = 0x18,  /* 自动校准补偿值 */
	DRV2605_REG_AUTOCALEMP  = 0x19,  /* 自动校准阻尼值 */
	DRV2605_REG_FEEDBACK    = 0x1A,  /* LRA/ERM 反馈配置 */
	DRV2605_REG_CONTROL1    = 0x1B,  /* 控制寄存器 1 */
	DRV2605_REG_CONTROL2    = 0x1C,  /* 控制寄存器 2 */
	DRV2605_REG_CONTROL3    = 0x1D,  /* 控制寄存器 3 */
	DRV2605_REG_CONTROL4    = 0x1E,  /* 控制寄存器 4 */
	DRV2605_REG_VBAT        = 0x21,  /* VBAT 实测值 */
	DRV2605_REG_LRARESON    = 0x22,  /* LRA 共振频率 */
	DRV2605_REG_CONTROL5    = 0x23   /* 控制寄存器 5 */
} DRV2605_Register;

/* 官方内置波形库编号（库寄存器 0x03 对应的值） */
typedef enum {
	DRV2605_LIBRARY_EMPTY = 0x00,  /* 不加载波形库，通常用于自定义 RTP */
	DRV2605_LIBRARY_A     = 0x01,  /* TS2200A：ERM 通用触感库 */
	DRV2605_LIBRARY_B     = 0x02,  /* TS2200B：ERM 精细触感库 */
	DRV2605_LIBRARY_C     = 0x03,  /* TS2200C：ERM 游戏/报警库 */
	DRV2605_LIBRARY_D     = 0x04,  /* TS2200D：ERM 机械按键库 */
	DRV2605_LIBRARY_E     = 0x05,  /* TS2200E：ERM 高级特效库 */
	DRV2605_LIBRARY_LRA   = 0x06,  /* LRA 专用触感库（推荐线性谐振） */
	DRV2605_LIBRARY_F     = 0x07   /* TS2200F：保留/扩展库 */
} DRV2605_Library;

/* ROM 波形效果 ID（写入 WaveSeq 寄存器的标准效果编号） */
typedef enum {
	DRV2605_EFFECT_STRONG_CLICK_100   = 0x01,  /* 波形：强点击 - 100% */
	DRV2605_EFFECT_STRONG_CLICK_60    = 0x02,  /* 波形：强点击 - 60% */
	DRV2605_EFFECT_STRONG_CLICK_30    = 0x03,  /* 波形：强点击 - 30% */
	DRV2605_EFFECT_SHARP_CLICK_100    = 0x04,  /* 波形：锐点击 - 100% */
	DRV2605_EFFECT_SHARP_CLICK_60     = 0x05,  /* 波形：锐点击 - 60% */
	DRV2605_EFFECT_SHARP_CLICK_30     = 0x06,  /* 波形：锐点击 - 30% */
	DRV2605_EFFECT_SOFT_BUMP_100      = 0x07,  /* 波形：柔和顶脉 - 100% */
	DRV2605_EFFECT_SOFT_BUMP_60       = 0x08,  /* 波形：柔和顶脉 - 60% */
	DRV2605_EFFECT_SOFT_BUMP_30       = 0x09,  /* 波形：柔和顶脉 - 30% */
	DRV2605_EFFECT_DOUBLE_CLICK_100   = 0x0A,  /* 波形：强双击 - 100% */
	DRV2605_EFFECT_DOUBLE_CLICK_60    = 0x0B,  /* 波形：强双击 - 60% */
	DRV2605_EFFECT_TRIPLE_CLICK_100   = 0x0C,  /* 波形：强三击 - 100% */
	DRV2605_EFFECT_SOFT_FUZZ_60       = 0x0D,  /* 波形：柔和毛糙感 - 60% */
	DRV2605_EFFECT_STRONG_BUZZ_100    = 0x0E,  /* 波形：强烈嗡鸣 - 100% */
	DRV2605_EFFECT_750_MS_ALERT_100   = 0x0F,  /* 波形：750ms 警报 - 100% */
	DRV2605_EFFECT_1000_MS_ALERT_100  = 0x10,  /* 波形：1000ms 警报 - 100% */
	DRV2605_EFFECT_STRONG_CLICK_1_100 = 0x11,  /* 波形：强点击 1 - 100% */
	DRV2605_EFFECT_STRONG_CLICK_2_80  = 0x12,  /* 波形：强点击 2 - 80% */
	DRV2605_EFFECT_STRONG_CLICK_3_60  = 0x13,  /* 波形：强点击 3 - 60% */
	DRV2605_EFFECT_STRONG_CLICK_4_30  = 0x14,  /* 波形：强点击 4 - 30% */
	DRV2605_EFFECT_MEDIUM_CLICK_1_100 = 0x15,  /* 波形：中等点击 1 - 100% */
	DRV2605_EFFECT_MEDIUM_CLICK_2_80  = 0x16,  /* 波形：中等点击 2 - 80% */
	DRV2605_EFFECT_MEDIUM_CLICK_3_60  = 0x17,  /* 波形：中等点击 3 - 60% */
	DRV2605_EFFECT_SHARP_TICK_1_100   = 0x18,  /* 波形：锐脉冲 1 - 100% */
	DRV2605_EFFECT_SHARP_TICK_2_80    = 0x19,  /* 波形：锐脉冲 2 - 80% */
	DRV2605_EFFECT_SHARP_TICK_3_60    = 0x1A,  /* 波形：锐脉冲 3 - 60% */
	DRV2605_EFFECT_SHORT_DOUBLE_CLICK_STRONG_1_100 = 0x1B,  /* 波形：短双击 强烈 1 - 100% */
	DRV2605_EFFECT_SHORT_DOUBLE_CLICK_STRONG_2_80  = 0x1C,  /* 波形：短双击 强烈 2 - 80% */
	DRV2605_EFFECT_SHORT_DOUBLE_CLICK_STRONG_3_60  = 0x1D,  /* 波形：短双击 强烈 3 - 60% */
	DRV2605_EFFECT_SHORT_DOUBLE_CLICK_STRONG_4_30  = 0x1E,  /* 波形：短双击 强烈 4 - 30% */
	DRV2605_EFFECT_SHORT_DOUBLE_CLICK_MEDIUM_1_100 = 0x1F,  /* 波形：短双击 中等 1 - 100% */
	DRV2605_EFFECT_SHORT_DOUBLE_CLICK_MEDIUM_2_80  = 0x20,  /* 波形：短双击 中等 2 - 80% */
	DRV2605_EFFECT_SHORT_DOUBLE_CLICK_MEDIUM_3_60  = 0x21,  /* 波形：短双击 中等 3 - 60% */
	DRV2605_EFFECT_SHORT_DOUBLE_SHARP_TICK_1_100   = 0x22,  /* 波形：短双击 锐脉冲 1 - 100% */
	DRV2605_EFFECT_SHORT_DOUBLE_SHARP_TICK_2_80    = 0x23,  /* 波形：短双击 锐脉冲 2 - 80% */
	DRV2605_EFFECT_SHORT_DOUBLE_SHARP_TICK_3_60    = 0x24,  /* 波形：短双击 锐脉冲 3 - 60% */
	DRV2605_EFFECT_LONG_DOUBLE_SHARP_CLICK_STRONG_1_100 = 0x25,  /* 波形：长双击 锐点击 强烈 1 - 100% */
	DRV2605_EFFECT_LONG_DOUBLE_SHARP_CLICK_STRONG_2_80  = 0x26,  /* 波形：长双击 锐点击 强烈 2 - 80% */
	DRV2605_EFFECT_LONG_DOUBLE_SHARP_CLICK_STRONG_3_60  = 0x27,  /* 波形：长双击 锐点击 强烈 3 - 60% */
	DRV2605_EFFECT_LONG_DOUBLE_SHARP_CLICK_STRONG_4_30  = 0x28,  /* 波形：长双击 锐点击 强烈 4 - 30% */
	DRV2605_EFFECT_LONG_DOUBLE_SHARP_CLICK_MEDIUM_1_100 = 0x29,  /* 波形：长双击 锐点击 中等 1 - 100% */
	DRV2605_EFFECT_LONG_DOUBLE_SHARP_CLICK_MEDIUM_2_80  = 0x2A,  /* 波形：长双击 锐点击 中等 2 - 80% */
	DRV2605_EFFECT_LONG_DOUBLE_SHARP_CLICK_MEDIUM_3_60  = 0x2B,  /* 波形：长双击 锐点击 中等 3 - 60% */
	DRV2605_EFFECT_LONG_DOUBLE_SHARP_TICK_1_100         = 0x2C,  /* 波形：长双击 锐脉冲 1 - 100% */
	DRV2605_EFFECT_LONG_DOUBLE_SHARP_TICK_2_80          = 0x2D,  /* 波形：长双击 锐脉冲 2 - 80% */
	DRV2605_EFFECT_LONG_DOUBLE_SHARP_TICK_3_60          = 0x2E,  /* 波形：长双击 锐脉冲 3 - 60% */
	DRV2605_EFFECT_BUZZ_1_100                          = 0x2F,  /* 波形：嗡鸣 1 - 100% */
	DRV2605_EFFECT_BUZZ_2_80                           = 0x30,  /* 波形：嗡鸣 2 - 80% */
	DRV2605_EFFECT_BUZZ_3_60                           = 0x31,  /* 波形：嗡鸣 3 - 60% */
	DRV2605_EFFECT_BUZZ_4_40                           = 0x32,  /* 波形：嗡鸣 4 - 40% */
	DRV2605_EFFECT_BUZZ_5_20                           = 0x33,  /* 波形：嗡鸣 5 - 20% */
	DRV2605_EFFECT_PULSING_STRONG_1_100                = 0x34,  /* 波形：强烈脉动 1 - 100% */
	DRV2605_EFFECT_PULSING_STRONG_2_60                 = 0x35,  /* 波形：强烈脉动 2 - 60% */
	DRV2605_EFFECT_PULSING_MEDIUM_1_100                = 0x36,  /* 波形：中等脉动 1 - 100% */
	DRV2605_EFFECT_PULSING_MEDIUM_2_60                 = 0x37,  /* 波形：中等脉动 2 - 60% */
	DRV2605_EFFECT_PULSING_SHARP_1_100                 = 0x38,  /* 波形：锐脉动 1 - 100% */
	DRV2605_EFFECT_PULSING_SHARP_2_60                  = 0x39,  /* 波形：锐脉动 2 - 60% */
	DRV2605_EFFECT_TRANSITION_CLICK_1_100              = 0x3A,  /* 波形：过渡点击 1 - 100% */
	DRV2605_EFFECT_TRANSITION_CLICK_2_80               = 0x3B,  /* 波形：过渡点击 2 - 80% */
	DRV2605_EFFECT_TRANSITION_CLICK_3_60               = 0x3C,  /* 波形：过渡点击 3 - 60% */
	DRV2605_EFFECT_TRANSITION_CLICK_4_40               = 0x3D,  /* 波形：过渡点击 4 - 40% */
	DRV2605_EFFECT_TRANSITION_CLICK_5_20               = 0x3E,  /* 波形：过渡点击 5 - 20% */
	DRV2605_EFFECT_TRANSITION_CLICK_6_10               = 0x3F,  /* 波形：过渡点击 6 - 10% */
	DRV2605_EFFECT_TRANSITION_HUM_1_100                = 0x40,  /* 波形：过渡低鸣 1 - 100% */
	DRV2605_EFFECT_TRANSITION_HUM_2_80                 = 0x41,  /* 波形：过渡低鸣 2 - 80% */
	DRV2605_EFFECT_TRANSITION_HUM_3_60                 = 0x42,  /* 波形：过渡低鸣 3 - 60% */
	DRV2605_EFFECT_TRANSITION_HUM_4_40                 = 0x43,  /* 波形：过渡低鸣 4 - 40% */
	DRV2605_EFFECT_TRANSITION_HUM_5_20                 = 0x44,  /* 波形：过渡低鸣 5 - 20% */
	DRV2605_EFFECT_TRANSITION_HUM_6_10                 = 0x45,  /* 波形：过渡低鸣 6 - 10% */
	DRV2605_EFFECT_TRANSITION_RAMP_DOWN_LONG_SMOOTH_1_100_TO_0  = 0x46,  /* 波形：渐弱 长 平滑 1 - 100%→0% */
	DRV2605_EFFECT_TRANSITION_RAMP_DOWN_LONG_SMOOTH_2_100_TO_0  = 0x47,  /* 波形：渐弱 长 平滑 2 - 100%→0% */
	DRV2605_EFFECT_TRANSITION_RAMP_DOWN_MEDIUM_SMOOTH_1_100_TO_0 = 0x48,  /* 波形：渐弱 中 平滑 1 - 100%→0% */
	DRV2605_EFFECT_TRANSITION_RAMP_DOWN_MEDIUM_SMOOTH_2_100_TO_0 = 0x49,  /* 波形：渐弱 中 平滑 2 - 100%→0% */
	DRV2605_EFFECT_TRANSITION_RAMP_DOWN_SHORT_SMOOTH_1_100_TO_0  = 0x4A,  /* 波形：渐弱 短 平滑 1 - 100%→0% */
	DRV2605_EFFECT_TRANSITION_RAMP_DOWN_SHORT_SMOOTH_2_100_TO_0  = 0x4B,  /* 波形：渐弱 短 平滑 2 - 100%→0% */
	DRV2605_EFFECT_TRANSITION_RAMP_DOWN_LONG_SHARP_1_100_TO_0    = 0x4C,  /* 波形：渐弱 长 急速 1 - 100%→0% */
	DRV2605_EFFECT_TRANSITION_RAMP_DOWN_LONG_SHARP_2_100_TO_0    = 0x4D,  /* 波形：渐弱 长 急速 2 - 100%→0% */
	DRV2605_EFFECT_TRANSITION_RAMP_DOWN_MEDIUM_SHARP_1_100_TO_0  = 0x4E,  /* 波形：渐弱 中 急速 1 - 100%→0% */
	DRV2605_EFFECT_TRANSITION_RAMP_DOWN_MEDIUM_SHARP_2_100_TO_0  = 0x4F,  /* 波形：渐弱 中 急速 2 - 100%→0% */
	DRV2605_EFFECT_TRANSITION_RAMP_DOWN_SHORT_SHARP_1_100_TO_0   = 0x50,  /* 波形：渐弱 短 急速 1 - 100%→0% */
	DRV2605_EFFECT_TRANSITION_RAMP_DOWN_SHORT_SHARP_2_100_TO_0   = 0x51,  /* 波形：渐弱 短 急速 2 - 100%→0% */
	DRV2605_EFFECT_TRANSITION_RAMP_UP_LONG_SMOOTH_1_0_TO_100     = 0x52,  /* 波形：渐强 长 平滑 1 - 0%→100% */
	DRV2605_EFFECT_TRANSITION_RAMP_UP_LONG_SMOOTH_2_0_TO_100     = 0x53,  /* 波形：渐强 长 平滑 2 - 0%→100% */
	DRV2605_EFFECT_TRANSITION_RAMP_UP_MEDIUM_SMOOTH_1_0_TO_100   = 0x54,  /* 波形：渐强 中 平滑 1 - 0%→100% */
	DRV2605_EFFECT_TRANSITION_RAMP_UP_MEDIUM_SMOOTH_2_0_TO_100   = 0x55,  /* 波形：渐强 中 平滑 2 - 0%→100% */
	DRV2605_EFFECT_TRANSITION_RAMP_UP_SHORT_SMOOTH_1_0_TO_100    = 0x56,  /* 波形：渐强 短 平滑 1 - 0%→100% */
	DRV2605_EFFECT_TRANSITION_RAMP_UP_SHORT_SMOOTH_2_0_TO_100    = 0x57,  /* 波形：渐强 短 平滑 2 - 0%→100% */
	DRV2605_EFFECT_TRANSITION_RAMP_UP_LONG_SHARP_1_0_TO_100      = 0x58,  /* 波形：渐强 长 急速 1 - 0%→100% */
	DRV2605_EFFECT_TRANSITION_RAMP_UP_LONG_SHARP_2_0_TO_100      = 0x59,  /* 波形：渐强 长 急速 2 - 0%→100% */
	DRV2605_EFFECT_TRANSITION_RAMP_UP_MEDIUM_SHARP_1_0_TO_100    = 0x5A,  /* 波形：渐强 中 急速 1 - 0%→100% */
	DRV2605_EFFECT_TRANSITION_RAMP_UP_MEDIUM_SHARP_2_0_TO_100    = 0x5B,  /* 波形：渐强 中 急速 2 - 0%→100% */
	DRV2605_EFFECT_TRANSITION_RAMP_UP_SHORT_SHARP_1_0_TO_100     = 0x5C,  /* 波形：渐强 短 急速 1 - 0%→100% */
	DRV2605_EFFECT_TRANSITION_RAMP_UP_SHORT_SHARP_2_0_TO_100     = 0x5D,  /* 波形：渐强 短 急速 2 - 0%→100% */
	DRV2605_EFFECT_TRANSITION_RAMP_DOWN_LONG_SMOOTH_1_50_TO_0    = 0x5E,  /* 波形：渐弱 长 平滑 1 - 50%→0% */
	DRV2605_EFFECT_TRANSITION_RAMP_DOWN_LONG_SMOOTH_2_50_TO_0    = 0x5F,  /* 波形：渐弱 长 平滑 2 - 50%→0% */
	DRV2605_EFFECT_TRANSITION_RAMP_DOWN_MEDIUM_SMOOTH_1_50_TO_0  = 0x60,  /* 波形：渐弱 中 平滑 1 - 50%→0% */
	DRV2605_EFFECT_TRANSITION_RAMP_DOWN_MEDIUM_SMOOTH_2_50_TO_0  = 0x61,  /* 波形：渐弱 中 平滑 2 - 50%→0% */
	DRV2605_EFFECT_TRANSITION_RAMP_DOWN_SHORT_SMOOTH_1_50_TO_0   = 0x62,  /* 波形：渐弱 短 平滑 1 - 50%→0% */
	DRV2605_EFFECT_TRANSITION_RAMP_DOWN_SHORT_SMOOTH_2_50_TO_0   = 0x63,  /* 波形：渐弱 短 平滑 2 - 50%→0% */
	DRV2605_EFFECT_TRANSITION_RAMP_DOWN_LONG_SHARP_1_50_TO_0     = 0x64,  /* 波形：渐弱 长 急速 1 - 50%→0% */
	DRV2605_EFFECT_TRANSITION_RAMP_DOWN_LONG_SHARP_2_50_TO_0     = 0x65,  /* 波形：渐弱 长 急速 2 - 50%→0% */
	DRV2605_EFFECT_TRANSITION_RAMP_DOWN_MEDIUM_SHARP_1_50_TO_0   = 0x66,  /* 波形：渐弱 中 急速 1 - 50%→0% */
	DRV2605_EFFECT_TRANSITION_RAMP_DOWN_MEDIUM_SHARP_2_50_TO_0   = 0x67,  /* 波形：渐弱 中 急速 2 - 50%→0% */
	DRV2605_EFFECT_TRANSITION_RAMP_DOWN_SHORT_SHARP_1_50_TO_0    = 0x68,  /* 波形：渐弱 短 急速 1 - 50%→0% */
	DRV2605_EFFECT_TRANSITION_RAMP_DOWN_SHORT_SHARP_2_50_TO_0    = 0x69,  /* 波形：渐弱 短 急速 2 - 50%→0% */
	DRV2605_EFFECT_TRANSITION_RAMP_UP_LONG_SMOOTH_1_0_TO_50      = 0x6A,  /* 波形：渐强 长 平滑 1 - 0%→50% */
	DRV2605_EFFECT_TRANSITION_RAMP_UP_LONG_SMOOTH_2_0_TO_50      = 0x6B,  /* 波形：渐强 长 平滑 2 - 0%→50% */
	DRV2605_EFFECT_TRANSITION_RAMP_UP_MEDIUM_SMOOTH_1_0_TO_50    = 0x6C,  /* 波形：渐强 中 平滑 1 - 0%→50% */
	DRV2605_EFFECT_TRANSITION_RAMP_UP_MEDIUM_SMOOTH_2_0_TO_50    = 0x6D,  /* 波形：渐强 中 平滑 2 - 0%→50% */
	DRV2605_EFFECT_TRANSITION_RAMP_UP_SHORT_SMOOTH_1_0_TO_50     = 0x6E,  /* 波形：渐强 短 平滑 1 - 0%→50% */
	DRV2605_EFFECT_TRANSITION_RAMP_UP_SHORT_SMOOTH_2_0_TO_50     = 0x6F,  /* 波形：渐强 短 平滑 2 - 0%→50% */
	DRV2605_EFFECT_TRANSITION_RAMP_UP_LONG_SHARP_1_0_TO_50       = 0x70,  /* 波形：渐强 长 急速 1 - 0%→50% */
	DRV2605_EFFECT_TRANSITION_RAMP_UP_LONG_SHARP_2_0_TO_50       = 0x71,  /* 波形：渐强 长 急速 2 - 0%→50% */
	DRV2605_EFFECT_TRANSITION_RAMP_UP_MEDIUM_SHARP_1_0_TO_50     = 0x72,  /* 波形：渐强 中 急速 1 - 0%→50% */
	DRV2605_EFFECT_TRANSITION_RAMP_UP_MEDIUM_SHARP_2_0_TO_50     = 0x73,  /* 波形：渐强 中 急速 2 - 0%→50% */
	DRV2605_EFFECT_TRANSITION_RAMP_UP_SHORT_SHARP_1_0_TO_50      = 0x74,  /* 波形：渐强 短 急速 1 - 0%→50% */
	DRV2605_EFFECT_TRANSITION_RAMP_UP_SHORT_SHARP_2_0_TO_50      = 0x75,  /* 波形：渐强 短 急速 2 - 0%→50% */
	DRV2605_EFFECT_LONG_BUZZ_FOR_PROGRAMMATIC_STOPPING_100       = 0x76,  /* 波形：程序可控长嗡鸣 - 100% */
	DRV2605_EFFECT_SMOOTH_HUM_1_NO_KICK_OR_BRAKE_PULSE_50        = 0x77,  /* 波形：平滑低鸣 1（无启动/刹车脉冲） - 50% */
	DRV2605_EFFECT_SMOOTH_HUM_2_NO_KICK_OR_BRAKE_PULSE_40        = 0x78,  /* 波形：平滑低鸣 2（无启动/刹车脉冲） - 40% */
	DRV2605_EFFECT_SMOOTH_HUM_3_NO_KICK_OR_BRAKE_PULSE_30        = 0x79,  /* 波形：平滑低鸣 3（无启动/刹车脉冲） - 30% */
	DRV2605_EFFECT_SMOOTH_HUM_4_NO_KICK_OR_BRAKE_PULSE_20        = 0x7A,  /* 波形：平滑低鸣 4（无启动/刹车脉冲） - 20% */
	DRV2605_EFFECT_SMOOTH_HUM_5_NO_KICK_OR_BRAKE_PULSE_10        = 0x7B   /* 波形：平滑低鸣 5（无启动/刹车脉冲） - 10% */
} DRV2605_Effect;

/* RTP 实时动作帧与动作组定义（用于脚本播放） */
typedef struct {
	u8 amplitude;   /* 实时播放值 0x00~0x7F */
	u16 holdMs;     /* 该动作保持的时间 */
} DRV2605_RtpAction;

typedef struct {
	const DRV2605_RtpAction *frames; /* 动作数组 */
	u16 frameCount;                   /* 动作数量 */
	u16 pauseMs;                      /* 一轮结束后的暂停 */
	u16 currentIndex;                 /* 当前执行到的索引 */
} DRV2605_ActionGroup;

typedef struct {
	u8 ratedVoltage;  /* 寄存器 0x16 */
	u8 clampVoltage;  /* 寄存器 0x17 */
	u8 control1;      /* 控制寄存器 1 */
	u8 control2;      /* 控制寄存器 2 */
	u8 control3;      /* 控制寄存器 3 */
	u8 control4;      /* 控制寄存器 4 */
	u8 control5;      /* 控制寄存器 5 */
	u16 timeoutMs;    /* 自动校准等待超时 */
} DRV2605_AutoCalConfig;

typedef struct {
	u8 status;        /* STATUS 寄存器 */
	u8 compensation;  /* AUTOCALCOMP */
	u8 backEMF;       /* AUTOCALEMP */
	u8 ratedVoltage;  /* 运行结束后的额定电压寄存器 */
	u8 clampVoltage;  /* 运行结束后的钳位电压寄存器 */
	u8 vbatRaw;       /* VBAT 原始值 */
	u8 lraResonance;  /* LRA 共振寄存器 */
} DRV2605_AutoCalResult;

typedef struct {
	FunctionalState useLRA; /* ENABLE 表示 LRA，DISABLE 表示 ERM */
	u8 driveTime;           /* CONTROL2 的驱动周期（0~63） */
	u8 strength;            /* 实时播放值 0~0x7F */
	DRV2605_Library libraryId; /* 所选触感库 */
} DRV2605_ContinuousConfig;

/**
 * @brief  频率/振幅直驱的时序参数集合。
 * @note   通过 DRV2605_SetFreqAmpTiming() 统一设定，主循环无需关心具体脉冲宽度。
 */
typedef struct {
	u16 burstDurationMs;  /* 高频 burst 的持续时间 */
	u16 pauseDurationMs;  /* 相邻 burst 之间的空档 */
} DRV2605_FreqAmpTiming;

/* 常用模式值，供 SetMode 使用 */
typedef enum {
	DRV2605_MODE_INT_TRIG   = 0x00,  /* 内部触发/序列模式 */
	DRV2605_MODE_EXT_EDGE   = 0x01,  /* 外部上升沿触发 */
	DRV2605_MODE_EXT_LEVEL  = 0x02,  /* 外部电平触发 */
	DRV2605_MODE_PWM_ANALOG = 0x03,  /* PWM/模拟输入模式 */
	DRV2605_MODE_AUDIO      = 0x04,  /* 音频到震动模式 */
	DRV2605_MODE_REALTIME   = 0x05,  /* 实时播放模式 */
	DRV2605_MODE_DIAGNOSTIC = 0x06,  /* 自检诊断模式 */
	DRV2605_MODE_AUTOCAL    = 0x07   /* 自动校准模式 */
} DRV2605_Mode;

/* ========================= API 入口 ========================= */

/* ----------- 基础寄存器访问 ----------- */

/**
 * @brief  使用推荐参数初始化 DRV2605（默认 LRA）。
 * @return READY 成功，NoREADY 失败。
 */
ErrorStatus DRV2605_InitDefaults(void);

/**
 * @brief  向指定寄存器写入 8 位数据。
 * @param  reg   寄存器枚举值（DRV2605_Register）。
 * @param  value 要写入的数值。
 * @return READY 成功，NoREADY 失败。
 */
ErrorStatus DRV2605_WriteRegister(DRV2605_Register reg, u8 value);

/**
 * @brief  读取指定寄存器的 8 位数据。
 * @param  reg   寄存器枚举值。
 * @param  value 输出指针，用于接收读数。
 * @return READY 成功，NoREADY 失败或参数非法。
 */
ErrorStatus DRV2605_ReadRegister(DRV2605_Register reg, u8 *value);

/**
 * @brief  读取 STATUS 寄存器。
 * @param  status 输出状态字节（bit0/1 表示 DIAG/OC 等）。
 * @return READY 成功，NoREADY 失败。
 */
ErrorStatus DRV2605_GetStatus(u8 *status);

/**
 * @brief  设置 DRV2605 的工作模式。
 * @param  mode DRV2605_Mode 枚举值，参考数据手册（如实时模式）。
 * @return READY 成功，NoREADY 失败。
 */
ErrorStatus DRV2605_SetMode(DRV2605_Mode mode);

/**
 * @brief  读取当前工作模式。
 * @param  mode 输出的模式指针。
 * @return READY 成功，NoREADY 失败。
 */
ErrorStatus DRV2605_GetMode(DRV2605_Mode *mode);

/**
 * @brief  选择内部波形库。
 * @param  libraryId DRV2605_Library 枚举值，对应官方触感库编号。
 * @return READY 成功，NoREADY 失败。
 */
ErrorStatus DRV2605_SetLibrary(DRV2605_Library libraryId);

/**
 * @brief  读取当前选择的内部波形库。
 * @param  libraryId 输出的库枚举指针。
 * @return READY 成功，NoREADY 失败。
 */
ErrorStatus DRV2605_GetLibrary(DRV2605_Library *libraryId);

/**
 * @brief  向波形序列槽位写入震动效果编号。
 * @param  slot      槽位编号 0~7。
 * @param  effectId  DRV2605_Effect 枚举值，对应 ROM 波形效果。
 * @return READY 成功，NoREADY 失败/槽位越界。
 */
ErrorStatus DRV2605_SetWaveform(u8 slot, DRV2605_Effect effectId);

/**
 * @brief  读取波形序列槽位当前存储的效果编号。
 * @param  slot      槽位编号 0~7。
 * @param  effectId  输出的效果指针。
 * @return READY 成功，NoREADY 失败/槽位越界。
 */
ErrorStatus DRV2605_GetWaveform(u8 slot, DRV2605_Effect *effectId);

/**
 * @brief  清空剩余波形槽位（WaveSeq2~8 置 0）。
 * @return READY 成功，NoREADY 失败。
 */
ErrorStatus DRV2605_ClearWaveforms(void);

/**
 * @brief  播放实时动作组（函数内部推进索引）。
 * @param  group 动作用组指针，frames/frameCount 等需提前配置。
 * @return READY 表示执行成功，NoREADY 表示写入失败或参数非法。
 */
ErrorStatus DRV2605_RunActionGroup(DRV2605_ActionGroup *group);

/**
 * @brief  填充自动校准默认配置（官方推荐寄存器）。
 * @param  cfg 指向配置结构体，函数会写入默认值。
 */
void DRV2605_FillAutoCalDefaults(DRV2605_AutoCalConfig *cfg);

/**
 * @brief  执行自动校准流程，并可返回测量结果。
 * @param  cfg     调用方配置（为空将返回错误）。
 * @param  result  可选，用于接收状态/补偿等结果。
 * @return READY 成功，NoREADY 失败或参数非法。
 */
ErrorStatus DRV2605_RunAutoCalibration(const DRV2605_AutoCalConfig *cfg,
									   DRV2605_AutoCalResult *result);

/**
 * @brief  将毫伏转为 RATEDV 寄存器编码（21.33mV/LSB）。
 */
u8 DRV2605_EncodeRatedVoltageMv(u16 millivolts);

/**
 * @brief  将毫伏转为 CLAMPV 寄存器编码（5.6mV/LSB）。
 */
u8 DRV2605_EncodeClampVoltageMv(u16 millivolts);

/**
 * @brief  将 RATEDV 编码转换回毫伏值。
 */
u16 DRV2605_DecodeRatedVoltageMv(u8 regValue);

/**
 * @brief  将 CLAMPV 编码转换回毫伏值。
 */
u16 DRV2605_DecodeClampVoltageMv(u8 regValue);

/**
 * @brief  预配置持续震动参数（频率/强度）。
 */
ErrorStatus DRV2605_ConfigureContinuous(const DRV2605_ContinuousConfig *cfg);

/**
 * @brief  启动持续震动（Real-Time Playback）。
 */
ErrorStatus DRV2605_StartContinuous(void);

/**
 * @brief  停止持续震动。
 */
ErrorStatus DRV2605_StopContinuous(void);

/* ----------- 频率/振幅直驱辅助 ----------- */

/**
 * @brief  自定义频率直驱模式的 burst/间隔时间。
 * @param  timing 传入的时序结构体，单位均为 ms。
 */
void DRV2605_SetFreqAmpTiming(const DRV2605_FreqAmpTiming *timing);

/**
 * @brief  设置频率直驱模式下的“满幅”电压映射。
 * @param  maxMillivolts mV 为单位的最大驱动电压（例如 5000 代表 5V）。
 */
void DRV2605_SetFreqAmpVoltageRange(u16 maxMillivolts);

/**
 * @brief  准备实时播放模式，统一配置为 LRA/自触发。
 * @return READY 表示准备完成，可直接调用 Play API。
 */
ErrorStatus DRV2605_PrepareFreqAmpRealtime(void);

/**
 * @brief  按指定频率与幅值执行一次 burst 震动。
 * @param  frequencyHz 目标频率，单位 Hz。
 * @param  amplitude   实时寄存器值 0x01~0x7F。
 */
ErrorStatus DRV2605_PlayFreqAmp(u16 frequencyHz, u8 amplitude);

/**
 * @brief  按指定频率与电压执行一次震动，内部自动映射幅值。
 * @param  frequencyHz 目标频率，单位 Hz。
 * @param  voltageMv   目标电压，单位 mV。
 */
ErrorStatus DRV2605_PlayFreqVoltage(u16 frequencyHz, u16 voltageMv);

/**
 * @brief  启动当前波形序列（GO 寄存器 = 1）。
 * @return READY 成功，NoREADY 失败。
 */
ErrorStatus DRV2605_Start(void);

/**
 * @brief  停止当前波形序列（GO 寄存器 = 0）。
 * @return READY 成功，NoREADY 失败。
 */
ErrorStatus DRV2605_Stop(void);

/**
 * @brief  设置实时播放寄存器，用于播放自定义强度。
 * @param  value 0x00~0x7F 代表不同驱动强度。
 * @return READY 成功，NoREADY 失败。
 */
ErrorStatus DRV2605_SetRealtimeValue(u8 value);

/**
 * @brief  读取实时播放寄存器的当前值。
 * @param  value 输出指针，返回当前 RTP 寄存器内容。
 * @return READY 成功，NoREADY 失败。
 */
ErrorStatus DRV2605_GetRealtimeValue(u8 *value);

/**
 * @brief  设置 Overdrive Clamp 值（限制最大驱动电压）。
 * @param  value 钳位电压相关数值。
 * @return READY 成功，NoREADY 失败。
 */
ErrorStatus DRV2605_SetOverdriveClamp(u8 value);

/**
 * @brief  设置正向/负向维持时间参数。
 * @param  pos 正半周期持续值。
 * @param  neg 负半周期持续值。
 * @return READY 成功，NoREADY 失败。
 */
ErrorStatus DRV2605_SetSustainLevel(u8 pos, u8 neg);

/**
 * @brief  设置制动（Brake）时间。
 * @param  value 刹车级别，值越大停止越快。
 * @return READY 成功，NoREADY 失败。
 */
ErrorStatus DRV2605_SetBrakeLevel(u8 value);

/**
 * @brief  设置 Audio-to-Vibe 模式下的最大幅度。
 * @param  value 0x00~0xFF，决定音频输入对应的峰值强度。
 * @return READY 成功，NoREADY 失败。
 */
ErrorStatus DRV2605_SetAudioMax(u8 value);

/**
 * @brief  设置 Audio-to-Vibe 模式下的最小幅度阈值。
 * @param  value 0x00~0xFF，低于该值时不触发震动。
 * @return READY 成功，NoREADY 失败。
 */
ErrorStatus DRV2605_SetAudioMin(u8 value);

/**
 * @brief  配置 Audio Control 寄存器（滤波、增益等）。
 * @param  value 参照数据手册的 bit 定义组合。
 * @return READY 成功，NoREADY 失败。
 */
ErrorStatus DRV2605_SetAudioControl(u8 value);

/**
 * @brief  设置额定电压（Rated Voltage Register）。
 * @param  value 参照电机电压换算后的寄存器值。
 * @return READY 成功，NoREADY 失败。
 */
ErrorStatus DRV2605_SetRatedVoltage(u8 value);

/**
 * @brief  设置钳位电压（Overdrive/Clamp Voltage Register）。
 * @param  value 参照数据手册计算的钳位值。
 * @return READY 成功，NoREADY 失败。
 */
ErrorStatus DRV2605_SetClampVoltage(u8 value);

/**
 * @brief  设置自动校准补偿值（AUTOCALCOMP）。
 * @param  value 将量产时测得的补偿写回寄存器。
 * @return READY 成功，NoREADY 失败。
 */
ErrorStatus DRV2605_SetAutoCalComp(u8 value);

/**
 * @brief  设置自动校准阻尼/反电动势值（AUTOCALEMP）。
 * @param  value 将量测得到的阻尼值写回寄存器。
 * @return READY 成功，NoREADY 失败。
 */
ErrorStatus DRV2605_SetAutoCalBackEMF(u8 value);

/**
 * @brief  写 CONTROL1~CONTROL5 寄存器，用于高级参数。
 * @param  ctrlValue 寄存器值。
 * @return READY 成功，NoREADY 失败。
 */
ErrorStatus DRV2605_SetControl1(u8 ctrlValue);
ErrorStatus DRV2605_SetControl2(u8 ctrlValue);
ErrorStatus DRV2605_SetControl3(u8 ctrlValue);
ErrorStatus DRV2605_SetControl4(u8 ctrlValue);
ErrorStatus DRV2605_SetControl5(u8 ctrlValue);

/**
 * @brief  读取 VBAT 原始寄存器值（0x21）。
 * @param  value 输出指针，接收 VBAT 读数。
 * @return READY 成功，NoREADY 失败。
 */
ErrorStatus DRV2605_ReadVbatRaw(u8 *value);

/**
 * @brief  读取 LRA 共振频率寄存器（0x22）。
 * @param  value 输出指针，返回共振测量值。
 * @return READY 成功，NoREADY 失败。
 */
ErrorStatus DRV2605_ReadLraResonance(u8 *value);

/**
 * @brief  将反馈模式配置为 LRA（线性谐振执行器）。
 * @return READY 成功，NoREADY 失败。
 */
ErrorStatus DRV2605_SelectLRA(void);

/**
 * @brief  将反馈模式配置为 ERM（偏心旋转马达）。
 * @return READY 成功，NoREADY 失败。
 */
ErrorStatus DRV2605_SelectERM(void);

#endif /* __DRV2605_H */
