/*!
 * @file Adafruit_DRV2605.h
 *
 * 这是用于Adafruit DRV2605L触觉驱动器的库 ---->
 * http://www.adafruit.com/products/2305
 *
 * 查看上面的链接了解我们的教程和接线图。
 *
 * 此电机/触觉驱动器使用I2C进行通信。
 *
 * Adafruit投入时间和资源提供此开源代码，
 * 请通过购买Adafruit的产品来支持Adafruit和开源硬件！
 *
 * 由Limor Fried/Ladyada为Adafruit Industries编写。
 *
 * MIT许可证，以上所有文本必须包含在任何重新发行中。
 *
 */
/**************************************************************************/

#ifndef _ADAFRUIT_DRV2605_H
#define _ADAFRUIT_DRV2605_H

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <Adafruit_I2CDevice.h>

#define DRV2605_ADDR 0x5A ///< 设备I2C地址

#define DRV2605_REG_STATUS 0x00       ///< 状态寄存器
#define DRV2605_REG_MODE 0x01         ///< 模式寄存器
#define DRV2605_MODE_INTTRIG 0x00     ///< 内部触发模式
#define DRV2605_MODE_EXTTRIGEDGE 0x01 ///< 外部边沿触发模式
#define DRV2605_MODE_EXTTRIGLVL 0x02  ///< 外部电平触发模式
#define DRV2605_MODE_PWMANALOG 0x03   ///< PWM/模拟输入模式
#define DRV2605_MODE_AUDIOVIBE 0x04   ///< 音频到振动模式
#define DRV2605_MODE_REALTIME 0x05    ///< 实时播放(RTP)模式
#define DRV2605_MODE_DIAGNOS 0x06     ///< 诊断模式
#define DRV2605_MODE_AUTOCAL 0x07     ///< 自动校准模式

#define DRV2605_REG_RTPIN 0x02    ///< 实时播放输入寄存器
#define DRV2605_REG_LIBRARY 0x03  ///< 波形库选择寄存器
#define DRV2605_REG_WAVESEQ1 0x04 ///< 波形序列寄存器1
#define DRV2605_REG_WAVESEQ2 0x05 ///< 波形序列寄存器2
#define DRV2605_REG_WAVESEQ3 0x06 ///< 波形序列寄存器3
#define DRV2605_REG_WAVESEQ4 0x07 ///< 波形序列寄存器4
#define DRV2605_REG_WAVESEQ5 0x08 ///< 波形序列寄存器5
#define DRV2605_REG_WAVESEQ6 0x09 ///< 波形序列寄存器6
#define DRV2605_REG_WAVESEQ7 0x0A ///< 波形序列寄存器7
#define DRV2605_REG_WAVESEQ8 0x0B ///< 波形序列寄存器8

#define DRV2605_REG_GO 0x0C         ///< 开始寄存器
#define DRV2605_REG_OVERDRIVE 0x0D  ///< 过驱动时间偏移寄存器
#define DRV2605_REG_SUSTAINPOS 0x0E ///< 保持时间偏移，正向寄存器
#define DRV2605_REG_SUSTAINNEG 0x0F ///< 保持时间偏移，负向寄存器
#define DRV2605_REG_BREAK 0x10      ///< 制动时间偏移寄存器
#define DRV2605_REG_AUDIOCTRL 0x11  ///< 音频到振动控制寄存器
#define DRV2605_REG_AUDIOLVL                                                   \
  0x12 ///< 音频到振动最小输入电平寄存器
#define DRV2605_REG_AUDIOMAX                                                   \
  0x13 ///< 音频到振动最大输入电平寄存器
#define DRV2605_REG_AUDIOOUTMIN                                                \
  0x14 ///< 音频到振动最小输出驱动寄存器
#define DRV2605_REG_AUDIOOUTMAX                                                \
  0x15                          ///< 音频到振动最大输出驱动寄存器
#define DRV2605_REG_RATEDV 0x16 ///< 额定电压寄存器
#define DRV2605_REG_CLAMPV 0x17 ///< 过驱动钳位电压寄存器
#define DRV2605_REG_AUTOCALCOMP                                                \
  0x18 ///< 自动校准补偿结果寄存器
#define DRV2605_REG_AUTOCALEMP                                                 \
  0x19                            ///< 自动校准反电动势结果寄存器
#define DRV2605_REG_FEEDBACK 0x1A ///< 反馈控制寄存器
#define DRV2605_REG_CONTROL1 0x1B ///< 控制1寄存器
#define DRV2605_REG_CONTROL2 0x1C ///< 控制2寄存器
#define DRV2605_REG_CONTROL3 0x1D ///< 控制3寄存器
#define DRV2605_REG_CONTROL4 0x1E ///< 控制4寄存器
#define DRV2605_REG_VBAT 0x21     ///< Vbat电压监测寄存器
#define DRV2605_REG_LRARESON 0x22 ///< LRA共振周期寄存器

/**************************************************************************/
/*!
  @brief DRV2605驱动类。
*/
/**************************************************************************/
class Adafruit_DRV2605 {
public:
  Adafruit_DRV2605(void);
  bool begin(TwoWire *theWire = &Wire);

  bool init();
  void writeRegister8(uint8_t reg, uint8_t val);
  uint8_t readRegister8(uint8_t reg);
  void setWaveform(uint8_t slot, uint8_t w);
  void selectLibrary(uint8_t lib);
  void go(void);
  void stop(void);
  void setMode(uint8_t mode);
  void setRealtimeValue(uint8_t rtp);
  // 选择ERM(偏心旋转质量)或LRA(线性谐振执行器)
  // 振动电机。默认值为ERM，这是最常见的
  void useERM();
  void useLRA();

private:
  Adafruit_I2CDevice *i2c_dev = NULL; ///< 指向I2C总线接口的指针
};

#endif
