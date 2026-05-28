#include <string>
#include "BuzzbyController.h"
#include "src/ArduinoLog.h"

const char* CHANNEL_NOT_SET = "<not set>";
const char* STANDARD_CHANNEL_NAMES[10] = {
	"POCSAG0", "POCSAG1", "POCSAG2", "POCSAG3", "POCSAG4",
	"POCSAG5", "POCSAG6", "POCSAG7", "POCSAG8", "POCSAG9"
};
const char* DEFAULT_CHANNEL = STANDARD_CHANNEL_NAMES[0];

void BuzzbyController::setup(Print* printer) {
	_out.setup(printer);
	_pagerRx.setup();
	_currentChannel = CHANNEL_NOT_SET;
	_channelNumber = 0;
	if (!selectChannel(DEFAULT_CHANNEL)) {
		saveChannel(DEFAULT_CHANNEL);
	}
}

void BuzzbyController::printHwDetails() {
	_pagerRx.printRadioHardwareDetails(_out);
}

void BuzzbyController::printStats() {
	_pagerRx.printStats(_out);
}

void BuzzbyController::printSettings() {
	_out.println("Current channel/memory: %s", _currentChannel);
    _out.println("Frequency: %s MHz", String(_pagerRx.getFrequency(), 5).c_str());
    _out.println("Bitrate: %s kbps", String(_pagerRx.getBitrate(), 3).c_str());
}

void BuzzbyController::printCurrentMessage() {
	if (_pagerRx.getMessageQueue().messagesWaiting()) {
		_out.println(_pagerRx.getMessageQueue().currentMessage().text.c_str());
	} else {
		_out.println("No messages waiting");
	}
}

void BuzzbyController::setFrequency(double newFrequency) {
	_currentChannel = CHANNEL_NOT_SET;
	_pagerRx.updateSettings(newFrequency, _pagerRx.getBitrate());
}

void BuzzbyController::setBitrate(double newBitrate) {
	_currentChannel = CHANNEL_NOT_SET;
	_pagerRx.updateSettings(_pagerRx.getFrequency(), newBitrate);
}

double BuzzbyController::getFrequency() {
	return _pagerRx.getFrequency();
}

double BuzzbyController::getBitrate() {
	return _pagerRx.getBitrate();
}

void BuzzbyController::restart() {
	rp2040.restart();
}

bool BuzzbyController::showChannel(const char* channelName) {
	return _channels.showChannel(channelName, _out);
}

bool BuzzbyController::selectChannel(const char* channelName) {
	auto c = _channels.getChannel(channelName);
	if (c.has_value()) {
		Channel ch = c.value();
		_pagerRx.updateSettings(ch.frequency_MHz, ch.bitrate_kbps);
		_currentChannel = channelName;
		return true;
	}
	return false;
}

void BuzzbyController::saveChannel(const char* channelName) {
	Channel c;
	c.frequency_MHz = _pagerRx.getFrequency();
	c.bitrate_kbps = _pagerRx.getBitrate();
	_channels.saveChannel(channelName, c);
	_currentChannel = channelName;
}

void BuzzbyController::eraseChannel(const char* channelName) {
	_channels.eraseChannel(channelName);
	if (strcmp(channelName, _currentChannel) == 0) {
		_currentChannel = CHANNEL_NOT_SET;
	}
}

void BuzzbyController::next() {
	bool channelSelected = false;
	bool looped = false;
	do {
		_channelNumber++;
		if (_channelNumber > 9) {
			_channelNumber = 0;
			looped = true;
		}
		channelSelected = selectChannel(STANDARD_CHANNEL_NAMES[_channelNumber]);
	} while (!(channelSelected | looped));
}

void BuzzbyController::save() {
	saveChannel(STANDARD_CHANNEL_NAMES[_channelNumber]);
}

PagerMessage BuzzbyController::getCurrentMsg() {
	PagerMessage copy;
	String text = _pagerRx.getMessageQueue().currentMessage().text;
	int length = text.length();
	char textCopy[length + 1];
	strcpy(textCopy, text.c_str());
	copy.text = textCopy;
	copy.ric = _pagerRx.getMessageQueue().currentMessage().ric;
	return copy;
}

bool BuzzbyController::messagesWaiting() {
	return _pagerRx.getMessageQueue().messagesWaiting();
}

void BuzzbyController::markAsRead() {
	_pagerRx.getMessageQueue().markAsRead();
}

void BuzzbyController::loop() {
	_pagerRx.pocsagWorker();
}