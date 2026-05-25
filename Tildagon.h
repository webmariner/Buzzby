//#include <dummy_rp2040.h>

#ifndef BUZZBY_TILDAGON_H
#define BUZZBY_TILDAGON_H

#include <Wire.h>
#include "BuzzbyController.h"

const uint HIGH_SPEED_F = 6;  // GPIO 6 on RPi Pico is HS_F (HS_1) on badge connector
const uint I2C_SDA = 20;     // GPIO 20 on Pi Pico is I2C0 SDA
const uint I2C_SCL = 21;     // GPIO 21 on Pi Pico is I2C0 SCL
const uint BUZZBY_I2C_ADDRESS = 0x62; // Device address for slave Pi Pico

const uint8_t CMD_READ_MSG_LENGTH = 0x01;
const uint8_t CMD_READ_MSG_BODY = 0x02;
const uint8_t CMD_READ_RIC = 0x03;
const uint8_t CMD_READ_FREQ = 0x04;
const uint8_t CMD_READ_BAUD = 0x05;
const uint8_t CMD_MSG_RECEIVED = 0x10;
const uint8_t CMD_NEXT_CHANNEL = 0x11;

class Tildagon {
public:
  static Tildagon& instance();
  void tildagonSetup(BuzzbyController* con);
  void tildagonTeardown();
  bool isSetupComplete();
  void tildagonLoop();
private:
  Tildagon();
  static void handleIncomingCommand(int commandLength);
  static void handleDataRequest();
  void objHandleCmd(int commandLength);
  void objHandleDataReq();
  void setMessageWaitingFlag();
  void clearMessageWaitingFlag();
  BuzzbyController* controller;
  bool tildagonSetupComplete;
  bool messageWaitingFlag;
  uint8_t badgeCommandBuffer[100];
  byte tildagonNumericOutputBuffer[4];
  char tildagonTextOutputBuffer[512];
  uint32_t workingNumber;
};

#endif
