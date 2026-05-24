#ifndef BUZZBY_SETTINGS_H
#define BUZZBY_SETTINGS_H

#include <Preferences.h>
Preferences prefs;

const char* FLASH_SETUP_WARNING = "Check flash is configured to allow a small filesystem for settings too, not all just reserved for this code";

void showSettings() {
  if (!prefs.begin("POCSAG", true)) {
    Log.warningln(FLASH_SETUP_WARNING);
  }
  Log.infoln("Frequency: %s MHz",String(prefs.getDouble("frequency"),5).c_str());
  Log.infoln("Bitrate: %s kbps",String(prefs.getDouble("bitrate"),3).c_str());
  prefs.end();
}

void loadSettings() {
  if (!prefs.begin("POCSAG", true)) {
    Log.warningln(FLASH_SETUP_WARNING);
  }
  pagerRx.updateSettings(prefs.getDouble("frequency", 439.9875), prefs.getDouble("bitrate", 1.2));
  prefs.end();
  Log.infoln("Settings loaded");
}

void saveSettings() {
  if (!prefs.begin("POCSAG", false)) {
    Log.warningln(FLASH_SETUP_WARNING);
  }
  double frequency = pagerRx.getFrequency();
  double bitrate = pagerRx.getBitrate();
  prefs.putDouble("frequency",frequency);
  prefs.putDouble("bitrate",bitrate);
  Log.infoln("Settings saved: Frequency %s MHz, Bitrate %s kbps",
    String(frequency, 5).c_str(),
    String(bitrate, 3).c_str()
  );
  prefs.end();
  Log.infoln("Settings saved");
}

void eraseSettings() {
  if (!prefs.begin("POCSAG", false)) {
    Log.warningln(FLASH_SETUP_WARNING);
  }
  prefs.clear();
  Log.infoln("Settings erased");
}

#endif