// Copyright (c) 2016 mogemimi. Distributed under the MIT license.

#pragma once

#include "../daily/Optional.h"
#include "IPEndPoint.h"
#include <cstdint>
#include <string>
#include <tuple>

namespace somera {

enum class ProtocolType {
    //IPv4,
    //IPv6,
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

    void Bind(const IPEndPoint& endPoint);

    int GetHandle() const;

    std::tuple<DescriptorPOSIX, IPEndPoint> Accept(ProtocolType protocolType);

    void Close();

private:
    ///@brief The file descriptor for socket
    somera::Optional<int> descriptor_;
};

} // namespace detail

enum class SocketError {
    TimedOut,
    Shutdown,
    NotConnected,
};

class Socket final {
public:
    explicit Socket(ProtocolType protocolType);

    Socket(const Socket&) = delete;
    Socket & operator=(const Socket&) = delete;

    Socket(Socket && other);

    Socket & operator=(Socket && other);

    ~Socket();

    void Bind(const IPEndPoint& endPoint);

    Socket Accept();

    void Close();

    std::tuple<size_t, Optional<SocketError>>
    Receive(void* buffer, size_t size);

    void Send(const void* buffer, size_t size);

    bool IsConnected() const;

    int GetHandle() const;

    IPEndPoint GetEndPoint() const;

private:
    using NativeDescriptorType = detail::DescriptorPOSIX;
    NativeDescriptorType descriptor_;
    IPEndPoint endPoint_;
    ProtocolType protocolType_;
    bool isConnected_;
};

} // namespace somera
