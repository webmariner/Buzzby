#ifndef BUZZBY_CLI_H
#define BUZZBY_CLI_H

#include "Flash.h"

String cmdLine="";

void help() {
  Log.print(0,"debug [0-3]\r\n");
  Log.print(0,"monitor\r\n");
  Log.print(0,"get version\r\n");
  Log.print(0,"get status\r\n");
  Log.print(0,"clear status\r\n");
  Log.print(0,"get configuration\r\n");
  Log.print(0,"get register\r\n");
  Log.print(0,"set frequency [137.000-525.000]\r\n");
  Log.print(0,"set offset [0-100|auto]\r\n");
  Log.print(0,"set bitrate [0.075-250]\r\n");
  Log.print(0,"set shift [0.6-200]\r\n");
  Log.print(0,"set rxbw [2.6-250|auto]\r\n");
  Log.print(0,"set afcbw [2.6-250|auto]\r\n");
  Log.print(0,"restart rx\r\n");
  Log.print(0,"restart cpu\r\n");
  Log.print(0,"get flash\r\n");
  Log.print(0,"read flash\r\n");
  Log.print(0,"write flash\r\n");
  Log.print(0,"erase flash\r\n");
  Log.print(0,"tildagon reset\r\n");
  Log.print(0,"exit\r\n");
  Log.print(0,"help\r\n");
}

void doParse() {
  cmdLine.trim();
  if (cmdLine!="") {
    Log.print(0,"\r\n");
  }
  String value=cmdLine.substring(cmdLine.lastIndexOf(" ")+1);
  if (cmdLine.startsWith("deb")) {
    Log.debug=value.toInt();
    Log.print(0,"Debug Level: %i\r\n",Log.debug);
  } else if (cmdLine.startsWith("mon")) {
    modem.monitorRx=!modem.monitorRx;
  } else if (cmdLine.startsWith("get ver")) {
    modem.printChip();
  } else if (cmdLine.startsWith("get stat")) {
    modem.printRx();
    Log.print(0,"Messages received: %i",modem.messageCount);
    Log.print(0,"   Errors corrected: %i",modem.errorCount.corrected);
    Log.print(0,"   uncorrected: %i",modem.errorCount.uncorrected);
    Log.print(0,"   Bytes queued: %i/%i\r\n",uxQueueMessagesWaitingFromISR(queueDIO1),queueSizeDIO1);
    Log.print(0,"   Monitor: %i",modem.monitorRx);
    Log.print(0,"   Debug: %i\r\n",Log.debug);
    Log.print(0,"Uptime: %i days %s hours\r\n",modem.upTime/86400,String((double)(modem.upTime%86400)/3600.0,2).c_str());
  } else if (cmdLine.startsWith("clear stat")) {
    modem.messageCount=0;
    modem.errorCount.corrected=0;
    modem.errorCount.uncorrected=0;
    Log.print(0,"Statistics cleared\r\n");
  } else if (cmdLine.startsWith("get conf")) {
    Log.print(0,"Center Frequency: %s MHz\r\n",String(modem.centerFreq,5).c_str());
    Log.print(0,"Rx Frequency Offset: %s kHz\r\n",String(modem.rxOffset,3).c_str());
    Log.print(0,"Bitrate: %s bps\r\n",String(modem.bitrate*1000,0).c_str());
    Log.print(0,"Shift Frequency: +/- %s Hz\r\n",String(modem.shift*1000,0).c_str());
    Log.print(0,"Rx Bandwidth: %s kHz\r\n",String(modem.rxBandwidth,1).c_str());
    Log.print(0,"AFC Bandwidth: %s kHz\r\n",String(modem.afcBandwidth,1).c_str());
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
    Log.print(0,"Rx and PLL restarted\r\n");
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
    Log.print(0,"Link to tildagon is %s\r\n", tildagonSetupComplete?"up":"down");
  } else if (cmdLine.startsWith("exit")) {
    modem.monitorRx=false;
    Log.debug=0;
  } else if (cmdLine.startsWith("help")) {
    help();
  }
  Log.print(0,"> ");
  Log.nextLogOnNewLine=true;
}

void cliWorker() {
  if (Serial.available()) {
    char serialByte=Serial.read();
    if (serialByte==127 && cmdLine.length() > 0) {
      Log.print(0,"\b");
      cmdLine.remove(cmdLine.length()-1);
    } else if (serialByte==10 || serialByte==13) {
      Log.nextLogOnNewLine=false;
      Log.print(0,"\r\n");
      doParse();
      cmdLine="";
    } else {
      Log.write(0,serialByte);
      cmdLine+=String(serialByte);
    }
  }
}

#endif
