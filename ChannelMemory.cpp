#include "ChannelMemory.h"
#include "src/ArduinoLog.h"

const char* FLASH_SETUP_WARNING = "Error reserving space for a channel";

bool ChannelMemory::showChannel(const char* channelName) {
  if (!prefs.begin(channelName, true)) {
    Log.warningln(FLASH_SETUP_WARNING);
    prefs.end();
    return false;
  }
  if (!prefs.isKey("frequency")) {
    Log.infoln("Channel '%s' not found", channelName);
    prefs.end();
    return false;
  }
  Log.infoln("Frequency: %s MHz",String(prefs.getDouble("frequency"),5).c_str());
  Log.infoln("Bitrate: %s kbps",String(prefs.getDouble("bitrate"),3).c_str());
  prefs.end();
  return true;
}

std::optional<Channel> ChannelMemory::getChannel(const char* channelName) {
  if (!prefs.begin(channelName, true)) {
    Log.warningln(FLASH_SETUP_WARNING);
  }
  if (!prefs.isKey("frequency")) {
    Log.infoln("Tried loading channel %s but didn't exist", channelName);
    prefs.end();
    return std::nullopt;
  }
  Channel chan;
  chan.frequency_MHz = prefs.getDouble("frequency");
  chan.bitrate_kbps = prefs.getDouble("bitrate");
  prefs.end();
  return chan;
}

void ChannelMemory::saveChannel(const char* channelName, Channel& newChannelDetails) {
  if (!prefs.begin(channelName, false)) {
    Log.warningln(FLASH_SETUP_WARNING);
  }
  double frequency = newChannelDetails.frequency_MHz;
  double bitrate = newChannelDetails.bitrate_kbps;
  prefs.putDouble("frequency",frequency);
  prefs.putDouble("bitrate",bitrate);
  Log.infoln("Channel '%s' saved: Frequency %s MHz, Bitrate %s kbps",
    channelName,
    String(frequency, 5).c_str(),
    String(bitrate, 3).c_str()
  );
  prefs.end();
}

void ChannelMemory::eraseChannel(const char* channelName) {
  if (!prefs.begin(channelName, false)) {
    Log.warningln(FLASH_SETUP_WARNING);
  }
  prefs.clear();
  Log.infoln("Channel '%s' erased", channelName);
  prefs.end();
}