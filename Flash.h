#ifndef BUZZBY_FLASH_H
#define BUZZBY_FLASH_H

#include <Preferences.h>

Preferences flash;

void getFlash() {
  flash.begin("SX1278FSK",true);
  Log.print(0,"Center Frequency: %s MHz\r\n",String(flash.getDouble("centerFreq",439.9875),5).c_str());
  Log.print(0,"Rx Frequency Offset: %s kHz\r\n",String(flash.getDouble("rxOffset",0.0),3).c_str());
  Log.print(0,"Bitrate: %s bps\r\n",String(flash.getDouble("bitrate",1.2)*1000,0).c_str());
  Log.print(0,"Shift Frequency: +/- %s Hz\r\n",String(flash.getDouble("shift",4.5)*1000,0).c_str());
  Log.print(0,"Rx Bandwidth: %s kHz\r\n",String(flash.getDouble("rxBandwidth",5.2),1).c_str());
  Log.print(0,"AFC Bandwidth: %s kHz\r\n",String(flash.getDouble("afcBandwidth",25),1).c_str());
  flash.end();
}

void readFlash() {
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
  Log.print(0,"Flash: read\r\n");
}

void writeFlash() {
  flash.begin("SX1278FSK",false);
  flash.putDouble("centerFreq",modem.centerFreq);
  flash.putDouble("rxOffset",modem.rxOffset);
  flash.putDouble("bitrate",modem.bitrate);
  flash.putDouble("shift",modem.shift);
  flash.putDouble("rxBandwidth",modem.rxBandwidth);
  flash.putDouble("afcBandwidth",modem.afcBandwidth);
  flash.end();
  Log.print(0,"Flash: written\r\n");
}

void eraseFlash() {
  flash.begin("SX1278FSK",false);
  flash.clear();
  Log.print(0,"Flash: erased\r\n");
}

#endif