#ifndef BUZZBY_CONTROLLER_H
#define BUZZBY_CONTROLLER_H

#include "PagerReceiver.h"
#include "PagerQueue.h"
#include "ChannelMemory.h"
#include "OutputHandler.h"

class BuzzbyController {
public:
  void setup(Print* printer);
  void printHwDetails();
  void printStats();
  void printSettings();
  void printCurrentMessage();
  void setFrequency(double newFrequency);
  void setBitrate(double newBitrate);
  double getFrequency();
  double getBitrate();
  void restart();
  bool showChannel(const char* channelName);
  bool selectChannel(const char* channelName);
  void saveChannel(const char* channelName);
  void eraseChannel(const char* channelName);
  void next();
  void save();
  const PagerMessage& getCurrentMsg() const;
  bool messagesWaiting();
  void markAsRead();
  void loop();
private:
  PagerReceiver _pagerRx;
  ChannelMemory _channels;
  PagerQueue _pagerq;
  const char* _currentChannel;
  uint8_t _channelNumber;
  OutputHandler _out;
};

#endif // BUZZBY_CONTROLLER_H