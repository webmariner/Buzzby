#ifndef BUZZBY_SX1278ISR_H
#define BUZZBY_SX1278ISR_H

const pin_size_t SX1278_SCK = 18;
const pin_size_t SX1278_MISO = 16;
const pin_size_t SX1278_MOSI = 19;
const pin_size_t SX1278_CS = 17;
const pin_size_t SX1278_RST = 22;
const pin_size_t SX1278_DIO0 = 10;
const pin_size_t SX1278_DIO1 = 11;
const pin_size_t SX1278_DIO2 = 12;
const pin_size_t SX1278_DIO3 = 13;

volatile bool detectDIO0Flag=false;
volatile bool detectDIO3Flag=false;

//portMUX_TYPE mutexDIO0=portMUX_INITIALIZER_UNLOCKED;
xQueueHandle queueDIO1;
UBaseType_t queueSizeDIO1=1024;
//portMUX_TYPE mutexDIO3=portMUX_INITIALIZER_UNLOCKED;

void dio0ISR() {
  UBaseType_t uxSavedInterruptStatus;
  uxSavedInterruptStatus = portENTER_CRITICAL_FROM_ISR();
  detectDIO0Flag=true;
  portEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);
}

void dio1ISR() {
  static uint8_t buffer=0; static uint8_t bufferMask=128;
  if (digitalRead(SX1278_DIO2)==LOW) {
    buffer|=bufferMask;
  }
  bufferMask>>=1;
  if (bufferMask==0) {
    xQueueSendFromISR(queueDIO1,&buffer,NULL);
    buffer=0; bufferMask=128;
  }
}

void dio3ISR() {
  UBaseType_t uxSavedInterruptStatus;
  uxSavedInterruptStatus = portENTER_CRITICAL_FROM_ISR();
  detectDIO3Flag=true;
  portEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);
}

#endif
