// Copyright (c) 2016 mogemimi. Distributed under the MIT license.

#include "../daily/Optional.h"
#include "../nazuna/Defer.h"
#include "../daily/StringHelper.h"
#include <iostream>
#include <fstream>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <vector>
#include <cassert>
#include <thread>

namespace {

enum class AddressFamily {
    InterNetwork,
    //InterNetworkV6,
};

enum class ProtocolType {
    IPv4,
    //IPv6,
    //Tcp,
    //Udp,
};

sa_family_t ToAddressFamilyPOSIX(AddressFamily family)
{
    switch (family) {
    case AddressFamily::InterNetwork: return PF_INET;
    }
    return PF_INET;
}

class Socket final {
private:
    somera::Optional<int> fileDescriptor_;
    AddressFamily family_;
    ProtocolType protocolType_;

public:
    Socket(AddressFamily family, ProtocolType protocolType)
        : family_(family)
        , protocolType_(protocolType)
    {
    }

    void Bind()
    {
        assert(!fileDescriptor_);
        fileDescriptor_ = ::socket(PF_INET, SOCK_STREAM, 0);

        // Set the socket to nonblocking mode:
        const int flags = ::fcntl(*fileDescriptor_, F_GETFL, 0);
        ::fcntl(*fileDescriptor_, F_SETFL, flags | O_NONBLOCK);

        struct sockaddr_in address;
        std::memset(&address, 0, sizeof(address));
        address.sin_family = ToAddressFamilyPOSIX(family_);
        address.sin_port = htons(8000);
        address.sin_addr.s_addr = htonl(INADDR_ANY);

        ::bind(
            *fileDescriptor_,
            reinterpret_cast<struct sockaddr*>(&address),
            sizeof(address));
    }

    int GetHandle() const
    {
        assert(fileDescriptor_);
        return *fileDescriptor_;
    }

    Socket Accept()
    {
        assert(fileDescriptor_);
        Socket client(family_, protocolType_);

        struct sockaddr_in address;
        std::memset(&address, 0, sizeof(address));
        address.sin_family = PF_INET;
        address.sin_port = htons(8000);
        address.sin_addr.s_addr = htonl(INADDR_ANY);

        socklen_t length = sizeof(address);

        client.fileDescriptor_ = ::accept(
            *fileDescriptor_,
            reinterpret_cast<struct sockaddr*>(&address),
            &length);

        return std::move(client);
    }

    void Close()
    {
        if (fileDescriptor_) {
            ::close(*fileDescriptor_);
            fileDescriptor_ = somera::NullOpt;
        }
    }
};

} // unnamed namespace

int main(int argc, char *argv[])
{
    Socket server(AddressFamily::InterNetwork, ProtocolType::IPv4);

    printf("%s\n", "=> Bind()");
    server.Bind();
    somera::Defer serverClose([&] {
        server.Close();
    });

    printf("%s\n", "=> ::listen()");
    ::listen(server.GetHandle(), 5);

    std::vector<Socket> clients;
    bool exitRequest = false;

    while (!exitRequest) {
        if (clients.size() < 5)
        {
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(server.GetHandle(), &fds);

            struct timeval tv;
            tv.tv_sec = 0;
            tv.tv_usec = 0;

            int clientCount = select(server.GetHandle() + 1, &fds, nullptr, nullptr, &tv);
            if (clientCount == -1) {
                // error
                return -1;
            }
            assert(clientCount >= 0);
            for (int i = 0; i < clientCount; i++) {
                Socket client = server.Accept();
                printf("%s\n", "=> Accept()");
                clients.push_back(std::move(client));
            }
        }
        for (auto & client : clients) {
            std::vector<char> buffer(1024, 0);
            ssize_t readSize = ::read(client.GetHandle(), buffer.data(), buffer.size() - 1);

            if (readSize == -1) {
                // error: Failed to receive
                continue;
            }

            if (readSize <= 0) {
                continue;
            }

            assert(readSize > 0);

            std::string text(buffer.data(), readSize);
            std::cout << "[client]" << text << std::endl;
            if (somera::StringHelper::startWith(text, "exit")) {
                std::cout << "[exit request]" << std::endl;
                exitRequest = true;
            }

            ::write(client.GetHandle(), text.data(), text.size());
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    for (auto & socket : clients) {
        socket.Close();
    }

    printf("%s\n", "=> Done");
    return 0;
}
