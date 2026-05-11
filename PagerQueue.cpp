#include <optional>
#include <stdexcept>
#include "PagerQueue.h"

void PagerQueue::push(const Message& message) {
    messages_.push_back(message);
    if (messages_.size() > 10) {
        messages_.erase(messages_.begin()+1);
    }
}

std::optional<Message> PagerQueue::front() const {
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

const Message& PagerQueue::currentMessage() const {
    if (messages_.empty()) {
        throw std::runtime_error("No unread messages in the queue");
    }
    return messages_.front();
}
