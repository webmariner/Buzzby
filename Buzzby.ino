#include "src/ArduinoLog.h"
#include "PagerQueue.h"
#include "PagerReceiver.h"
#include <USB.h>

PagerQueue pagerq;
PagerReceiver pagerRx;

#include "Tildagon.h"
#include "CLI.h"

void setup() {
  USB.disconnect();
  USB.setVIDPID(0xf055, 0x4d10);
  USB.setManufacturer("ECHQ");
  USB.setProduct("Radiolarian Buzzby Controller");
  USB.connect();
  Serial.begin(115200);
  Log.begin(LOG_LEVEL_WARNING, &Serial, false);
  pagerRx.setup(pagerq);
  loadSettings();
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
  pagerRx.pocsagWorker();
  tildagonLoop();
  cliWorker();
}
