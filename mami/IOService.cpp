// Copyright (c) 2016 mogemimi. Distributed under the MIT license.

#include "IOService.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>

namespace somera {
namespace {

void ReadSocket(Socket & socket, std::function<void(Socket & socket, const ArrayView<uint8_t>&)> onRead)
{
    if (!socket.IsConnected()) {
        return;
    }

    std::vector<uint8_t> buffer(1024, 0);
    size_t readSize;
    Optional<SocketError> errorCode;
    std::tie(readSize, errorCode) = socket.Receive(buffer.data(), buffer.size() - 1);

    if (errorCode) {
        if (*errorCode == SocketError::NotConnected) {
            printf("%s\n", "=> graceful close socket");
            socket.Close();
            return;
        }
        if (*errorCode == SocketError::Shutdown) {
            printf("%s\n", "=> close socket");
            socket.Close();
            return;
        }
    }
    assert(!errorCode || *errorCode == SocketError::TimedOut);
    if (readSize <= 0) {
        return;
    }
    if (onRead) {
        onRead(socket, MakeArrayView<uint8_t>(buffer.data(), readSize));
    }
}

class ConnectionEventLoop final : public detail::ConnectionBody {
public:
    ConnectionEventLoop(IOService* serviceIn, int connectionIdIn)
        : service(serviceIn)
        , connectionId(connectionIdIn)
    {
    }

    void Disconnect() override
    {
        if (service == nullptr) {
            return;
        }
        service->Disconnect(connectionId);
        service = nullptr;
        connectionId = 0;
    }

    bool Valid() const noexcept override
    {
        return (service != nullptr) && service->IsConnected(connectionId);
    }

