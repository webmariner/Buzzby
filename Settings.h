#ifndef BUZZBY_SETTINGS_H
#define BUZZBY_SETTINGS_H

#include <Preferences.h>
Preferences prefs;

void showSettings() {
  prefs.begin("POCSAG", true);
  Log.infoln("Frequency: %s MHz",String(prefs.getDouble("frequency",439.9875),5).c_str());
  Log.infoln("Bitrate: %s bps",String(prefs.getDouble("bitrate",1.2)*1000,0).c_str());;
  prefs.end();
}

void loadSettings() {
  prefs.begin("POCSAG", true);
  pagerRx.updateSettings(prefs.getDouble("frequency", 439.9875), prefs.getDouble("bitrate", 1.2));
  prefs.end();
  Log.infoln("Settings loaded");
}

void saveSettings() {
  prefs.begin("POCSAG", false);
  prefs.putDouble("frequency",pagerRx.getFrequency());
  prefs.putDouble("bitrate",pagerRx.getBitrate());
  prefs.end();
  Log.infoln("Settings saved");
}

void eraseSettings() {
  prefs.begin("POCSAG", false);
  prefs.clear();
  Log.infoln("Settings erased");
}

#endif