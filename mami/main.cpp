// Copyright (c) 2016 mogemimi. Distributed under the MIT license.

#include "../daily/Optional.h"
#include "../nazuna/Defer.h"
#include "../daily/StringHelper.h"
#include "Pipeline.h"
#include "ContainerAlgorithm.h"
#include "IPEndPoint.h"
#include "Socket.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>

#include <string>
#include <vector>
#include <cassert>
#include <thread>
#include <array>
#include <cstdint>
#include <iostream>
#include <tuple>

using namespace somera;

namespace {

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

    operator bool() const noexcept
    {
        return description.operator bool();
    }

    static const Error NoError;
};

const Error Error::NoError {};

Error MakeError(const std::string& description)
{
    Error error;
    error.description = description;
    return std::move(error);
}

class Session {
public:
    Session(Socket && socket)
        : socket_(std::move(socket))
    {
    }

    const Socket & GetSocket() const
    {
        return socket_;
    }

    Socket & GetSocket()
    {
        return socket_;
    }

    void Read()
    {
        if (!socket_.IsConnected()) {
            return;
        }

        std::vector<uint8_t> buffer(1024, 0);
        size_t readSize;
        Optional<SocketError> errorCode;
        std::tie(readSize, errorCode) = socket_.Receive(buffer.data(), buffer.size() - 1);

        if (errorCode) {
            if (*errorCode == SocketError::NotConnected) {
                printf("%s\n", "=> graceful close socket");
                socket_.Close();
                return;
            }
            if (*errorCode == SocketError::Shutdown) {
                printf("%s\n", "=> close socket");
                socket_.Close();
                return;
            }
        }
        assert(!errorCode || *errorCode == SocketError::TimedOut);
        if (readSize <= 0) {
            return;
        }
        if (OnRead) {
            OnRead(socket_, MakeArrayView<uint8_t>(buffer.data(), readSize));
        }
    }

public:
    std::function<void(Socket & socket, const ArrayView<uint8_t>&)> OnRead;

private:
    Socket socket_;
};

class IOService {
public:
    IOService();

    void Run();

    void SetMaxConnectionCount(int count);

    void SetOnAccept(std::function<void(std::shared_ptr<Session>, const Error&)> onAccept);

    void ExitEventLoop();

private:
    std::function<void(std::shared_ptr<Session>, const Error&)> onAccept_;
    int maxConnectionCount;
    bool exitRequest;
};

IOService::IOService()
    : maxConnectionCount(10)
    , exitRequest(false)
{
}

void IOService::SetOnAccept(std::function<void(std::shared_ptr<Session>, const Error&)> onAccept)
{
    onAccept_ = std::move(onAccept);
}

void IOService::SetMaxConnectionCount(int count)
{
    assert(count > 0);
    maxConnectionCount = count;
}

void IOService::Run()
{
    Socket server(ProtocolType::Tcp);

    printf("%s\n", "=> Bind()");
    const auto address = IPEndPoint::CreateFromV4("localhost", 8000);
    server.Bind(address);
    somera::Defer serverClose([&] {
        server.Close();
    });

    printf("%s\n", "=> ::listen()");
    ::listen(server.GetHandle(), 5);

    std::vector<std::shared_ptr<Session>> sessions;

    while (!exitRequest) {
        if (static_cast<int>(sessions.size()) < maxConnectionCount) {
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(server.GetHandle(), &fds);

            struct timeval tv;
            tv.tv_sec = 0;
            tv.tv_usec = 0;

            int clientCount = ::select(server.GetHandle() + 1, &fds, nullptr, nullptr, &tv);
            if (clientCount == -1) {
                // error
                throw std::runtime_error("Error: Failed to call ::select()");
            }
            assert(clientCount >= 0);
            for (int i = 0; i < clientCount; i++) {
                Socket client = server.Accept();
                auto session = std::make_shared<Session>(std::move(client));
                if (onAccept_) {
                    onAccept_(session, Error::NoError);
                }
                sessions.push_back(std::move(session));
            }
        }

        for (auto & session : sessions) {
            session->Read();
        }
        somera::EraseIf(sessions, [](const std::shared_ptr<Session>& session) -> bool {
            return !session->GetSocket().IsConnected();
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    sessions.clear();
}

void IOService::ExitEventLoop()
{
    exitRequest = true;
}

} // unnamed namespace

int main(int argc, char *argv[])
{
    IOService service;
    service.SetMaxConnectionCount(5);
    service.SetOnAccept([&service](std::shared_ptr<Session> session, const Error& error) {
        if (error) {
            printf("%s\n", "=> Error: in OnAccept");
            return;
        }
        printf("%s\n", "=> OnAccept");
        printf(
            "Address = %s, Port = %d\n",
            session->GetSocket().GetEndPoint().GetAddressNumber().c_str(),
            session->GetSocket().GetEndPoint().GetPort());
        session->OnRead = [&service](Socket & socket, const ArrayView<uint8_t>& buffer) {
            std::string text(reinterpret_cast<const char*>(buffer.data), buffer.size);

            if (somera::StringHelper::startWith(text, "exit")) {
                std::cout << "[client]: [exit]" << std::endl;
                service.ExitEventLoop();
            }
            else {
                std::cout
                    << "[client]: [echo] = '"
                    << StringHelper::trimRight(StringHelper::trimRight(text, '\n'), '\r')
                    << "'" << std::endl;
                socket.Send(text.data(), text.size());
            }
        };
    });

    service.Run();
    printf("%s\n", "=> Done");
    return 0;
}