    std::unique_ptr<ConnectionBody> DeepCopy() const override
    {
        auto copy = std::make_unique<ConnectionEventLoop>(service, connectionId);
        return std::move(copy);
    }

private:
    IOService* service = nullptr;
    int connectionId = 0;
};

} // unnamed namespace

// MARK: Error

Error::operator bool() const noexcept
{
    return description.operator bool();
}

std::string Error::What() const noexcept
{
    std::string what;
    bool needToSeparate = false;
    if (description) {
        what += *description;
        needToSeparate = true;
    }
    if (file) {
        if (needToSeparate) {
            what += ", ";
        }
        what += *file;
        needToSeparate = true;
    }
    if (line) {
        if (needToSeparate) {
            what += ", Line=";
        }
        what += *line;
    }
    return std::move(what);
}

Error MakeError(const std::string& description)
{
    Error error;
    error.description = description;
    return std::move(error);
}

// MARK: IOService

void IOService::Run()
{
    bool exitRequest = false;
    while (!exitRequest) {
        for (auto & listener : listeners) {
            // NOTE: Execute listener's callback
            if (listener.callback) {
                listener.callback();
            }
        }
        if (!addedListeners.empty()) {
            // NOTE: Add listener to listeners container
            std::vector<Listener> temp;
            std::swap(temp, addedListeners);
            for (auto & listener : temp) {
                listeners.push_back(std::move(listener));
            }
        }
        if (!removedListeners.empty()) {
            // NOTE: Remove listener from listeners container
            std::vector<int> temp;
            std::swap(temp, removedListeners);
            EraseIf(listeners, [&temp](const Listener& listener) {
                return Find(temp, listener.id) != std::end(temp);
            });
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

Connection IOService::ScheduleTask(std::function<void()> func)
{
    if (!func) {
        return {};
    }
    Listener listener;
    listener.callback = std::move(func);
    listener.id = this->nextListenersId;
    nextListenersId++;

    addedListeners.push_back(std::move(listener));

    auto body = std::make_unique<ConnectionEventLoop>(this, listener.id);
    Connection connection(std::move(body));
    return std::move(connection);
}

bool IOService::IsConnected(int connectionId) const
{
    if (Find(removedListeners, connectionId) != std::end(removedListeners)) {
        return false;
    }
    const auto equal = [&connectionId](const Listener& listener)-> bool {
        return listener.id == connectionId;
    };
    if (FindIf(listeners, equal) != std::end(listeners)) {
        return true;
    }
    if (FindIf(addedListeners, equal) != std::end(addedListeners)) {
        return true;
    }
    return false;
}

void IOService::Disconnect(int connectionId)
{
    removedListeners.push_back(connectionId);
}

// MARK: TcpServerSocket

void TcpServerSocket::ListenEventLoop()
{
    if (static_cast<int>(sessions_.size()) >= maxSessionCount_) {
        return;
    }

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(socket_.GetHandle(), &fds);

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    int clientCount = ::select(socket_.GetHandle() + 1, &fds, nullptr, nullptr, &tv);
    if (clientCount == -1) {
        // error
        throw std::runtime_error("Error: Failed to call ::select()");
    }
    assert(clientCount >= 0);
    if (clientCount == 0) {
        return;
    }

    for (int i = 0; i < clientCount; i++) {
        Socket client = socket_.Accept();
        auto session = std::make_shared<TcpSession>();
        session->socket = std::move(client);
        if (onAccept_) {
            onAccept_(session->socket, {});
        }
        sessions_.push_back(std::move(session));
    }

    if (!connectionRead_) {
        connectionRead_ = service_->ScheduleTask([this] { this->ReadEventLoop(); });
    }
}

void TcpServerSocket::ReadEventLoop()
{
    if (sessions_.empty()) {
        connectionRead_.Disconnect();
        return;
    }

    for (auto & session : sessions_) {
        ReadSocket(session->socket, onRead_);
    }

    EraseIf(sessions_, [](const std::shared_ptr<TcpSession>& s) {
        return !s->socket.IsConnected();
    });
}

TcpServerSocket::TcpServerSocket(IOService & service)
    : service_(&service)
    , socket_(ProtocolType::Tcp)
{}

TcpServerSocket::~TcpServerSocket()
{
    socket_.Close();
}

void TcpServerSocket::Bind(const EndPoint& endPoint)
{
    socket_.Bind(endPoint);
}

void TcpServerSocket::Listen(int backlog, const std::function<void(Socket&, const Error&)>& onAccept)
{
    assert(backlog > 0);
    ::listen(socket_.GetHandle(), backlog);

    assert(service_ != nullptr);
    connectionListen_ = service_->ScheduleTask([this] { this->ListenEventLoop(); });
    onAccept_ = onAccept;
}

void TcpServerSocket::Read(const std::function<void(Socket&, const ArrayView<uint8_t>&)>& onRead)
{
    onRead_ = onRead;
}

// MARK: TcpSocket

void TcpSocket::ConnectEventLoop(const std::chrono::system_clock::time_point& startTime)
{
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(socket_.GetHandle(), &fds);

    auto result = ::select(socket_.GetHandle() + 1, nullptr, &fds, nullptr, nullptr);
    auto code = errno;

    if (result < 0 && code != EINTR) {
        assert(result == -1);
        onConnected_(socket_, MakeError(StringHelper::format(
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
        onConnected_(socket_, MakeError(StringHelper::format(
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
            onConnected_(socket_, MakeError(StringHelper::format(
                "Error: Timeout. socket error : %d - %s",
                code,
                strerror(code))));
            connectionConnect_.Disconnect();
        }
        return;
    }

    int socketError;
    socklen_t socketErrorSize = sizeof(socketError);

    if (::getsockopt(socket_.GetHandle(), SOL_SOCKET, SO_ERROR,
        static_cast<void*>(&socketError), &socketErrorSize) < 0) {
        code = errno;
        onConnected_(socket_, MakeError(StringHelper::format(
            "Error: Failed to call getsockopt() : %d - %s",
            code,
            strerror(code))));
        connectionConnect_.Disconnect();
        return;
    }
    if (socketError != 0) {
        onConnected_(socket_, MakeError(StringHelper::format(
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
    onConnected_(socket_, {});
}

void TcpSocket::ReadEventLoop()
{
    std::vector<uint8_t> buffer(1024, 0);
    size_t readSize;
    Optional<SocketError> errorCode;
    std::tie(readSize, errorCode) = socket_.Receive(buffer.data(), buffer.size() - 1);

    if (errorCode) {
        if (*errorCode == SocketError::NotConnected) {
            printf("%s\n", "=> graceful close socket");
            socket_.Close();
            connectionRead_.Disconnect();
            return;
        }
        if (*errorCode == SocketError::Shutdown) {
            printf("%s\n", "=> close socket");
            socket_.Close();
            connectionRead_.Disconnect();
            return;
        }
    }
    assert(!errorCode || *errorCode == SocketError::TimedOut);
    if (readSize <= 0) {
        return;
    }
    if (onRead_) {
        onRead_(socket_, MakeArrayView<uint8_t>(buffer.data(), readSize));
    }
}

TcpSocket::TcpSocket(IOService & service)
    : service_(&service)
    , socket_(ProtocolType::Tcp)
{}

TcpSocket::~TcpSocket()
{
    socket_.Close();
}

void TcpSocket::Connect(const EndPoint& endPoint, std::function<void(Socket & socket, const Error&)> onConnected)
{
    assert(onConnected);
    onConnected_ = onConnected;

    bool success;
    errno_t errorCode;
    std::tie(success, errorCode) = socket_.Connect(endPoint);
    if (success) {
        assert(service_ != nullptr);
        connectionRead_ = service_->ScheduleTask([this]{ this->ReadEventLoop(); });
        onConnected_(socket_, {});
        return;
    }

    if (errorCode != EINPROGRESS) {
        onConnected_(socket_, MakeError(StringHelper::format(
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

void TcpSocket::Read(const std::function<void(Socket&, const ArrayView<uint8_t>&)>& onRead)
{
    onRead_ = onRead;
}

} // namespace somera
