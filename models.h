#ifndef MODELS_H
#define MODELS_H

#include <cstdint>
#include <Arduino.h>

struct Channel {
  double frequency_MHz;
  double bitrate_kbps;
};

struct PagerMessage {
  String text;
  uint32_t ric;
};


#endif // MODELS_H