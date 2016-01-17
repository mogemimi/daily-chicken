// Copyright (c) 2016 mogemimi. Distributed under the MIT license.

#pragma once

#include "../daily/Optional.h"
#include "EndPoint.h"
#include <cstdint>
#include <string>
#include <tuple>

namespace somera {

enum class ProtocolType {
    Tcp,
    //Udp,
};

namespace detail {

class DescriptorPOSIX final {
public:
    DescriptorPOSIX() = default;

    DescriptorPOSIX(const DescriptorPOSIX&) = delete;

    DescriptorPOSIX(DescriptorPOSIX && other);

    DescriptorPOSIX & operator=(const DescriptorPOSIX&) = delete;

    DescriptorPOSIX & operator=(DescriptorPOSIX && other);

    void Bind(const EndPoint& endPoint);

    std::tuple<bool, errno_t> Connect(const EndPoint& endPoint);

    std::tuple<DescriptorPOSIX, EndPoint> Accept(ProtocolType protocolType);

    void Close();

    int GetHandle() const;

private:
    ///@brief The file descriptor for socket
    Optional<int> descriptor_;
};

} // namespace detail

enum class SocketError {
    TimedOut,
    Shutdown,
    NotConnected,
};

class Socket final {
public:
    Socket();

    explicit Socket(ProtocolType protocolType);

    Socket(const Socket&) = delete;
    Socket & operator=(const Socket&) = delete;

    Socket(Socket && other);

    Socket & operator=(Socket && other);

    ~Socket();

    void Bind(const EndPoint& endPoint);

    Socket Accept();

    void Close();

    std::tuple<size_t, Optional<SocketError>>
    Receive(void* buffer, size_t size);

    void Send(const void* buffer, size_t size);

    std::tuple<bool, errno_t> Connect(const EndPoint& endPoint);

    bool IsConnected() const;

    int GetHandle() const;

    EndPoint GetEndPoint() const;

private:
    using NativeDescriptorType = detail::DescriptorPOSIX;
    NativeDescriptorType descriptor_;
    EndPoint endPoint_;
    ProtocolType protocolType_;
    bool isConnected_;
};

} // namespace somera
