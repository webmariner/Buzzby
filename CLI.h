#ifndef BUZZBY_CLI_H
#define BUZZBY_CLI_H

#include <Arduino.h>
#include "OutputHandler.h"
#include "BuzzbyController.h"

class CLI {
public:
  void setup(SerialUSB* ser, Print* printer, BuzzbyController* con);
  void cliWorker();
private:
  void help();
  void parseCommand();
  String cmdLine="";
  SerialUSB* _ser;
  OutputHandler _out;
  BuzzbyController* _control;
};

#endif
