#include <optional>
#include <stdexcept>
#include "PagerQueue.h"
#include "src/ArduinoLog.h"

std::vector<PagerMessage> PagerQueue::messages_;

void PagerQueue::push(const PagerMessage& message) {
    messages_.push_back(message);
    if (messages_.size() > 10) {
        messages_.erase(messages_.begin()+1);
    }
    Log.traceln("Message pushed into queue, queue now holding %d messages", messages_.size());
}

void PagerQueue::markAsRead() {
    if (!messages_.empty()) {
        auto it = messages_.begin();
        messages_.erase(it);
    }
    Log.traceln("Mark as read completed - queue holds %d messages", messages_.size());
}

bool PagerQueue::messagesWaiting() {
    return !messages_.empty();
}

const PagerMessage& PagerQueue::currentMessage() {
    Log.traceln("Checking for current message - queue currently holds %d messages", messages_.size());
    if (messages_.empty()) {
        throw std::runtime_error("No unread messages in the queue");
    }
    return messages_.front();
}
