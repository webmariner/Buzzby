#include "PagerReceiver.h"
#include "SX1278.h"
#include "src/BCH3121.h"
CBCH3121 bch;


volatile bool detectDIO0Flag = false;
volatile bool detectDIO3Flag = false;

xQueueHandle queueDIO1;
UBaseType_t queueSizeDIO1 = 1024;

// Interrupt Service Routines for SX1278
void dio0ISR() {
  UBaseType_t uxSavedInterruptStatus;
  uxSavedInterruptStatus = portENTER_CRITICAL_FROM_ISR();
  detectDIO0Flag = true;
  portEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);
}

void dio1ISR() {
  static uint8_t buffer = 0;
  static uint8_t bufferMask = 128;
  if (digitalRead(SX1278_DIO2) == LOW) {
    buffer |= bufferMask;
  }
  bufferMask >>= 1;
  if (bufferMask == 0) {
    xQueueSendFromISR(queueDIO1, &buffer, NULL);
    buffer=0;
    bufferMask=128;
  }
}

void dio3ISR() {
  UBaseType_t uxSavedInterruptStatus;
  uxSavedInterruptStatus = portENTER_CRITICAL_FROM_ISR();
  detectDIO3Flag = true;
  portEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);
}

void PagerReceiver::setModeFskRxCont() {
    _radio->setReg(regOpMode, 7, 7, 0);           // Mode 0:FSK/OOK 1:LoRa
    _radio->setReg(regOpMode, 6, 5, 0);           // Modulation 0:FSK 1:OOK
    _radio->setReg(regOpMode, 3, 3, 1);           // Frequency Mode 0:High 1:Low
    _radio->setReg(regOpMode, 2, 0, 1);           // Transceiver Mode 0:Sleep 1:Standby 2:FSTx 3:Tx 4:FSRx 5:Rx
    _radio->setReg(regOokPeak, 5, 5, 1);          // Bit Synchronizer 0:Off 1:On
    _radio->setReg(regRxCfg, 7, 7, 0);            // Restart Rx on Collision 0:Off 1:On
    _radio->setReg(regRxCfg, 4, 4, 1);            // AFC Auto 0:Off 1:On
    _radio->setReg(regRxCfg, 3, 3, 1);            // AGC Auto 0:Off 1:On
    _radio->setReg(regRxCfg, 2, 0, 6);            // Rx Interrupt triggers 6:Preamble AGC+AFC 7:Preamble+RSSI AGC+AFC
    _radio->setReg(regAfcFei, 0, 0, 0);           // AFC Auto Clear 0:Off 1:On
    _radio->setReg(regPreambleDetect, 7, 7, 1);   // Preamble Detector 0:Off 1:On
    _radio->setReg(regPreambleDetect, 6, 5, 1);   // Preamble Detector Size 0:1 Byte 1:2 Bytes 2:3 Bytes
    _radio->setReg(regPreambleDetect, 4, 0, 10);  // Preamble Detector Errors tolerated
    _radio->setReg(regSynCfg, 5, 5, 0);           // Preamble Polarity 0:AA 1:55
    _radio->setReg(regSynCfg, 4, 4, 1);           // Sync Word Rx/Tx Processing 0:Off 1:On
    _radio->setReg(regSynCfg, 2, 0, 3);           // Sync Byte Count x:x+1
    _radio->writeSPI(regSynByte1, pocsagSyncWord[0]);   // Frame Sync Byte 1
    _radio->writeSPI(regSynByte2, pocsagSyncWord[1]);   // Frame Sync Byte 2
    _radio->writeSPI(regSynByte3, pocsagSyncWord[2]);   // Frame Sync Byte 3
    _radio->writeSPI(regSynByte4, pocsagSyncWord[3]);   // Frame Sync Byte 4
    _radio->setReg(regPckCfg1, 7, 7, 1);          // Packet Length 0:Fixed 1:Variable
    _radio->setReg(regPckCfg1, 6, 5, 0);          // DC Free 0:Off 1:Manchester 2:Whitening
    _radio->setReg(regPckCfg1, 4, 4, 0);          // CRC Rx/Tx Processing 0:Off 1:On
    _radio->setReg(regPckCfg1, 2, 1, 0);          // Address Filtering 0:Off
    _radio->setReg(regPckCfg2, 6, 6, 0);          // Data Mode 0:Continuous 1:Packet
    _radio->setReg(regPckCfg2, 2, 0, 7);          // Packet Length MSB
    _radio->setReg(regPckLength, 7, 0, 255);      // Packet Length LSB
}

