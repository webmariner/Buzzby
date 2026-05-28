#ifndef PAGERQUEUE_H
#define PAGERQUEUE_H

#include <vector>
#include <optional>

#include "models.h"

class PagerQueue {
public:
    static void push(const PagerMessage& message);
    static void markAsRead();
    static bool messagesWaiting();
    const PagerMessage& currentMessage();
private:
    static std::vector<PagerMessage> messages_;
};

#endif // PAGERQUEUE_H
