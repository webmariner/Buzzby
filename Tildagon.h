#include <dummy_rp2040.h>

#ifndef BUZZBY_TILDAGON_H
#define BUZZBY_TILDAGON_H

#include <Wire.h>

const uint HIGH_SPEED_F = 6;  // GPIO 6 on RPi Pico is HS_F (HS_1) on badge connector
const uint I2C_SDA = 20;     // GPIO 20 on Pi Pico is I2C0 SDA
const uint I2C_SCL = 21;     // GPIO 21 on Pi Pico is I2C0 SCL
const uint BUZZBY_I2C_ADDRESS = 0x62; // Device address for slave Pi Pico
const uint8_t CMD_READ_MSG_LENGTH = 0x01;
const uint8_t CMD_READ_MSG_BODY = 0x02;
const uint8_t CMD_READ_RIC = 0x03;
const uint8_t CMD_READ_SETTING = 0x04;
const uint8_t CMD_MSG_RECEIVED = 0x10;
const uint8_t CMD_NEXT_SETTING = 0x11;

bool tildagonSetupComplete = false;
bool messageWaitingFlag = false;
uint8_t badgeCommandBuffer[100];
byte tildagonNumericOutputBuffer[4];
char tildagonTextOutputBuffer[512];
uint32_t workingNumber = 0;

void handleIncomingCommand(int commandLength) {
  int i;
  for (i = 0; i < commandLength; i++) {
    badgeCommandBuffer[i] = Wire.read();
  }
  badgeCommandBuffer[i] = 0;
}

void handleDataRequest() {
  if (badgeCommandBuffer[0] > 0) {
    switch (badgeCommandBuffer[0]) {
      case CMD_READ_MSG_LENGTH:
        Wire.write(pager.currentMessage().text.length());
        badgeCommandBuffer[0] = 0;
        break;
      case CMD_READ_MSG_BODY:
        pager.currentMessage().text.toCharArray(tildagonTextOutputBuffer, pager.currentMessage().text.length() + 1);
        Wire.write(tildagonTextOutputBuffer, pager.currentMessage().text.length());
        badgeCommandBuffer[0] = 0;
        //pager.markAsRead();
        break;
      case CMD_READ_RIC:
        tildagonNumericOutputBuffer[3] = (pager.currentMessage().ric >> 24) & 0xFF;
        tildagonNumericOutputBuffer[2] = (pager.currentMessage().ric >> 16) & 0xFF;
        tildagonNumericOutputBuffer[1] = (pager.currentMessage().ric >> 8) & 0xFF;
        tildagonNumericOutputBuffer[0]  = pager.currentMessage().ric & 0xFF;
        Wire.write(tildagonNumericOutputBuffer, 4);
        badgeCommandBuffer[0] = 0;
        break;
      case CMD_READ_SETTING:
        workingNumber = (uint32_t)round((modem.centerFreq+(modem.rxOffset/1000))*(1<<14));
        tildagonNumericOutputBuffer[3] = (workingNumber >> 24) & 0xFF;
        tildagonNumericOutputBuffer[2] = (workingNumber >> 16) & 0xFF;
        tildagonNumericOutputBuffer[1] = (workingNumber >> 8) & 0xFF;
        tildagonNumericOutputBuffer[0] = workingNumber & 0xFF;
        Wire.write(tildagonNumericOutputBuffer, 4);
        badgeCommandBuffer[0] = 0;
        break;
    }
  }
}

void tildagonSetup() {
  badgeCommandBuffer[0] = 0;
  Wire.setSDA(I2C_SDA);
  Wire.setSCL(I2C_SCL);
  Wire.begin(BUZZBY_I2C_ADDRESS);// Will usually need to use this device as a slave, address 0x62/98/01100010
  Wire.onReceive(handleIncomingCommand);
  Wire.onRequest(handleDataRequest);
  pinMode(HIGH_SPEED_F, OUTPUT);
  messageWaitingFlag = false;
  digitalWrite(HIGH_SPEED_F, LOW);
  tildagonSetupComplete = true;
  Log.print(0,"\r\nTildagon I2C and HS_1 setup complete\r\n");
}

void setMessageWaitingFlag() {
  if (!messageWaitingFlag) {
    messageWaitingFlag = true;
    digitalWrite(HIGH_SPEED_F, HIGH);
    Log.print(0, "\r\nTildagon MWI ON\r\n");
  }
}

void clearMessageWaitingFlag() {
  if (messageWaitingFlag) {
    messageWaitingFlag = false;
    digitalWrite(HIGH_SPEED_F, LOW);
    Log.print(0, "\r\nTildagon MWI OFF\r\n");
  }
}

void tildagonTeardown() {
  Wire.end();
  tildagonSetupComplete = false;
  Log.print(0,"Tildagon I2C deinitialised\r\n");
}

void tildagonLoop() {
  if (badgeCommandBuffer[0] > 0) {
    switch (badgeCommandBuffer[0]) {
      case CMD_MSG_RECEIVED:
        pager.markAsRead();
        badgeCommandBuffer[0] = 0;
        break;
      case CMD_NEXT_SETTING:
        ;//TODO:Move to next frequency and baud combo
    }
  }
  if (pager.messagesWaiting()) {
    if (pager.currentMessage().text.length() > 0) {
      setMessageWaitingFlag();
    } else {
      pager.markAsRead();
    }
  }
  if (!pager.messagesWaiting()) {
    clearMessageWaitingFlag();
  }
}

#endif
