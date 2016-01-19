// Copyright (c) 2016 mogemimi. Distributed under the MIT license.

#pragma once

#if 0

class FirstFilterHandler final : public Handler<int, double> {
public:
    double Execute(int n) override
    {
        return static_cast<double>(n) * 0.5;
    }
};

class SecondFilterHandler final : public Handler<double, std::string> {
public:
    std::string Execute(double count) override
    {
        return std::to_string(count);
    }
};

class StdOutHandler final : public InboundHandler<std::string> {
public:
    void Execute(std::string text) override
    {
        std::cout << text << std::endl;
    }
};

void TestCase_Trivials()
{
    auto pipeline = std::make_shared<Pipeline<int, std::string>>();
    pipeline->AddBack(std::make_shared<FirstFilterHandler>());
    pipeline->AddBack(std::make_shared<SecondFilterHandler>());
    pipeline->AddBack(std::make_shared<StdOutHandler>());
    pipeline->Build();

    pipeline->Write(42);
}

class SocketHandler final : public Handler<int, double> {
public:
    double Execute(int n) override
    {
        return static_cast<double>(n) * 0.5;
    }
};





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
    const auto address = EndPoint::CreateFromV4("localhost", 8000);
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

void RunServer()
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
}

int RunClient()
{
    Socket socket(ProtocolType::Tcp);

    auto address = EndPoint::CreateFromV4("localhost", 8000);
    bool success;
    errno_t errorCode;
    std::tie(success, errorCode) = socket.Connect(address);

    if (!success) {
        if (errorCode == EINPROGRESS) {
            do {
                fd_set fds;
                FD_ZERO(&fds);
                FD_SET(socket.GetHandle(), &fds);

                struct timeval timeout;
                timeout.tv_sec = 5;
                timeout.tv_usec = 0;
                auto result = ::select(socket.GetHandle() + 1, nullptr, &fds, nullptr, &timeout);
                auto code = errno;
                if (result < 0 && code != EINTR) {
                    assert(result == -1);
                    printf("Error: Failed to call select() : %d - %s", code, strerror(code));
                    return 1;
                }
                else if (result > 0) {
                    int valopt;
                    socklen_t lon = sizeof(valopt);

                    if (::getsockopt(socket.GetHandle(), SOL_SOCKET, SO_ERROR,
                        reinterpret_cast<void*>(&valopt), &lon) < 0) {
                        code = errno;
                        printf("Error: Failed to call getsockopt() : %d - %s", code, strerror(code));
                        return 1;
                    }
                    if (valopt != 0) {
                        code = errno;
                        printf("Error: valopt : %d - %s", code, strerror(code));
                        return 1;
                    }
                    break;
                }
                else {
                    printf("Error: Failed to call select(), cancelling, code=%d", code);
                    return 1;
                }
            } while (0);
        }
        else {
            printf("Error: Failed to call connect(), code=%d", errorCode);
            return 1;
        }
    }

    printf("connected.\n");

    bool exitRequest = false;
    std::function<void(Socket & socket, const ArrayView<uint8_t>&)> onRead =
    [&exitRequest](Socket & socket, const ArrayView<uint8_t>& buffer) {
        std::string text(reinterpret_cast<const char*>(buffer.data), buffer.size);

        if (somera::StringHelper::startWith(text, "exit")) {
            std::cout << "[client]: [exit]" << std::endl;
            exitRequest = true;
        }
        else {
            std::cout
                << "[server]: [out] = '"
                << StringHelper::trimRight(StringHelper::trimRight(text, '\n'), '\r')
                << "'" << std::endl;
            text = "exit";
            socket.Send(text.data(), text.size());
        }
    };

    std::string text = "Hello, socket!";
    socket.Send(text.data(), text.size());

    while (!exitRequest) {
        if (!socket.IsConnected()) {
            break;
        }

        std::vector<uint8_t> buffer(1024, 0);
        size_t readSize;
        Optional<SocketError> errorCode;
        std::tie(readSize, errorCode) = socket.Receive(buffer.data(), buffer.size() - 1);

        if (errorCode) {
            if (*errorCode == SocketError::NotConnected) {
                printf("%s\n", "=> graceful close socket");
                socket.Close();
                break;
            }
            if (*errorCode == SocketError::Shutdown) {
                printf("%s\n", "=> close socket");
                socket.Close();
                break;
            }
        }
        assert(!errorCode || *errorCode == SocketError::TimedOut);
        if (readSize <= 0) {
            continue;
        }
        if (onRead) {
            onRead(socket, MakeArrayView<uint8_t>(buffer.data(), readSize));
        }
    }

    return 0;
}

#endif
