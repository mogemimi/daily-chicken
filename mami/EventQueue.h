// Copyright (c) 2016 mogemimi. Distributed under the MIT license.

#pragma once

#include "Any.h"
#include <functional>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>

namespace somera {

class EventQueue final {
public:
    EventQueue() = default;
    EventQueue(EventQueue const&) = delete;
    EventQueue(EventQueue &&) = delete;
    EventQueue & operator=(EventQueue const&) = delete;
    EventQueue & operator=(EventQueue &&) = delete;

    void Connect(const std::function<void(const Any&)>& handler);

    void Enqueue(Any && event);

    template <typename T>
    void Enqueue(T && argument)
    {
        Any event{std::forward<std::remove_reference_t<T>>(argument)};
        Enqueue(std::move(event));
    }

    template <typename T>
    void Enqueue(const T & argument)
    {
        Any event{std::remove_reference_t<T>(argument)};
        Enqueue(std::move(event));
    }

    void Emit();

private:
    std::vector<Any> events;
    std::recursive_mutex notificationMutex;
    std::function<void(const Any&)> handler;
};

} // namespace somera
