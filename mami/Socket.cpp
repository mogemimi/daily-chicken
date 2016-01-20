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

#include "../daily/StringHelper.h"
#include "ContainerAlgorithm.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>
#include <cassert>
#include <thread>

namespace somera {
namespace detail {
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

// MARK: DescriptorPOSIX

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

void DescriptorPOSIX::Bind(const EndPoint& endPoint)
{
    assert(!descriptor_);
    descriptor_ = ::socket(PF_INET, SOCK_STREAM, 0);

    // Set the socket to nonblocking mode:
    SetNonBlockingPOSIX(*descriptor_, true);
    SetTimeoutPOSIX(*descriptor_, 5);

    const auto address = endPoint.GetAddressViewPOSIX();
    ::bind(*descriptor_, address.data, address.size);
}

std::tuple<bool, errno_t> DescriptorPOSIX::Connect(const EndPoint& endPoint)
{
    assert(!descriptor_);
    descriptor_ = ::socket(PF_INET, SOCK_STREAM, 0);

    // Set the socket to nonblocking mode:
    SetNonBlockingPOSIX(*descriptor_, true);
    SetTimeoutPOSIX(*descriptor_, 5);

    auto address = endPoint.GetAddressViewPOSIX();
    auto result = ::connect(*descriptor_, address.data, address.size);
    if (result < 0) {
        const errno_t errorCode = errno;
        assert(result == -1);
        assert(errorCode != EBADF);
        assert(errorCode != EFAULT);
        assert(errorCode != ENOTSOCK);
        return std::make_tuple(false, errorCode);
    }
    return std::make_tuple<bool, errno_t>(true, 0);
}

std::tuple<DescriptorPOSIX, EndPoint> DescriptorPOSIX::Accept(ProtocolType protocolType)
{
    assert(descriptor_);
    DescriptorPOSIX client;

    sockaddr_storage address;
    socklen_t length = sizeof(address);

    client.descriptor_ = ::accept(
        *descriptor_,
        reinterpret_cast<struct sockaddr*>(&address),
        &length);

    auto endPoint = EndPoint::CreateFromAddressStorage(address);
    return std::make_tuple(std::move(client), std::move(endPoint));
}

void DescriptorPOSIX::Close()
{
    if (descriptor_) {
        ::close(*descriptor_);
        descriptor_ = somera::NullOpt;
    }
}

int DescriptorPOSIX::GetHandle() const
{
    assert(descriptor_);
    return *descriptor_;
}

} // namespace detail

// MARK: unnamed namespace
namespace {

std::tuple<size_t, Optional<SocketError>>
ReadSocket(int descriptor, void* buffer, size_t size)
{
    assert(buffer != nullptr);
    assert(size > 0);

    constexpr int flags = 0;
    ssize_t readSize = ::recv(descriptor, buffer, size, flags);
    const auto errorCode = errno;

    if (readSize == 0) {
        // NOTE: graceful close socket
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
        return std::make_tuple<size_t, Optional<SocketError>>(
            0, SocketError::NotConnected);
    }
    assert(readSize > 0);
    return std::make_tuple<size_t, Optional<SocketError>>(readSize, NullOpt);
}

} // unnamed namespace

// MARK: Socket

Socket::Socket(IOService & service)
    : service_(&service)
{
}

Socket::Socket(
    IOService & service,
    detail::DescriptorPOSIX && descriptor,
    EndPoint && endPoint)
    : service_(&service)
    , descriptor_(std::move(descriptor))
    , endPoint_(std::move(endPoint))
{
}

Socket::~Socket()
{
    descriptor_.Close();
}

Socket::Socket(Socket && other)
{
    this->Close();

    service_ = std::move(other.service_);
    descriptor_ = std::move(other.descriptor_);
    endPoint_ = std::move(other.endPoint_);
    connectionConnect_ = std::move(other.connectionConnect_);
    connectionRead_ = std::move(other.connectionRead_);
    onConnected_ = std::move(other.onConnected_);
    onRead_ = std::move(other.onRead_);
}

Socket & Socket::operator=(Socket && other)
{
    this->Close();

    service_ = std::move(other.service_);
    descriptor_ = std::move(other.descriptor_);
    endPoint_ = std::move(other.endPoint_);
    connectionConnect_ = std::move(other.connectionConnect_);
    connectionRead_ = std::move(other.connectionRead_);
    onConnected_ = std::move(other.onConnected_);
    onRead_ = std::move(other.onRead_);

    return *this;
}

void Socket::Close()
{
    connectionConnect_.Disconnect();
    connectionRead_.Disconnect();
    descriptor_.Close();
}

void Socket::Connect(const EndPoint& endPoint, std::function<void(Socket & socket, const Error&)> onConnected)
{
    assert(onConnected);
    onConnected_ = onConnected;

    bool success;
    errno_t errorCode;
    std::tie(success, errorCode) = descriptor_.Connect(endPoint);
    if (success) {
        assert(service_ != nullptr);
        connectionRead_ = service_->ScheduleTask([this]{ this->ReadEventLoop(); });
        onConnected_(*this, {});
        return;
    }

    if (errorCode != EINPROGRESS) {
        onConnected_(*this, MakeError(StringHelper::format(
            "Error: Failed to call connect(). code=%d, %s",
            errorCode,
            strerror(errorCode))));
        return;
    }

    // NOTE: The non-blocking socket always returns 'EINPROGRESS' error.
    assert(!success);
    assert(errorCode == EINPROGRESS);

    assert(service_ != nullptr);
    auto now = std::chrono::system_clock::now();
    connectionConnect_ = service_->ScheduleTask([this, now = now] {
        this->ConnectEventLoop(now);
    });
}

void Socket::Read(const std::function<void(Socket&, const ArrayView<uint8_t>&)>& onRead)
{
    onRead_ = onRead;
}

void Socket::Write(const ArrayView<uint8_t const>& data)
{
    assert(data.data != nullptr);
    assert(data.size > 0);

    ::write(descriptor_.GetHandle(), data.data, data.size);
}

EndPoint Socket::GetEndPoint() const
{
    return endPoint_;
}

int Socket::GetHandle() const
{
    return descriptor_.GetHandle();
}

void Socket::ConnectEventLoop(const std::chrono::system_clock::time_point& startTime)
{
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(descriptor_.GetHandle(), &fds);

    auto result = ::select(descriptor_.GetHandle() + 1, nullptr, &fds, nullptr, nullptr);
    auto code = errno;

    if (result < 0 && code != EINTR) {
        assert(result == -1);
        onConnected_(*this, MakeError(StringHelper::format(
            "Error: Failed to call select() : %d - %s",
            code,
            strerror(code))));
        connectionConnect_.Disconnect();
        printf("Line=%d\n", __LINE__);
        return;
    }

    assert(result >= 0 || code == EINTR);
    if (result <= 0) {
        assert(result == 0);
        onConnected_(*this, MakeError(StringHelper::format(
            "Error: Failed to call select(), cancelling, code=%d, %s",
            code,
            strerror(code))));
        connectionConnect_.Disconnect();
        return;
    }

    if (code == ETIMEDOUT) {
        const auto now = std::chrono::system_clock::now();
        const auto timeout = std::chrono::seconds(10);
        if (now - startTime >= timeout) {
            onConnected_(*this, MakeError(StringHelper::format(
                "Error: Timeout. socket error : %d - %s",
                code,
                strerror(code))));
            connectionConnect_.Disconnect();
        }
        return;
    }

    int socketError;
    socklen_t socketErrorSize = sizeof(socketError);

    if (::getsockopt(descriptor_.GetHandle(), SOL_SOCKET, SO_ERROR,
        static_cast<void*>(&socketError), &socketErrorSize) < 0) {
        code = errno;
        onConnected_(*this, MakeError(StringHelper::format(
            "Error: Failed to call getsockopt() : %d - %s",
            code,
            strerror(code))));
        connectionConnect_.Disconnect();
        return;
    }
    if (socketError != 0) {
        onConnected_(*this, MakeError(StringHelper::format(
            "Error: socketError : %d - %s",
            socketError,
            strerror(socketError))));
        connectionConnect_.Disconnect();
        return;
    }

    // NOTE: Connected.
    assert(service_ != nullptr);
    connectionRead_ = service_->ScheduleTask([this]{ this->ReadEventLoop(); });
    connectionConnect_.Disconnect();
    onConnected_(*this, {});
}

void Socket::ReadEventLoop()
{
    std::vector<uint8_t> buffer(1024, 0);
    size_t readSize;
    Optional<SocketError> errorCode;
    std::tie(readSize, errorCode) = ReadSocket(
        descriptor_.GetHandle(), buffer.data(), buffer.size() - 1);

    if (errorCode) {
        switch (*errorCode) {
        case SocketError::NotConnected: {
            printf("%s\n", "=> graceful close socket");
            this->Close();
            return;
        }
        case SocketError::Shutdown: {
            printf("%s\n", "=> close socket");
            this->Close();
            return;
        }
        case SocketError::TimedOut:
            // NOTE: There is no data to be read yet
            return;
        }
    }
    assert(!errorCode || *errorCode == SocketError::TimedOut);
    if (readSize <= 0) {
        return;
    }
    if (onRead_) {
        onRead_(*this, MakeArrayView<uint8_t>(buffer.data(), readSize));
    }
}

// MARK: ServerSocket

ServerSocket::ServerSocket(IOService & service)
    : service_(&service)
{}

ServerSocket::~ServerSocket()
{
    this->Close();
}

void ServerSocket::Bind(const EndPoint& endPoint)
{
    descriptor_.Bind(endPoint);
}

void ServerSocket::Listen(int backlog, const std::function<void(Socket&, const Error&)>& onAccept)
{
    assert(backlog > 0);
    ::listen(descriptor_.GetHandle(), backlog);

    // TODO: Add error handling code here for listen()

    assert(service_ != nullptr);
    connectionListen_ = service_->ScheduleTask([this] { this->ListenEventLoop(); });
    onAccept_ = onAccept;
}

void ServerSocket::Close()
{
    sessions_.clear();
    connectionListen_.Disconnect();
    connectionRead_.Disconnect();
    descriptor_.Close();
}

void ServerSocket::Read(const std::function<void(Socket&, const ArrayView<uint8_t>&)>& onRead)
{
    onRead_ = onRead;
}

void ServerSocket::ListenEventLoop()
{
    if (static_cast<int>(sessions_.size()) >= maxSessionCount_) {
        return;
    }

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(descriptor_.GetHandle(), &fds);

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    int clientCount = ::select(descriptor_.GetHandle() + 1, &fds, nullptr, nullptr, &tv);
    if (clientCount == -1) {
        // error
        throw std::runtime_error("Error: Failed to call ::select()");
    }
    assert(clientCount >= 0);
    if (clientCount == 0) {
        return;
    }

    for (int i = 0; i < clientCount; i++) {
        DescriptorType descriptor;
        EndPoint endPoint;
        std::tie(descriptor, endPoint) = descriptor_.Accept(detail::ProtocolType::Tcp);

        Socket socket(*service_, std::move(descriptor), std::move(endPoint));

        Session session;
        session.socket = std::move(socket);
        session.isClosed = false;
        if (onAccept_) {
            onAccept_(session.socket, {});
        }
        sessions_.push_back(std::move(session));
    }

    if (!connectionRead_) {
        connectionRead_ = service_->ScheduleTask([this] { this->ReadEventLoop(); });
    }
}

void ServerSocket::ReadEventLoop()
{
    if (sessions_.empty()) {
        connectionRead_.Disconnect();
        return;
    }

    for (auto & session : sessions_) {
        assert(!session.isClosed);

        std::vector<uint8_t> buffer(1024, 0);
        size_t readSize;
        Optional<SocketError> errorCode;
        std::tie(readSize, errorCode) = ReadSocket(
            session.socket.GetHandle(), buffer.data(), buffer.size() - 1);

        if (errorCode) {
            switch (*errorCode) {
            case SocketError::NotConnected: {
                printf("%s\n", "=> graceful close socket");
                session.socket.Close();
                session.isClosed = true;
                continue;
            }
            case SocketError::Shutdown: {
                printf("%s\n", "=> close socket");
                session.socket.Close();
                session.isClosed = true;
                continue;
            }
            case SocketError::TimedOut:
                // NOTE: There is no data to be read yet
                continue;
            }
        }

        assert(!errorCode || *errorCode == SocketError::TimedOut);
        if (readSize <= 0) {
            continue;
        }
        if (onRead_) {
            onRead_(session.socket, MakeArrayView<uint8_t>(buffer.data(), readSize));
        }
    }

    EraseIf(sessions_, [](const Session& session) { return session.isClosed; });
}

} // namespace somera
