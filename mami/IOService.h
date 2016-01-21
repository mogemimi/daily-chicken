// Copyright (c) 2016 mogemimi. Distributed under the MIT license.

#pragma once

#include "Connection.h"
#include <functional>
#include <vector>

namespace somera {

class IOService {
public:
    void Run();

    void ExitLoop();

    Connection ScheduleTask(std::function<void()> func);

    bool IsConnected(int connectionId) const;

    void Disconnect(int connectionId);

private:
    struct Listener {
        std::function<void()> callback;
        int id = 0;
        bool needToRemove = false;
    };

private:
    std::vector<Listener> listeners;
    std::vector<Listener> addedListeners;
    std::vector<int> removedListeners;
    int nextListenersId = 42;
    bool exitRequest = false;
};

} // namespace somera
