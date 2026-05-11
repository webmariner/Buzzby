#ifndef PAGERQUEUE_H
#define PAGERQUEUE_H

#include <vector>
#include <optional>
#include <Arduino.h>

struct Message {
    String text;
    uint32_t ric;
};

class PagerQueue {
public:
    void push(const Message& message);
    std::optional<Message> front() const;
    void markAsRead();
    bool messagesWaiting();
    const Message& currentMessage() const;
private:
    std::vector<Message> messages_;
};

#endif // PAGERQUEUE_H
