// Copyright (c) 2015 mogemimi. Distributed under the MIT license.

#include "HttpService.h"
#include <algorithm>
#include <cassert>

namespace somera {

HttpRequest::HttpRequest(
    std::string const& uri,
    std::function<void(bool error, std::vector<std::uint8_t> const& data)> callbackIn)
    : callback(callbackIn)
    , curl(nullptr)
{
    curl = curl_easy_init();

    if (curl == nullptr) {
        throw std::runtime_error("curl_easy_init() failed");
    }

    curl_easy_setopt(curl, CURLOPT_URL, uri.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
}

HttpRequest::~HttpRequest()
{
    if (curl != nullptr) {
        curl_easy_cleanup(curl);
        curl = nullptr;
    }
}

CURL* HttpRequest::getCurl() const
{
    return curl;
}

void HttpRequest::setTimeout(const std::chrono::seconds& timeout)
{
    if (timeout >= timeout.zero()) {
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout.count());
    }
}

void HttpRequest::writeCallback(
    void const* contents,
    size_t size,
    size_t nmemb,
    void* userPointer)
{
    auto userData = reinterpret_cast<HttpRequest*>(userPointer);
    auto & blob = userData->blob;

    auto dataLength = size * nmemb;
    blob.resize(blob.size() + dataLength);
    std::memcpy(blob.data() + blob.size() - dataLength, contents, dataLength);
}

void HttpRequest::onCompleted()
{
    if (callback) {
        callback(false, blob);
    }
}

void HttpRequest::onError()
{
    if (callback) {
        callback(true, {});
    }
}

HttpService::HttpService()
    : multiHandle(nullptr)
    , timeout(decltype(timeout)::zero())
{
    multiHandle = curl_multi_init();
}

HttpService::~HttpService()
{
    if (multiHandle != nullptr) {
        curl_multi_cleanup(multiHandle);
        multiHandle = nullptr;
    }
}

void HttpService::poll()
{
    assert(multiHandle != nullptr);

    int runningCount = 0;
    curl_multi_perform(multiHandle, &runningCount);

    int messageCount = 0;
    CURLMsg* message = curl_multi_info_read(multiHandle, &messageCount);

    while (message != nullptr)
    {
        assert(message->msg == CURLMSG_DONE);
        CURL* curl = message->easy_handle;

        curl_multi_remove_handle(multiHandle, curl);

        auto pair = sessions.find(curl);
        assert(pair != std::end(sessions));

        if (pair != std::end(sessions)) {
            auto & request = pair->second;
            request->onCompleted();
            sessions.erase(pair);
        }

        message = curl_multi_info_read(multiHandle, &messageCount);
    }
}

void HttpService::waitAll()
{
    for (;;) {
        this->poll();
        if (this->empty()) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::microseconds(200));
    }
}

void HttpService::get(
    std::string const& uri,
    std::function<void(bool, std::vector<std::uint8_t> const&)> callback)
{
    auto request = std::make_unique<HttpRequest>(uri, callback);
    if (timeout > decltype(timeout)::zero()) {
        request->setTimeout(timeout);
    }

    auto result = curl_multi_add_handle(multiHandle, request->getCurl());
    if (result != CURLE_OK) {
        throw std::runtime_error("curl_multi_add_handle() failed");
    }
    sessions.emplace(request->getCurl(), std::move(request));
}

bool HttpService::empty() const
{
    return sessions.size() <= 0;
}

void HttpService::setTimeout(const std::chrono::seconds& timeoutIn)
{
    assert(timeout >= timeout.zero());
    this->timeout = timeoutIn;
}

} // namespace somera
