// Copyright (c) 2016 mogemimi. Distributed under the MIT license.

#include "Socket.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <cassert>

namespace somera {
namespace {

void SetNonBlockingPOSIX(int descriptor, bool nonBlocking)
{
    int flags = ::fcntl(descriptor, F_GETFL, 0);
    if (nonBlocking) {
        flags = flags | O_NONBLOCK;
    } else {
        flags = flags ^ O_NONBLOCK;
    }
    ::fcntl(descriptor, F_SETFL, flags);
}

void SetTimeoutPOSIX(int descriptor, time_t seconds)
{
    struct timeval timeout;
    timeout.tv_sec = seconds;
    timeout.tv_usec = 0;

    const auto data = reinterpret_cast<const char*>(&timeout);
    const auto size = sizeof(timeout);

    if (::setsockopt(descriptor, SOL_SOCKET, SO_RCVTIMEO, data, size) < 0) {
        const auto errorCode = errno;
        assert(errorCode != EBADF && "Invalid file descriptor");
        assert(errorCode != EFAULT);
        assert(errorCode != EINVAL);
        assert(errorCode != ENOPROTOOPT);
        assert(errorCode != ENOTSOCK);
        printf("Failed to set timeout for socket receive, %d", errorCode);
        return;
    }
    if (::setsockopt(descriptor, SOL_SOCKET, SO_SNDTIMEO, data, size) < 0) {
        const auto errorCode = errno;
        assert(errorCode != EBADF && "Invalid file descriptor");
        assert(errorCode != EFAULT);
        assert(errorCode != EINVAL);
        assert(errorCode != ENOPROTOOPT);
        assert(errorCode != ENOTSOCK);
        printf("Failed to set timeout for socket send, %d", errorCode);
        return;
    }
}

} // unnamed namespace

DescriptorPOSIX::DescriptorPOSIX(DescriptorPOSIX && other)
{
    descriptor_ = std::move(other.descriptor_);
    other.descriptor_ = somera::NullOpt;
}

DescriptorPOSIX & DescriptorPOSIX::operator=(DescriptorPOSIX && other)
{
    descriptor_ = std::move(other.descriptor_);
    other.descriptor_ = somera::NullOpt;
    return *this;
}

void DescriptorPOSIX::Bind(const IPEndPoint& endPoint)
{
    assert(!descriptor_);
    descriptor_ = ::socket(PF_INET, SOCK_STREAM, 0);

    assert(descriptor_);

    // Set the socket to nonblocking mode:
    SetNonBlockingPOSIX(*descriptor_, true);
    SetTimeoutPOSIX(*descriptor_, 5);

    const auto address = endPoint.GetAddressViewPOSIX();
    ::bind(*descriptor_, address.data, address.size);
}

int DescriptorPOSIX::GetHandle() const
{
    assert(descriptor_);
    return *descriptor_;
}

std::tuple<DescriptorPOSIX, IPEndPoint> DescriptorPOSIX::Accept(ProtocolType protocolType)
{
    assert(descriptor_);
    DescriptorPOSIX client;

    sockaddr_storage address;
    socklen_t length = sizeof(address);

    client.descriptor_ = ::accept(
        *descriptor_,
        reinterpret_cast<struct sockaddr*>(&address),
        &length);

    auto endPoint = IPEndPoint::CreateFromAddressStorage(address);
    return std::make_tuple(std::move(client), std::move(endPoint));
}

void DescriptorPOSIX::Close()
{
    if (descriptor_) {
        ::close(*descriptor_);
        descriptor_ = somera::NullOpt;
    }
}

Socket::Socket(ProtocolType protocolType)
    : protocolType_(protocolType)
    , isConnected_(false)
{
}

Socket::Socket(Socket && other)
{
    this->Close();
    descriptor_ = std::move(other.descriptor_);
    endPoint_ = other.endPoint_;
    protocolType_ = other.protocolType_;
    isConnected_ = other.isConnected_;
}

Socket & Socket::operator=(Socket && other)
{
    this->Close();
    descriptor_ = std::move(other.descriptor_);
    endPoint_ = other.endPoint_;
    protocolType_ = other.protocolType_;
    isConnected_ = other.isConnected_;
    return *this;
}

Socket::~Socket()
{
    this->Close();
}

void Socket::Bind(const IPEndPoint& endPoint)
{
    endPoint_ = endPoint;
    descriptor_.Bind(endPoint_);
    isConnected_ = true;
}

Socket Socket::Accept()
{
    NativeDescriptorType descriptor;
    IPEndPoint endPoint;
    std::tie(descriptor, endPoint) = descriptor_.Accept(protocolType_);

    Socket socket(protocolType_);
    socket.descriptor_ = std::move(descriptor);
    socket.endPoint_ = std::move(endPoint);
    socket.isConnected_ = true;
    return std::move(socket);
}

void Socket::Close()
{
    if (!isConnected_) {
        return;
    }
    descriptor_.Close();
    isConnected_ = false;
}

std::tuple<size_t, Optional<SocketError>>
Socket::Receive(void* buffer, size_t size)
{
    assert(buffer != nullptr);
    assert(size > 0);
    assert(isConnected_);
    if (!isConnected_) {
        return std::make_tuple<size_t, Optional<SocketError>>(
            0, SocketError::NotConnected);
    }

    constexpr int flags = 0;
    ssize_t readSize = ::recv(descriptor_.GetHandle(), buffer, size, flags);
    const auto errorCode = errno;

    if (readSize == 0) {
        // NOTE: graceful close socket
        this->Close();
        return std::make_tuple<size_t, Optional<SocketError>>(
            0, SocketError::Shutdown);
    }
    if (readSize == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // NOTE: time out
            return std::make_tuple<size_t, Optional<SocketError>>(
                0, SocketError::TimedOut);
        }
        // NOTE: socket closed
        assert(errorCode != EBADF && "Invalid socket file descriptor");
        assert(errorCode != EINVAL && "Invalid argument");
        assert(errorCode != EFAULT);
        assert(errorCode != ENOTSOCK);
        this->Close();
        return std::make_tuple<size_t, Optional<SocketError>>(
            0, SocketError::NotConnected);
    }
    assert(readSize > 0);
    return std::make_tuple<size_t, Optional<SocketError>>(readSize, NullOpt);
}

void Socket::Send(const void* buffer, size_t size)
{
    assert(buffer != nullptr);
    assert(size > 0);
    assert(isConnected_);

    if (!isConnected_) {
        return;
    }
    ::write(descriptor_.GetHandle(), buffer, size);
}

int Socket::GetHandle() const
{
    return descriptor_.GetHandle();
}

IPEndPoint Socket::GetEndPoint() const
{
    return endPoint_;
}

bool Socket::IsConnected() const
{
    return isConnected_;
}

} // namespace somera
