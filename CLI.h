#ifndef BUZZBY_CLI_H
#define BUZZBY_CLI_H

#include "Flash.h"

String cmdLine="";

void help() {
  Log.infoln("debug [0-3]");
  Log.infoln("monitor");
  Log.infoln("get version");
  Log.infoln("get status");
  Log.infoln("clear status");
  Log.infoln("get configuration");
  Log.infoln("get register");
  Log.infoln("set frequency [137.000-525.000]");
  Log.infoln("set offset [0-100|auto]");
  Log.infoln("set bitrate [0.075-250]");
  Log.infoln("set shift [0.6-200]");
  Log.infoln("set rxbw [2.6-250|auto]");
  Log.infoln("set afcbw [2.6-250|auto]");
  Log.infoln("restart rx");
  Log.infoln("restart cpu");
  Log.infoln("get flash");
  Log.infoln("read flash");
  Log.infoln("write flash");
  Log.infoln("erase flash");
  Log.infoln("tildagon reset");
  Log.infoln("exit");
  Log.infoln("help");
}

void doParse() {
  cmdLine.trim();
  if (cmdLine!="") {
    Log.infoln("");
  }
  String value=cmdLine.substring(cmdLine.lastIndexOf(" ")+1);
  if (cmdLine.startsWith("deb")) {
    Log.setLevel(LOG_LEVEL_WARNING + value.toInt());
    Log.infoln("Debug Level: %i",Log.getLevel() - LOG_LEVEL_WARNING);
  } else if (cmdLine.startsWith("mon")) {
    modem.monitorRx=!modem.monitorRx;
  } else if (cmdLine.startsWith("get ver")) {
    modem.printChip();
  } else if (cmdLine.startsWith("get stat")) {
    modem.printRx();
    Log.info("Messages received: %i",modem.messageCount);
    Log.info("   Errors corrected: %i",modem.errorCount.corrected);
    Log.info("   uncorrected: %i",modem.errorCount.uncorrected);
    Log.infoln("   Bytes queued: %i/%i",uxQueueMessagesWaitingFromISR(queueDIO1),queueSizeDIO1);
    Log.info("   Monitor: %i",modem.monitorRx);
    Log.infoln("   Debug: %i",Log.getLevel() - LOG_LEVEL_WARNING);
    Log.infoln("Uptime: %i days %s hours",modem.upTime/86400,String((double)(modem.upTime%86400)/3600.0,2).c_str());
  } else if (cmdLine.startsWith("clear stat")) {
    modem.messageCount=0;
    modem.errorCount.corrected=0;
    modem.errorCount.uncorrected=0;
    Log.infoln("Statistics cleared");
  } else if (cmdLine.startsWith("get conf")) {
    Log.infoln("Center Frequency: %s MHz",String(modem.centerFreq,5).c_str());
    Log.infoln("Rx Frequency Offset: %s kHz",String(modem.rxOffset,3).c_str());
    Log.infoln("Bitrate: %s bps",String(modem.bitrate*1000,0).c_str());
    Log.infoln("Shift Frequency: +/- %s Hz",String(modem.shift*1000,0).c_str());
    Log.infoln("Rx Bandwidth: %s kHz",String(modem.rxBandwidth,1).c_str());
    Log.infoln("AFC Bandwidth: %s kHz",String(modem.afcBandwidth,1).c_str());
  }
  else if (cmdLine.startsWith("get reg")) {
    modem.regDump();
  } else if (cmdLine.startsWith("set freq")) {
    modem.stopSequencer();
    modem.setFrequency(value.toDouble());
    modem.startSequencer();
    modem.restartRx(true);
  } else if (cmdLine.startsWith("set offset")) {
    if (value!="auto") {
      modem.rxOffset=value.toDouble();
    }
    modem.stopSequencer();
    modem.setFrequency(modem.centerFreq,modem.rxOffset);
    modem.startSequencer();
    modem.restartRx(true);
  } else if (cmdLine.startsWith("set bitrate")) {
    modem.setBitrate(value.toDouble());
  } else if (cmdLine.startsWith("set shift")) {
    modem.stopSequencer();
    modem.setShift(value.toDouble());
    modem.startSequencer();
    modem.restartRx(true);
  } else if (cmdLine.startsWith("set rxbw")) {
    if (value=="auto") {
      modem.setRxBwAuto();
    } else {
      modem.setRxBandwidth(value.toDouble());
    }
  } else if (cmdLine.startsWith("set afcbw")) {
    if (value=="auto") {
      modem.setAfcBwAuto();
    } else {
      modem.setAfcBandwidth(value.toDouble());
    }
  } else if (cmdLine.startsWith("restart rx")) {
    modem.restartRx(true);
    Log.infoln("Rx and PLL restarted");
  } else if (cmdLine.startsWith("restart cpu")) {
    rp2040.restart();
  } else if (cmdLine.startsWith("get flash")) {
    getFlash();
  } else if (cmdLine.startsWith("read flash")) {
    readFlash();
  } else if (cmdLine.startsWith("write flash")) {
    writeFlash();
  } else if (cmdLine.startsWith("erase flash")) {
    eraseFlash();
  } else if (cmdLine.startsWith("tildagon reset")) {
    tildagonTeardown();
    tildagonSetup();
    Log.infoln("Link to tildagon is %s", tildagonSetupComplete ? "up" : "down");
  } else if (cmdLine.startsWith("exit")) {
    modem.monitorRx = false;
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
      doParse();
      cmdLine="";
    } else if (serialByte > 31 && serialByte < 127) {
      Serial.write(serialByte);
      cmdLine+=String(serialByte);
    }
  }
}

#endif
