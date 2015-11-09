// Copyright (c) 2015 mogemimi. Distributed under the MIT license.

#pragma once

extern "C" {
#include <curl/curl.h>
}
#include <chrono>
#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>

namespace somera {

class HttpRequest {
private:
    std::function<void(bool error, std::vector<std::uint8_t> const& data)> callback;
    std::vector<std::uint8_t> blob;
    CURL* curl;

public:
    HttpRequest(
        std::string const& uri,
        std::function<void(bool error, std::vector<std::uint8_t> const& data)> callback);

    ~HttpRequest();

    HttpRequest(HttpRequest const&) = delete;
    HttpRequest & operator=(HttpRequest const&) = delete;

    CURL* getCurl() const;
    void onCompleted();
    void onError();

    void setTimeout(const std::chrono::seconds& timeout);

private:
    static void writeCallback(
        void const* contents,
        size_t size,
        size_t nmemb,
        void* userPointer);
};

class HttpService {
private:
    CURLM* multiHandle;
    std::map<CURL*, std::unique_ptr<HttpRequest>> sessions;
    std::chrono::seconds timeout;

public:
    HttpService();
    ~HttpService();

    HttpService(HttpService const&) = delete;
    HttpService & operator=(HttpService const&) = delete;

    void setTimeout(const std::chrono::seconds& timeout);

    void poll();

    void waitAll();

    void get(
        std::string const& uri,
        std::function<void(bool, std::vector<std::uint8_t> const&)> callback);

    bool empty() const;
};

} // namespace somera