void PagerReceiver::startSequencer() {
    _radio->setReg(regOpMode, 2, 0, 1);      // Transceiver Mode 0:Sleep 1:Standby 2:FSTx 3:Tx 4:FSRx 5:Rx
    _radio->setReg(regRxTimeout2, 7, 0, 4);  // Preamble Timeout 0:Off x:x*16*Tbit
    _radio->setReg(regRxTimeout3, 7, 0, 0);  // Frame Sync Timeout 0:Off x:x*16*Tbit
    _radio->setReg(regSeqConfig1, 4, 3, 1);  // Sequence from Start to 0:Low Power 1:Rx 2:Tx
    _radio->setReg(regSeqConfig2, 4, 3, 0);  // Sequence from Rx Timeout to 0:Rx Restart 1:Tx 2:Low Power 3:Off
    _radio->setReg(regSeqConfig1, 7, 7, 1);  // Sequencer Start
}

void PagerReceiver::stopSequencer() {
    timerRx = millis() + 2000;
    _radio->setReg(regSeqConfig1, 6, 6, 1);  // Sequencer Stop
    _radio->setReg(regRxTimeout2, 7, 0, 0);  // Preamble Timeout 0:Off x:x*16*Tbit
    _radio->setReg(regRxTimeout3, 7, 0, 0);  // Frame Sync Timeout 0:Off x:x*16*Tbit
    //delay(100);
    _radio->setReg(regOpMode, 2, 0, 4);      // Transceiver Mode 0:Sleep 1:Standby 2:FSTx 3:Tx 4:FSRx 5:Rx
}

bool PagerReceiver::available() {
    uint8_t lastByte;
    return xQueuePeekFromISR(queueDIO1, &lastByte);
}

uint8_t PagerReceiver::read() {
    uint8_t lastByte;
    xQueueReceiveFromISR(queueDIO1, &lastByte, NULL);
    return lastByte;
}

