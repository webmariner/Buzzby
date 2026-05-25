#ifndef PAGERRECEIVER_H
#define PAGERRECEIVER_H

#include "models.h"
#include "src/BCH3121.h"
#include "SX1278.h"
#include "PagerQueue.h"
#include "OutputHandler.h"


const double DEFAULT_FREQUENCY = 439.9875;
const double DEFAULT_BITRATE = 1.2;

struct bchErrorTotals {
  uint32_t corrected;
  uint32_t uncorrected;
};

const uint8_t pocsagSyncWord[4] = { 0x7c, 0xd2, 0x15, 0xd8 };
const char bcdCodes[16] = { 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x2a, 0x55, 0x20, 0x2d, 0x29, 0x28 };

class PagerReceiver {
public:
    void setup(PagerQueue pagerq);
    void updateSettings(double frequency, double bitrate);
    void pocsagWorker();
    double getFrequency();
    double getBitrate();
    void printRadioHardwareDetails(OutputHandler out);
    void printStats(OutputHandler out);
    PagerReceiver();
private:
    bchErrorTotals _errorCount;
    uint32_t _messageCount;
    SX1278* _radio;
    PagerQueue _pagerq;
    uint64_t timerRx;
    bool isText;
    bool isIdle;
    bool isAddress;
    errors error;
    char function;
    bool isMessageRun;
    uint32_t ric;
    String messageAlpha;
    String messageNumeric = "";
    bool parity;
    double rssi;
    uint8_t character = 0;
    uint8_t digit = 0;
    uint8_t textPos = 0;
    uint8_t numberPos = 0;
    void setModeFskRxCont();
    void startSequencer();
    void stopSequencer();
    bool available();
    uint8_t read();
    void messageReceived();
    uint8_t searchSync(uint8_t rxByte);
    String guessFormat(String alpha, String numeric);
};

#endif // PAGERRECEIVER_H
