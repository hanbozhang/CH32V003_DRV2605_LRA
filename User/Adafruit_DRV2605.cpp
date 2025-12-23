/*!
 * @file Adafruit_DRV2605.cpp
 *
 * @mainpage Adafruit DRV2605L触觉驱动器
 *
 * @section intro_sec 介绍
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
 * @section author 作者
 *
 * 由Limor Fried/Ladyada为Adafruit Industries编写。
 *
 * @section license 许可证
 *
 * MIT许可证，以上所有文本必须包含在任何重新发行中。
 *
 */
/**************************************************************************/

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <Adafruit_DRV2605.h>

/*========================================================================*/
/*                            构造函数                                */
/*========================================================================*/

/**************************************************************************/
/*!
  @brief  创建一个新的DRV2605类的实例。I2C，无地址调整或引脚
*/
/**************************************************************************/
Adafruit_DRV2605::Adafruit_DRV2605() {}

/*========================================================================*/
/*                           公共函数                             */
/*========================================================================*/

/**************************************************************************/
/*!
  @brief 使用指定的Wire设置硬件
  @param theWire 指向TwoWire对象的指针，默认为&Wire
  @return init()的返回值
*/
/**************************************************************************/
bool Adafruit_DRV2605::begin(TwoWire *theWire) {
  if (i2c_dev)
    delete i2c_dev;
  i2c_dev = new Adafruit_I2CDevice(DRV2605_ADDR, theWire);
  return init();
}

/**************************************************************************/
/*!
  @brief  设置硬件
  @return 总是返回true
*/
/**************************************************************************/
bool Adafruit_DRV2605::init() {
  if (!i2c_dev->begin())
    return false;
  // uint8_t id = readRegister8(DRV2605_REG_STATUS);
  // Serial.print("Status 0x"); Serial.println(id, HEX);

  writeRegister8(DRV2605_REG_MODE, 0x00); // 退出待机状态

  writeRegister8(DRV2605_REG_RTPIN, 0x00); // 无实时播放

  writeRegister8(DRV2605_REG_WAVESEQ1, 1); // 强点击
  writeRegister8(DRV2605_REG_WAVESEQ2, 0); // 序列结束

  writeRegister8(DRV2605_REG_OVERDRIVE, 0); // 无过驱动

  writeRegister8(DRV2605_REG_SUSTAINPOS, 0);
  writeRegister8(DRV2605_REG_SUSTAINNEG, 0);
  writeRegister8(DRV2605_REG_BREAK, 0);
  writeRegister8(DRV2605_REG_AUDIOMAX, 0x64);

  // ERM开环

  // 关闭N_ERM_LRA
  writeRegister8(DRV2605_REG_FEEDBACK,
                 readRegister8(DRV2605_REG_FEEDBACK) & 0x7F);
  // 打开ERM_OPEN_LOOP
  writeRegister8(DRV2605_REG_CONTROL3,
                 readRegister8(DRV2605_REG_CONTROL3) | 0x20);

  return true;
}

/**************************************************************************/
/*!
  @brief 选择要使用的触觉波形。
  @param slot 要设置的波形槽，从0到7
  @param w 波形序列值，指的是ROM库中的索引。

    播放从槽0开始并继续到槽7，如果遇到值0则停止。
    可用波形列表可在数据手册第11.2节中找到：
    http://www.adafruit.com/datasheets/DRV2605.pdf
*/
/**************************************************************************/
void Adafruit_DRV2605::setWaveform(uint8_t slot, uint8_t w) {
  writeRegister8(DRV2605_REG_WAVESEQ1 + slot, w);
}

/**************************************************************************/
/*!
  @brief 选择要使用的波形库。
  @param lib 库选择，0 = 空，1-5为ERM，6为LRA。

    有关详细信息，请参阅数据手册第7.6.4节：
    http://www.adafruit.com/datasheets/DRV2605.pdf
*/
/**************************************************************************/
void Adafruit_DRV2605::selectLibrary(uint8_t lib) {
  writeRegister8(DRV2605_REG_LIBRARY, lib);
}

/**************************************************************************/
/*!
  @brief 开始播放波形(开始移动！)。
*/
/**************************************************************************/
void Adafruit_DRV2605::go() { writeRegister8(DRV2605_REG_GO, 1); }

/**************************************************************************/
/*!
  @brief 停止播放。
*/
/**************************************************************************/
void Adafruit_DRV2605::stop() { writeRegister8(DRV2605_REG_GO, 0); }

/**************************************************************************/
/*!
  @brief 设置设备模式。
  @param mode 模式值，请参见数据手册第7.6.2节：
  http://www.adafruit.com/datasheets/DRV2605.pdf

    0: 内部触发，调用go()开始播放\n
    1: 外部触发，IN引脚上升沿开始播放\n
    2: 外部触发，播放遵循IN引脚的状态\n
    3: PWM/模拟输入\n
    4: 音频\n
    5: 实时播放\n
    6: 诊断\n
    7: 自动校准
*/
/**************************************************************************/
void Adafruit_DRV2605::setMode(uint8_t mode) {
  writeRegister8(DRV2605_REG_MODE, mode);
}

/**************************************************************************/
/*!
  @brief 在RTP模式下设置实时值，用于直接驱动
  触觉电机。
  @param rtp 8位驱动值。
*/
/**************************************************************************/
void Adafruit_DRV2605::setRealtimeValue(uint8_t rtp) {
  writeRegister8(DRV2605_REG_RTPIN, rtp);
}

/**************************************************************************/
/*!
  @brief 读取8位寄存器。
  @param reg 要读取的寄存器。
  @return 寄存器的8位值。
*/
/**************************************************************************/
uint8_t Adafruit_DRV2605::readRegister8(uint8_t reg) {
  uint8_t buffer[1] = {reg};
  i2c_dev->write_then_read(buffer, 1, buffer, 1);
  return buffer[0];
}

/**************************************************************************/
/*!
  @brief 写入8位寄存器。
  @param reg 要写入的寄存器。
  @param val 要写入的值。
*/
/**************************************************************************/
void Adafruit_DRV2605::writeRegister8(uint8_t reg, uint8_t val) {
  uint8_t buffer[2] = {reg, val};
  i2c_dev->write(buffer, 2);
}

/**************************************************************************/
/*!
  @brief 使用ERM(偏心旋转质量)模式。
*/
/**************************************************************************/
void Adafruit_DRV2605::useERM() {
  writeRegister8(DRV2605_REG_FEEDBACK,
                 readRegister8(DRV2605_REG_FEEDBACK) & 0x7F);
}

/**************************************************************************/
/*!
  @brief 使用LRA(线性谐振执行器)模式。
*/
/**************************************************************************/
void Adafruit_DRV2605::useLRA() {
  writeRegister8(DRV2605_REG_FEEDBACK,
                 readRegister8(DRV2605_REG_FEEDBACK) | 0x80);
}
