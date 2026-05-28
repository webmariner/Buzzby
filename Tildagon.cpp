#include "Tildagon.h"

Tildagon& Tildagon::instance() {
  static Tildagon s_instance;
  return s_instance;
}

Tildagon::Tildagon()
  : tildagonSetupComplete(false)
  , messageWaitingFlag(false)
  , workingNumber(0)
  , curMsgLength(0)
  , curRic(0)
{}

void Tildagon::handleIncomingCommand(int commandLength) {
  Tildagon::instance().objHandleCmd(commandLength);
}

void Tildagon::objHandleCmd(int commandLength) {
  int i;
  for (i = 0; i < commandLength; i++) {
    badgeCommandBuffer[i] = Wire.read();
  }
  badgeCommandBuffer[i] = 0;
  switch (badgeCommandBuffer[0]) {
    case CMD_MSG_RECEIVED:
      prepRequired = false;
      messagePrepared = false;
      break;
    case CMD_READ_MSG_LENGTH:
    case CMD_READ_MSG_BODY:
    case CMD_READ_RIC:
    case CMD_READ_BAUD:
    case CMD_READ_FREQ:
      prepRequired = true;
  }
}

void Tildagon::handleDataRequest() {
  Tildagon::instance().objHandleDataReq();
}

void Tildagon::objHandleDataReq() {
  if (prepRequired) prepForDataRequest();
  if (badgeCommandBuffer[0] > 0) {
    switch (badgeCommandBuffer[0]) {
      case CMD_READ_MSG_LENGTH:
        Wire.write(curMsgLength);
        badgeCommandBuffer[0] = 0;
        break;
      case CMD_READ_MSG_BODY:
        Wire.write(tildagonTextOutputBuffer, curMsgLength);
        badgeCommandBuffer[0] = 0;
        break;
      case CMD_READ_RIC:
        tildagonNumericOutputBuffer[3] = (curRic >> 24) & 0xFF;
        tildagonNumericOutputBuffer[2] = (curRic >> 16) & 0xFF;
        tildagonNumericOutputBuffer[1] = (curRic >> 8) & 0xFF;
        tildagonNumericOutputBuffer[0]  = curRic & 0xFF;
        Wire.write(tildagonNumericOutputBuffer, 4);
        badgeCommandBuffer[0] = 0;
        break;
      case CMD_READ_FREQ:
        tildagonNumericOutputBuffer[3] = (workingNumber >> 24) & 0xFF;
        tildagonNumericOutputBuffer[2] = (workingNumber >> 16) & 0xFF;
        tildagonNumericOutputBuffer[1] = (workingNumber >> 8) & 0xFF;
        tildagonNumericOutputBuffer[0] = workingNumber & 0xFF;
        Wire.write(tildagonNumericOutputBuffer, 4);
        badgeCommandBuffer[0] = 0;
        break;
      case CMD_READ_BAUD:
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

void Tildagon::tildagonSetup(BuzzbyController* con) {
  controller = con;
  badgeCommandBuffer[0] = 0;
  Wire.setSDA(I2C_SDA);
  Wire.setSCL(I2C_SCL);
  Wire.begin(BUZZBY_I2C_ADDRESS);// Will usually need to use this device as a slave, address 0x62/98/01100010
  Wire.onReceive(Tildagon::handleIncomingCommand);
  Wire.onRequest(Tildagon::handleDataRequest);
  pinMode(HIGH_SPEED_F, OUTPUT);
  prepRequired = false;
  messageWaitingFlag = false;
  messagePrepared = false;
  digitalWrite(HIGH_SPEED_F, LOW);
  tildagonSetupComplete = true;
  Log.traceln("Tildagon I2C and HS_1 setup complete");
}

void Tildagon::setMessageWaitingFlag() {
  if (!messageWaitingFlag) {
    messageWaitingFlag = true;
    messagePrepared = false;
    digitalWrite(HIGH_SPEED_F, HIGH);
    Log.traceln( "Tildagon MWI ON");
  }
}

void Tildagon::clearMessageWaitingFlag() {
  if (messageWaitingFlag) {
    messageWaitingFlag = false;
    messagePrepared = false;
    digitalWrite(HIGH_SPEED_F, LOW);
    Log.traceln( "Tildagon MWI OFF");
  }
}

void Tildagon::tildagonTeardown() {
  Wire.end();
  tildagonSetupComplete = false;
  Log.traceln("Tildagon I2C deinitialised");
}

bool Tildagon::isSetupComplete() {
  return tildagonSetupComplete;
}

void Tildagon::tildagonLoop() {
  if (badgeCommandBuffer[0] > 0) {
    if (prepRequired) {
      prepForDataRequest();
    }
    switch (badgeCommandBuffer[0]) {
      case CMD_MSG_RECEIVED:
        controller->markAsRead();
        messagePrepared = false;
        badgeCommandBuffer[0] = 0;
        Log.traceln("Marked message as received");
        break;
      case CMD_NEXT_CHANNEL:
        controller->next();
        badgeCommandBuffer[0] = 0;
        Log.traceln("Moved to next channel");
        break;
    }
  }
  if (!messageWaitingFlag && controller->messagesWaiting()) {
    setMessageWaitingFlag();
  } else if (messageWaitingFlag && !controller->messagesWaiting()) {
    clearMessageWaitingFlag();
  }
}

void Tildagon::prepForDataRequest() {
  switch (badgeCommandBuffer[0]) {
    case CMD_READ_MSG_LENGTH:
    case CMD_READ_MSG_BODY:
    case CMD_READ_RIC:
      if (!messagePrepared) {
        PagerMessage curMsg = controller->getCurrentMsg();
        curMsgLength = curMsg.text.length();
        curRic = curMsg.ric;
        curMsg.text.toCharArray(tildagonTextOutputBuffer, curMsgLength + 1);
        messagePrepared = true;
      }
      break;
    case CMD_READ_BAUD:
      workingNumber = (uint32_t)round(controller->getBitrate()*1000);
      break;
    case CMD_READ_FREQ:
      workingNumber = (uint32_t)round(controller->getFrequency()*1000000);
      break;
  }
  prepRequired = false;
}