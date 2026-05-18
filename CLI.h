#ifndef BUZZBY_CLI_H
#define BUZZBY_CLI_H

#include "Settings.h"

String cmdLine="";

void help() {
  Log.infoln("debug [0-3]");
  Log.infoln("get version");
  Log.infoln("get status");
  Log.infoln("get configuration");
  Log.infoln("set frequency [137.000-525.000]");
  Log.infoln("set bitrate [0.075-250]");
  Log.infoln("restart cpu");
  Log.infoln("show settings");
  Log.infoln("load settings");
  Log.infoln("save settings");
  Log.infoln("erase settings");
  Log.infoln("tildagon reset");
  Log.infoln("exit");
  Log.infoln("help");
}

void parseCommand() {
  cmdLine.trim();
  if (cmdLine!="") {
    Log.infoln("");
  }
  String value=cmdLine.substring(cmdLine.lastIndexOf(" ")+1);
  if (cmdLine.startsWith("deb")) {
    Log.setLevel(LOG_LEVEL_WARNING + value.toInt());
    Log.infoln("Debug Level: %i", Log.getLevel() - LOG_LEVEL_WARNING);
  } else if (cmdLine.startsWith("get ver")) {
    pagerRx.printRadioHardwareDetails();
  } else if (cmdLine.startsWith("get stat")) {
    pagerRx.printStats();
  } else if (cmdLine.startsWith("get conf")) {
    Log.infoln("Frequency: %s MHz", String(pagerRx.getFrequency(), 5).c_str());
    Log.infoln("Bitrate: %s kbps", String(pagerRx.getBitrate(), 3).c_str());
  } else if (cmdLine.startsWith("set freq")) {
    pagerRx.updateSettings(value.toDouble(), pagerRx.getBitrate());
  } else if (cmdLine.startsWith("set bitrate")) {
    pagerRx.updateSettings(pagerRx.getFrequency(), value.toDouble());
  } else if (cmdLine.startsWith("restart cpu")) {
    rp2040.restart();
  } else if (cmdLine.startsWith("show set")) {
    showSettings();
  } else if (cmdLine.startsWith("load set")) {
    loadSettings();
  } else if (cmdLine.startsWith("save set")) {
    saveSettings();
  } else if (cmdLine.startsWith("erase set")) {
    eraseSettings();
  } else if (cmdLine.startsWith("tildagon reset")) {
    tildagonTeardown();
    tildagonSetup();
    Log.infoln("Link to tildagon is %s", tildagonSetupComplete ? "up" : "down");
  } else if (cmdLine.startsWith("exit")) {
    Log.setLevel(LOG_LEVEL_SILENT);
  } else if (cmdLine.startsWith("help")) {
    help();
  }
  Log.info("> ");
}

void cliWorker() {
  if (Serial.available()) {
    char serialByte=Serial.read();
    if (serialByte==127 && cmdLine.length() > 0) {
      Log.info("\b");
      cmdLine.remove(cmdLine.length()-1);
    } else if (serialByte==10 || serialByte==13) {
      Log.infoln("");
      parseCommand();
      cmdLine="";
    } else if (serialByte > 31 && serialByte < 127) {
      Serial.write(serialByte);
      cmdLine+=String(serialByte);
    }
  }
}

#endif
