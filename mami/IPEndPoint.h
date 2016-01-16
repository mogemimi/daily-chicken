// Copyright (c) 2016 mogemimi. Distributed under the MIT license.

#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <string>

namespace somera {

enum class AddressFamily {
    InterNetwork,
    InterNetworkV6,
};

struct AddressViewPOSIX final {
    const ::sockaddr* data = nullptr;
    ::socklen_t size = 0;
};

class IPEndPoint final {
public:
    AddressFamily GetFamily() const;

    AddressViewPOSIX GetAddressViewPOSIX() const;

    std::string GetAddressNumber() const;

    int GetPort() const;

    static IPEndPoint CreateFromAddressStorage(const ::sockaddr_storage& storage);

    static IPEndPoint CreateFromV4(const std::string& internetAddress, uint16_t port);

    static IPEndPoint CreateFromV6(uint16_t port, uint32_t scopeId);

private:
    AddressFamily family;
    union {
        ::sockaddr_in asV4;
        ::sockaddr_in6 asV6;
    } address;
};

} // namespace somera
