#include <optional>
#include <stdexcept>
#include "PagerQueue.h"
#include "src/ArduinoLog.h"

void PagerQueue::push(const PagerMessage& message) {
    messages_.push_back(message);
    if (messages_.size() > 10) {
        messages_.erase(messages_.begin()+1);
    }
    Log.verboseln("Message pushed into queue, queue now holding %d messages", messages_.size());
}

std::optional<PagerMessage> PagerQueue::front() const {
    if (!messages_.empty()) {
        return messages_.front();
    }
    return std::nullopt;
}

void PagerQueue::markAsRead() {
    if (!messages_.empty()) {
        auto it = messages_.begin();
        messages_.erase(it);
    }
}

bool PagerQueue::messagesWaiting() {
    return !messages_.empty();
}

const PagerMessage& PagerQueue::currentMessage() const {
    Log.verboseln("Checking for current message - queue currently holds %d messages", messages_.size());
    if (messages_.empty()) {
        throw std::runtime_error("No unread messages in the queue");
    }
    return messages_.front();
}
