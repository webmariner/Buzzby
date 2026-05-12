#ifndef BUZZBY_SETTINGS_H
#define BUZZBY_SETTINGS_H

#include <Preferences.h>
Preferences flash;

void showSettings() {
  flash.begin("SX1278FSK",true);
  Log.infoln("Center Frequency: %s MHz",String(flash.getDouble("centerFreq",439.9875),5).c_str());
  Log.infoln("Rx Frequency Offset: %s kHz",String(flash.getDouble("rxOffset",0.0),3).c_str());
  Log.infoln("Bitrate: %s bps",String(flash.getDouble("bitrate",1.2)*1000,0).c_str());
  Log.infoln("Shift Frequency: +/- %s Hz",String(flash.getDouble("shift",4.5)*1000,0).c_str());
  Log.infoln("Rx Bandwidth: %s kHz",String(flash.getDouble("rxBandwidth",5.2),1).c_str());
  Log.infoln("AFC Bandwidth: %s kHz",String(flash.getDouble("afcBandwidth",25),1).c_str());
  flash.end();
}

void loadSettings() {
  flash.begin("SX1278FSK",true);
  modem.stopSequencer();
  modem.setFrequency(flash.getDouble("centerFreq",439.9875),flash.getDouble("rxOffset",0.0));
  modem.setBitrate(flash.getDouble("bitrate",1.2));
  modem.setShift(flash.getDouble("shift",4.5));
  modem.setRxBandwidth(flash.getDouble("rxBandwidth",5.2));
  modem.setAfcBandwidth(flash.getDouble("afcBandwidth",25));
  modem.startSequencer();
  modem.restartRx(true);
  flash.end();
  Log.infoln("Flash: read");
}

void saveSettings() {
  flash.begin("SX1278FSK",false);
  flash.putDouble("centerFreq",modem.centerFreq);
  flash.putDouble("rxOffset",modem.rxOffset);
  flash.putDouble("bitrate",modem.bitrate);
  flash.putDouble("shift",modem.shift);
  flash.putDouble("rxBandwidth",modem.rxBandwidth);
  flash.putDouble("afcBandwidth",modem.afcBandwidth);
  flash.end();
  Log.infoln("Flash: written");
}

void eraseSettings() {
  flash.begin("SX1278FSK",false);
  flash.clear();
  Log.infoln("Flash: erased");
}

#endif