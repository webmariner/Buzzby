#ifndef BUZZBY_SX1278_H
#define BUZZBY_SX1278_H

#include "src/ArduinoLog.h"
#include <SPI.h>

const pin_size_t SX1278_SCK = 18;
const pin_size_t SX1278_MISO = 16;
const pin_size_t SX1278_MOSI = 19;
const pin_size_t SX1278_CS = 17;
const pin_size_t SX1278_RST = 22;
const pin_size_t SX1278_DIO0 = 10;
const pin_size_t SX1278_DIO1 = 11;
const pin_size_t SX1278_DIO2 = 12;
const pin_size_t SX1278_DIO3 = 13;

#define regOpMode 0x1
#define regBrMSB 0x2
#define regBrLSB 0x3
#define regShiftMSB 0x4
#define regShiftLSB 0x5
#define regFreqMSB 0x6
#define regFreqMID 0x7
#define regFreqLSB 0x8
#define regRxLna 0xc
#define regRxCfg 0xd
#define regRssiThreshold 0x10
#define regRssi 0x11
#define regRxBw 0x12
#define regAfcBw 0x13
#define regOokPeak 0x14
#define regAfcFei 0x1a
#define regAfcMSB 0x1b
#define regAfcLSB 0x1c
#define regFeiMSB 0x1d
#define regFeiLSB 0x1e
#define regPreambleDetect 0x1f
#define regRxTimeout2 0x21
#define regRxTimeout3 0x22
#define regSynCfg 0x27
#define regSynByte1 0x28
#define regSynByte2 0x29
#define regSynByte3 0x2a
#define regSynByte4 0x2b
#define regPckCfg1 0x30
#define regPckCfg2 0x31
#define regPckLength 0x32
#define regSeqConfig1 0x36
#define regSeqConfig2 0x37
#define regIrqFlags1 0x3e
#define regDioMap1 0x40
#define regDioMap2 0x41
#define regChipVersion 0x42

const double gainValues[8] = { 0, 0, -6, -12, -24, -36, -48, 0 };
const double bwValues[21] = {
  2.6, 3.1, 3.9, 5.2, 6.3, 7.8, 10.4, 12.5, 15.6, 20.8, 25, 31.3, 41.7, 50, 62.5, 83.3, 100, 125, 166.7, 200, 250
};
const uint8_t bwIntegers[21] = { 23, 15, 7, 22, 14, 6, 21, 13, 5, 20, 12, 4, 19, 11, 3, 18, 10, 2, 17, 9, 1 };


class SX1278 {
public:
  void setup();
  uint8_t readSPI(uint8_t addr);
  void writeSPI(uint8_t addr, uint8_t value);
  void setReg(uint8_t addr, uint8_t msb, uint8_t lsb, uint8_t value);
  uint8_t getReg(uint8_t addr, uint8_t msb, uint8_t lsb);
  void resetChip();
  void setFrequency(double frequency);
  void setBitrate(double bitrate);
  void setShift(double shift);
  void setRxBandwidth(double rxBandwidth);
  void setRxBwAuto();
  void setAfcBandwidth(double afcBandwidth);
  void setAfcBwAuto(double error = 12);
  void restartRx(bool withPLL);
  double getFrequency();
  double getBitrate();
  double getAFC();
  double getFEI();
  double getGain();
  void setRssiThreshold(int threshold);
  double getRSSI();
  void printHwVersion();
  void printCurrentRxStats();
  SX1278(double frequency_MHz, double bitrate_kbps, double shift_kHz, double rxBandwidth_kHz, double afcBandwidth_kHz);
private:
  double _frequency;
  double _bitrate;
  double _shift;
  double _rxBandwidth;
  double _afcBandwidth;
};

#endif
