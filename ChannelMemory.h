#ifndef BUZZBY_SETTINGS_H
#define BUZZBY_SETTINGS_H

#include <optional>
#include <Preferences.h>
#include "models.h"

class ChannelMemory {
public:
  bool showChannel(const char* channelName);
  std::optional<Channel> getChannel(const char* channelName);
  void saveChannel(const char* channelName, Channel& newChannelDetails);
  void eraseChannel(const char* channelName);
private:
  Preferences prefs;
};

#endif