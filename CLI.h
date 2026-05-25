#ifndef BUZZBY_CLI_H
#define BUZZBY_CLI_H

String cmdLine="";

void help() {
  Log.infoln("debug [0-3]");
  Log.infoln("show version");
  Log.infoln("show stats");
  Log.infoln("show configuration");
  Log.infoln("set frequency [137.000-525.000]");
  Log.infoln("set bitrate [0.075-250]");
  Log.infoln("restart cpu");
  Log.infoln("show channel [channel name]");
  Log.infoln("select channel [channel name]");
  Log.infoln("save channel [channel name]");
  Log.infoln("erase channel [channel name]");
  Log.infoln("next");
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
  } else if (cmdLine.startsWith("show ver")) {
    controller.printHwDetails();
  } else if (cmdLine.startsWith("show stat")) {
    controller.printStats();
  } else if (cmdLine.startsWith("show conf")) {
    controller.printSettings();
  } else if (cmdLine.startsWith("set freq")) {
    controller.setFrequency(value.toDouble());
  } else if (cmdLine.startsWith("set bitrate")) {
    controller.setBitrate(value.toDouble());
  } else if (cmdLine.startsWith("restart cpu")) {
    controller.restart();
  } else if (cmdLine.startsWith("show ch")) {
    controller.showChannel(value.c_str());
  } else if (cmdLine.startsWith("select ch")) {
    controller.selectChannel(value.c_str());
  } else if (cmdLine.startsWith("save ch")) {
    controller.saveChannel(value.c_str());
  } else if (cmdLine.startsWith("erase ch")) {
    controller.eraseChannel(value.c_str());
  } else if (cmdLine.startsWith("nex")) {
    controller.next();
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
