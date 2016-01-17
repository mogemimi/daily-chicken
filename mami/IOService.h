// Copyright (c) 2016 mogemimi. Distributed under the MIT license.

#pragma once

#include "../daily/Optional.h"
#include "../nazuna/Defer.h"
#include "../daily/StringHelper.h"
#include "Pipeline.h"
#include "ContainerAlgorithm.h"
#include "EndPoint.h"
#include "Socket.h"
#include "Connection.h"

#include <string>
#include <vector>
#include <cassert>
#include <thread>
#include <array>
#include <cstdint>
#include <tuple>

namespace somera {

template <typename T>
struct ArrayView {
    T* data = nullptr;
    size_t size = 0;
};

template <typename T>
ArrayView<T> MakeArrayView(T* data, size_t size)
{
    ArrayView<T> view;
    view.data = data;
    view.size = size;
    return std::move(view);
}

struct Error {
    somera::Optional<std::string> description;
    somera::Optional<std::string> file;
    somera::Optional<int> line;

    operator bool() const noexcept;

    std::string What() const noexcept;
};

Error MakeError(const std::string& description);

struct Listener {
    std::function<void()> callback;
    int id = 0;
};

class IOService {
public:
    void Run();

    Connection ScheduleTask(std::function<void()> func);

    bool IsConnected(int connectionId) const;

    void Disconnect(int connectionId);

private:
    std::vector<Listener> listeners;
    std::vector<Listener> addedListeners;
    std::vector<int> removedListeners;
    int nextListenersId = 42;
};

struct TcpSession {
    Socket socket;
};

class TcpServerSocket {
public:
    TcpServerSocket(IOService & service);

    ~TcpServerSocket();

    void Bind(const EndPoint& endPoint);

    void Listen(int backlog, const std::function<void(Socket&, const Error&)>& onAccept);

    void Read(const std::function<void(Socket&, const ArrayView<uint8_t>&)>& onRead);

//    void ReadError(const std::function<void(Socket&, const Error&)>& onReadError);
//
//    void Timeout(const std::function<void(Socket&)>& onReadError);
//
//    void Disconnect(const std::function<void(Socket&)>& onReadError);

private:
    void ListenEventLoop();

    void ReadEventLoop();

private:
    IOService* service_ = nullptr;
    Socket socket_;
    std::vector<std::shared_ptr<TcpSession>> sessions_;
    ScopedConnection connectionListen_;
    ScopedConnection connectionRead_;
    int maxSessionCount_ = 5;
    std::function<void(Socket & socket, const Error&)> onAccept_;
    std::function<void(Socket & socket, const ArrayView<uint8_t>& view)> onRead_;
};

class TcpSocket {
public:
    TcpSocket(IOService & service);

    ~TcpSocket();

    void Connect(const EndPoint& endPoint, std::function<void(Socket & socket, const Error&)> onConnected);

    void Read(const std::function<void(Socket&, const ArrayView<uint8_t>&)>& onRead);

//    void ReadError(const std::function<void(Socket&, const Error&)>& onReadError);
//
//    void Timeout(const std::function<void(Socket&)>& onReadError);
//
//    void Disconnect(const std::function<void(Socket&)>& onReadError);

private:
    void ConnectEventLoop(const std::chrono::system_clock::time_point& startTime);

    void ReadEventLoop();

private:
    IOService* service_ = nullptr;
    Socket socket_;
    ScopedConnection connectionConnect_;
    ScopedConnection connectionRead_;
    std::function<void(Socket & socket, const Error&)> onConnected_;
    std::function<void(Socket & socket, const ArrayView<uint8_t>& view)> onRead_;
};

} // namespace somera
