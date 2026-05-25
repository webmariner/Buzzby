#include "SX1278.h"
  SX1278::SX1278(double frequency_MHz, double bitrate_kbps, double shift_kHz, double rxBandwidth_kHz, double afcBandwidth_kHz)
  	: _frequency{ frequency_MHz }
	  , _bitrate{ bitrate_kbps }
	  , _shift { shift_kHz }
	  , _rxBandwidth{ rxBandwidth_kHz }
	  , _afcBandwidth{ afcBandwidth_kHz }
  {}

  void SX1278::setup() {
    // Setup IO pins to communicate with the SX1278 module
    SPI.end();
    pinMode(SX1278_RST, OUTPUT);
    pinMode(SX1278_CS, OUTPUT);
    pinMode(SX1278_DIO0, INPUT);
    pinMode(SX1278_DIO1, INPUT);
    pinMode(SX1278_DIO2, INPUT);
    pinMode(SX1278_DIO3, INPUT);
    pinMode(SX1278_SCK, OUTPUT);
    pinMode(SX1278_MISO, INPUT);
    pinMode(SX1278_MOSI, OUTPUT);
    digitalWrite(SX1278_RST, HIGH);
    digitalWrite(SX1278_CS, HIGH);
    SPI.setSCK(SX1278_SCK);
    SPI.setMISO(SX1278_MISO);
    SPI.setMOSI(SX1278_MOSI);
    SPI.setCS(SX1278_CS);
    SPI.begin(true);
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
    setFrequency(_frequency);
    setBitrate(_bitrate);
    setShift(_shift);
    setRxBandwidth(_rxBandwidth);
    setAfcBandwidth(_afcBandwidth);
  }

  uint8_t SX1278::readSPI(uint8_t addr) {
    digitalWrite(SX1278_CS, LOW);
    SPI.transfer(addr);
    uint8_t value = SPI.transfer(0x00);
    digitalWrite(SX1278_CS, HIGH);
    return value;
  }

  void SX1278::writeSPI(uint8_t addr, uint8_t value) {
    digitalWrite(SX1278_CS, LOW);
    SPI.transfer(addr | 0x80);
    SPI.transfer(value);
    digitalWrite(SX1278_CS, HIGH);
  }

  void SX1278::setReg(uint8_t addr, uint8_t msb, uint8_t lsb, uint8_t value) {
    uint8_t oldValue = readSPI(addr);
    uint16_t mask = (1 << (msb - lsb + 1)) - 1;
    value &= mask;
    mask <<= lsb;
    value <<= lsb;
    uint8_t newValue = (oldValue & (~mask)) | value;
    writeSPI(addr, newValue);
  }

  uint8_t SX1278::getReg(uint8_t addr, uint8_t msb, uint8_t lsb) {
    uint8_t value = readSPI(addr);
    uint16_t mask = (1 << (msb - lsb + 1)) - 1;
    return (value & (mask << lsb)) >> lsb;
  }

  void SX1278::resetChip() {
    digitalWrite(SX1278_RST, LOW);
    delay(1);
    digitalWrite(SX1278_RST, HIGH);
    delay(25);
  }

  void SX1278::setFrequency(double frequency_MHz) {
    _frequency = frequency_MHz;
    Log.infoln("Frequency: %s MHz", String(_frequency, 5).c_str());
    uint32_t value = (uint32_t)round(_frequency * (1 << 14));
    writeSPI(regFreqMSB, (value & 0xFF0000) >> 16);
    writeSPI(regFreqMID, (value & 0x00FF00) >> 8);
    writeSPI(regFreqLSB, value & 0x0000FF);
  }

  void SX1278::setBitrate(double bitrate_kbps) {
    _bitrate = bitrate_kbps;
    Log.infoln("Bitrate: %s kbps", String(_bitrate, 3).c_str());
    uint16_t value = (uint16_t)round(32000.0 / _bitrate);
    writeSPI(regBrMSB, (value & 0xFF00) >> 8);
    writeSPI(regBrLSB, value & 0x00FF);
  }

  void SX1278::setShift(double shift_kHz) {
    _shift = shift_kHz;
    Log.infoln("Shift: +/- %s kHz", String(_shift, 3).c_str());
    uint16_t value = (uint16_t)round(_shift * (1 << 11) / 125.0);
    writeSPI(regShiftMSB, (value & 0xFF00) >> 8);
    writeSPI(regShiftLSB, value & 0x00FF);
  }

  void SX1278::setRxBandwidth(double rxBandwidth_kHz) {
    uint8_t selected = 20;
    for (uint8_t idx = 0; idx <= 20; idx++) {
      if (bwValues[idx] >= rxBandwidth_kHz) {
        selected = idx;
        break;
      }
    }
    Log.infoln("Rx Bandwidth: %s kHz", String(bwValues[selected], 1).c_str());
    setReg(regRxBw, 4, 0, bwIntegers[selected]);
    _rxBandwidth = bwValues[selected];
  }

  void SX1278::setRxBwAuto() {
    setRxBandwidth(_shift + (_bitrate / 2));
  }

  void SX1278::setAfcBandwidth(double afcBandwidth_kHz) {
    uint8_t selected = 20;
    for (uint8_t idx = 0; idx <= 20; idx++) {
      if (bwValues[idx] >= afcBandwidth_kHz) {
        selected = idx;
        break;
      }
    }
    Log.infoln("AFC Bandwidth: %s kHz", String(bwValues[selected], 1).c_str());
    setReg(regAfcBw, 4, 0, bwIntegers[selected]);
    _afcBandwidth = bwValues[selected];
  }

  void SX1278::setAfcBwAuto(double error) {
    setAfcBandwidth(2 * (_shift + (_bitrate / 2)) + error);
  }

  void SX1278::restartRx(bool withPLL) {
    if (!withPLL) {
      setReg(regRxCfg, 6, 6, 1);
    } else {
      setReg(regRxCfg, 5, 5, 1);
    }
  }

  double SX1278::getFrequency() {
    return _frequency;
  }

  double SX1278::getBitrate() {
    return _bitrate;
  }

  double SX1278::getAFC() {
    uint8_t valueMSB = readSPI(regAfcMSB);
    uint8_t valueLSB = readSPI(regAfcLSB);
    int16_t value = (valueMSB << 8) | valueLSB;
    return (double)value / 16.384;
  }

  double SX1278::getFEI() {
    uint8_t valueMSB = readSPI(regFeiMSB);
    uint8_t valueLSB = readSPI(regFeiLSB);
    int16_t value = (valueMSB << 8) | valueLSB;
    return (double)value / 16.384;
  }

  double SX1278::getGain() {
    return gainValues[getReg(regRxLna, 7, 5)];
  }

  void SX1278::setRssiThreshold(int threshold) {
    writeSPI(regRssiThreshold, (uint8_t)(threshold * -2));
  }

  double SX1278::getRSSI() {
    return readSPI(regRssi) / -2.0;
  }

  void SX1278::printHwVersion(OutputHandler out) {
    out.println("SX1278 Version: %i Hardware Revision: %i", getReg(regChipVersion, 7, 4), getReg(regChipVersion, 3, 0));
  }

  void SX1278::logCurrentRxStats() {
    Log.infoln("RSSI: %s dBm, Gain: %s dBm, AFC: %s kHz, FEI: %s kHz", String(getRSSI(), 1).c_str(), String(getGain(), 1).c_str(), String(getAFC(), 3).c_str(), String(getFEI(), 3).c_str());
  }

  void SX1278::printCurrentRxStats(OutputHandler out) {
    out.println("RSSI: %s dBm, Gain: %s dBm, AFC: %s kHz, FEI: %s kHz", String(getRSSI(), 1).c_str(), String(getGain(), 1).c_str(), String(getAFC(), 3).c_str(), String(getFEI(), 3).c_str());
  }