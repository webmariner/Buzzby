#include "CLI.h"

void CLI::setup(SerialUSB* ser, Print* printer, BuzzbyController* con) {
  _ser = ser;
  _out.setup(printer);
  _control = con;
}

void CLI::help() {
  _out.println("debug [0-3]");
  _out.println("show version");
  _out.println("show stats");
  _out.println("show configuration");
  _out.println("show msg");
  _out.println("set frequency [137.000-525.000]");
  _out.println("set bitrate [0.075-250]");
  _out.println("restart cpu");
  _out.println("show channel [channel name]");
  _out.println("select channel [channel name]");
  _out.println("save channel [channel name]");
  _out.println("erase channel [channel name]");
  _out.println("next");
  _out.println("tildagon reset");
  _out.println("help");
}

void CLI::parseCommand() {
  cmdLine.trim();
  if (cmdLine!="") {
    _out.println("");
  }
  String value=cmdLine.substring(cmdLine.lastIndexOf(" ")+1);
  if (cmdLine.startsWith("deb")) {
    Log.setLevel(LOG_LEVEL_WARNING + value.toInt());
    _out.println("Debug Level: %i", Log.getLevel() - LOG_LEVEL_WARNING);
  } else if (cmdLine.startsWith("show ver")) {
    _control->printHwDetails();
  } else if (cmdLine.startsWith("show stat")) {
    _control->printStats();
  } else if (cmdLine.startsWith("show conf")) {
    _control->printSettings();
  } else if (cmdLine.startsWith("show msg")) {
    _control->printCurrentMessage();
  } else if (cmdLine.startsWith("set freq")) {
    _control->setFrequency(value.toDouble());
  } else if (cmdLine.startsWith("set bitrate")) {
    _control->setBitrate(value.toDouble());
  } else if (cmdLine.startsWith("restart cpu")) {
    _control->restart();
  } else if (cmdLine.startsWith("show ch")) {
    _control->showChannel(value.c_str());
  } else if (cmdLine.startsWith("select ch")) {
    _control->selectChannel(value.c_str());
  } else if (cmdLine.startsWith("save ch")) {
    _control->saveChannel(value.c_str());
  } else if (cmdLine.startsWith("erase ch")) {
    _control->eraseChannel(value.c_str());
  } else if (cmdLine.startsWith("nex")) {
    _control->next();
  //} else if (cmdLine.startsWith("tildagon reset")) {
  //  tildagonTeardown();
  //  tildagonSetup();
  //  _out.println("Link to tildagon is %s", tildagonSetupComplete ? "up" : "down");
  } else if (cmdLine.startsWith("help")) {
    help();
  } else {
    _out.println("Unknown command");
  }
  _out.print("> ");
}

void CLI::cliWorker() {
  if (_ser->available()) {
    char serialByte=_ser->read();
    if (serialByte==127 && cmdLine.length() > 0) {
      _ser->print("\b");
      cmdLine.remove(cmdLine.length()-1);
    } else if (serialByte==10 || serialByte==13) {
      _ser->println("");
      parseCommand();
      cmdLine="";
    } else if (serialByte > 31 && serialByte < 127) {
      _ser->write(serialByte);
      cmdLine+=String(serialByte);
    }
  }
}