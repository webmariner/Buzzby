#include <USB.h>
#include "src/ArduinoLog.h"
#include "BuzzbyController.h"
#include "CLI.h"

BuzzbyController controller;

#include "Tildagon.h"

CLI cli;

void setup() {
  USB.disconnect();
  USB.setVIDPID(0xf055, 0x4d10);
  USB.setManufacturer("ECHQ");
  USB.setProduct("Radiolarian Buzzby Controller");
  USB.connect();
  Serial.begin(115200);
  Log.begin(LOG_LEVEL_WARNING, &Serial, false);
  controller.setup(&Serial);
  cli.setup(&Serial, &Serial, &controller);
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
  controller.loop();
  tildagonLoop();
  cli.cliWorker();
}
