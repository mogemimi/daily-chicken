// Copyright (c) 2016 mogemimi. Distributed under the MIT license.

#include "EventQueue.h"

namespace somera {

void EventQueue::Connect(const std::function<void(const Any&)>& handlerIn)
{
    assert(handlerIn);
    handler = handlerIn;
}

void EventQueue::Enqueue(Any && event)
{
    std::lock_guard<std::recursive_mutex> lock(notificationMutex);
    events.emplace_back(std::move(event));
}

void EventQueue::Emit()
{
    if (!handler) {
        return;
    }
    std::vector<Any> notifications;
    {
        std::lock_guard<std::recursive_mutex> lock(notificationMutex);
        std::swap(notifications, events);
    }
    for (auto & event: notifications) {
        handler(event);
    }
}

} // namespace somera
