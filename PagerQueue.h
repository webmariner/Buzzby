#ifndef PAGERQUEUE_H
#define PAGERQUEUE_H

#include <vector>
#include <optional>
#include <Arduino.h>

#include "models.h"

class PagerQueue {
public:
    void push(const PagerMessage& message);
    std::optional<PagerMessage> front() const;
    void markAsRead();
    bool messagesWaiting();
    const PagerMessage& currentMessage() const;
private:
    std::vector<PagerMessage> messages_;
};

#endif // PAGERQUEUE_H
