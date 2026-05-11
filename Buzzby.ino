//#define __FREERTOS 1
//#include <FreeRTOS.h>
//#include <task.h>


#include "Log.h"
Logging Log;
#include "PagerQueue.h"
PagerQueue pager;
#include "SX1278.h"
SX1278FSK modem(false,0);
#include <USB.h>
#include "Tildagon.h"
#include "CLI.h"

void setup() {
  USB.disconnect();
  USB.setVIDPID(0xf055, 0x4d10);
  USB.setManufacturer("ECHQ");
  USB.setProduct("Radiolarian Buzzby Controller");
  USB.connect();
  modem.radioSetup();
  readFlash();
  tildagonSetup();
  delay(4000);
}

void loop() {
  if (BOOTSEL) {
    while (BOOTSEL) {
      delay(1);
    }
    rp2040.rebootToBootloader();
  }
  modem.pocsagWorker();
  tildagonLoop();
  cliWorker();
}