PagerReceiver::PagerReceiver() {
    this->_radio = new SX1278(439.9875, 1.2, 4.5, 5.2, 25);
}

  void PagerReceiver::setup(PagerQueue pagerq) {
    _pagerq = pagerq;

    // Initialise SX1278 hardware & IO
    _radio->setup();

    // Setup SX1278 registers and callbacks
    setModeFskRxCont();
    _radio->setReg(regDioMap1, 7, 6, 1);  // DIO0 Mapping 0:Sync Word 1:RSSI/Preamble Detect
    _radio->setReg(regDioMap1, 5, 4, 0);  // DIO1 Mapping 0:Clock
    _radio->setReg(regDioMap1, 3, 2, 0);  // DIO2 Mapping 0:Data
    _radio->setReg(regDioMap1, 1, 0, 0);  // DIO3 Mapping 0:Timeout 1:RSSI/Preamble Detect
    _radio->setReg(regDioMap2, 0, 0, 1);  // Map Detect Interrupt 0:RSSI 1:Preamble
    queueDIO1 = xQueueCreate(queueSizeDIO1, sizeof(uint8_t));
    attachInterrupt(SX1278_DIO0, dio0ISR, RISING);
    attachInterrupt(SX1278_DIO1, dio1ISR, RISING);
    attachInterrupt(SX1278_DIO3, dio3ISR, RISING);
    _radio->restartRx(true);
    startSequencer();
    delay(500);
    timerRx = millis() + 1000;
    Log.traceln("POCSAG Rx started");
  }

  void PagerReceiver::updateSettings(double frequency, double bitrate) {
    stopSequencer();
    _radio->setFrequency(frequency);
    _radio->setBitrate(bitrate);
    _radio->setShift(4.5);
    _radio->setRxBandwidth(5.2);
    _radio->setAfcBandwidth(25);
    startSequencer();
    _radio->restartRx(true);
  }

  double PagerReceiver::getFrequency() {
    return _radio->getFrequency();
  }

  double PagerReceiver::getBitrate() {
    return _radio->getBitrate();
  }

  void PagerReceiver::printRadioHardwareDetails() {
    _radio->printHwVersion();
  }

  void PagerReceiver::printStats() {
    _radio->printCurrentRxStats();
    Log.info("Messages received: %i", _messageCount);
    Log.info("   Errors corrected: %i", _errorCount.corrected);
    Log.info("   uncorrected: %i", _errorCount.uncorrected);
    Log.infoln("   Bytes queued: %i/%i",uxQueueMessagesWaitingFromISR(queueDIO1),queueSizeDIO1);
    Log.infoln("   Debug: %i", Log.getLevel() - LOG_LEVEL_WARNING);
  }

  void PagerReceiver::messageReceived() {
    Log.verboseln("    BCH Errors: %i/%i", error.corrected, error.uncorrected);
    Log.traceln("    Message as numeric: %s", messageNumeric.c_str());
    Log.traceln("    Message as alpha  : %s", messageAlpha.c_str());
    String message = guessFormat(messageAlpha, messageNumeric);
    String postValue = "rssi:     " + String(rssi, 1);
    postValue += "\r\nerror:    " + String(error.uncorrected);
    postValue += "\r\nric:      " + String(ric);
    postValue += "\r\nfunction: " + String(int(function));
    postValue += "\r\nmessage:  " + message;
    Log.infoln(postValue.c_str());
    error.corrected = 0;
    error.uncorrected = 0;
    messageAlpha = "";
    messageNumeric = "";
    _messageCount++;
    _pagerq.push({message, ric});
  }

  uint8_t PagerReceiver::searchSync(uint8_t rxByte) {
    static uint32_t syncBuffer;
    for (uint8_t bitPos = 0; bitPos <= 7; bitPos++) {
      syncBuffer <<= 1;
      syncBuffer |= (rxByte & 128) >> 7;
      rxByte <<= 1;
      if (syncBuffer == 0x7cd215d8) { return 7 - bitPos; }
    }
    return 255;
  }

  void PagerReceiver::pocsagWorker() {
    if (millis() >= timerRx) {
      timerRx = millis() + 1000;
      if (isMessageRun) {
        isMessageRun = false;
        messageReceived();
        ric = 0;
        function = 0xff;
      }
      _radio->restartRx(false);
    }

    taskENTER_CRITICAL();
    if (detectDIO0Flag) {
      detectDIO0Flag = false;
      taskEXIT_CRITICAL();
      Log.verboseln("Preamble Detected!");
      if (Log.getLevel() > LOG_LEVEL_WARNING) _radio->printCurrentRxStats();
      Log.verboseln("Bytes queued: %i/%i", uxQueueMessagesWaitingFromISR(queueDIO1), queueSizeDIO1);
      rssi = _radio->getRSSI() - _radio->getGain();
      error.corrected = 0;
      error.uncorrected = 0;
      ric = 0;
      function = 0xff;
      messageAlpha = "";
      messageNumeric = "";
      timerRx = millis() + 1000;
    } else {
      taskEXIT_CRITICAL();
    }

    if (available()) {
      uint8_t rxByte = read();
      uint8_t bitShift = searchSync(rxByte);

      if (bitShift == 255) return;

      timerRx = millis() + 1000;
      Log.verboseln("Sync Frame Detected! Bit Shift: %i", bitShift);
      uint32_t frame[16] = { 0 };

      for (uint8_t idx = 0; idx <= 63; idx++) {
        uint8_t codeWord = idx >> 2;
        frame[codeWord] <<= bitShift;
        frame[codeWord] |= rxByte & ((1 << bitShift) - 1);
        while (!available()) {}
        rxByte = read();
        frame[codeWord] <<= 8 - bitShift;
        frame[codeWord] |= rxByte >> bitShift;
        if (idx == 63 && bitShift != 0) {
          searchSync(rxByte);
        }
      }

      String postValue = "";

      for (uint8_t idx = 0; idx <= 15; idx++) {
        errors currentError = bch.decode(frame[idx]);

        if (frame[idx] == 0x7a89c197) {
          isIdle = true;
        } else {
          isIdle = false;
        }

        if (!(frame[idx] & (1 << 31))) {
          isAddress = true;
        } else {
          isAddress = false;
        }

        if (isAddress && isMessageRun && (!isIdle)) {
          isMessageRun = false;
          messageReceived();
          timerRx = millis() + 1000;
        }

        error.corrected += currentError.corrected;
        error.uncorrected += currentError.uncorrected;
        _errorCount.corrected += currentError.corrected;
        _errorCount.uncorrected += currentError.uncorrected;

        Log.verbose("%02u: ", idx);
        for (int8_t bit = 31; bit >= 0; bit--) {
          if (bit == 30 || (isAddress && bit == 12) || bit == 10 || bit == 0) {
            Log.verbose(" ");
          }
          if (frame[idx] & (1 << bit)) {
            Log.verbose("1");
          } else {
            Log.verbose("0");
          }
        }
        if (isIdle) {
          Log.verboseln(" Idle");
        }
        if (isAddress) {
          Log.verboseln(" Address");
        } else {
          Log.verboseln(" Message");
        }
        Log.verboseln(" BCH Error %i/%i", currentError.corrected, currentError.uncorrected);

        if (isAddress && (!isIdle)) {
          ric = ((frame[idx] & 0x7fffe000) >> 10) | (idx >> 1);
          Log.traceln("  RIC: %i", ric);
          function = (frame[idx] & 0x1800) >> 11;
        }
        if (isAddress) {
          character = 0;
          digit = 0;
          textPos = 0;
          numberPos = 0;
        }
        if (!isIdle) { isMessageRun = true; }

        if (!isAddress) {
          for (uint8_t bitPos = 30; bitPos >= 11; bitPos--) {
            character >>= 1;
            character |= (frame[idx] & (1 << bitPos)) >> (bitPos - 7);
            digit >>= 1;
            digit |= (frame[idx] & (1 << bitPos)) >> (bitPos - 7);
            textPos++;
            numberPos++;
            if (textPos >= 7) {
              character >>= 1;
              messageAlpha += (char)character;
              character = 0;
              textPos = 0;
            }
            if (numberPos >= 4) {
              digit >>= 4;
              messageNumeric += String(bcdCodes[digit]);
              digit = 0;
              numberPos = 0;
            }
          }
        }
      }
    }
  }

  String PagerReceiver::guessFormat(String alpha, String numeric) {
    int alphaSmell = 0;
    const char* alphaChars = alpha.c_str();
    for (int i = 0; *(alphaChars + i); i++) {
      unsigned char cp = *(alphaChars + i);
      if ((cp > 0 && cp < 32) || cp == 127) {
        alphaSmell -= 5;
      } else if ((cp > 31 && cp < 48) || (cp > 57 && cp < 65) || (cp > 90 && cp < 97) || (cp > 122 && cp < 127)) {
        alphaSmell -= 1;
      }
    }
    int numberSmell = 0;
    const char* numberDigits = numeric.c_str();
    for (int i = 0; *(numberDigits + i); i++) {
      unsigned char cp = *(numberDigits + i);
      if (cp == '*') {
        numberSmell -= 10;
      } else if (cp == 'U' || cp == '(' || cp == ')') {
        numberSmell -= 3;
      } else if (cp == ' ' || cp == '-') {
        numberSmell -= 1;
      }
    }
    if (numeric.length() > 15) {
      numberSmell -= 15;
    }
    if (alpha.length() < 3) {
      alphaSmell -= 15;
    }
    if (numberSmell > alphaSmell) {
      return numeric;
    } else {
      return alpha;
    }
  }