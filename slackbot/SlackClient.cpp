// Copyright (c) 2015 mogemimi. Distributed under the MIT license.

#include "SlackClient.h"
#include <rapidjson/document.h>
#include <cstdint>
#include <sstream>
#include <regex>
#include <utility>

namespace somera {
namespace {

char toHex(char code)
{
    constexpr auto hex = "0123456789ABCDEF";
    return hex[code & 15];
}

bool isSafeCharacter(uint32_t u32)
{
    return ::isalnum(u32)
        || ('-' == u32)
        || ('.' == u32)
        || ('_' == u32)
        || ('~' == u32);
}

std::string encodeURIComponent(const std::string& source)
{
    std::stringstream stream;
    for (const auto& c : source) {
        if (isSafeCharacter(c)) {
            stream << c;
        } else {
            stream << '%' << toHex(c >> 4) << toHex(c & 15);
        }
    }
    return stream.str();
}

namespace QueryString {

std::string concatRequestUrl(
    const std::string& hostName,
    const std::string& path,
    const std::string& postData)
{
    std::stringstream stream;
    stream << hostName << path;
    if (!postData.empty()) {
        stream << '?' << postData;
    }
    return stream.str();
}

std::string stringify(const std::map<std::string, std::string>& params)
{
    std::stringstream stream;
    bool needAmpersand = false;
    for (auto & pair : params) {
        if (needAmpersand) {
            stream << '&';
        }
        stream << pair.first << '=' << encodeURIComponent(pair.second);
        needAmpersand = true;
    }
    return stream.str();
}

} // namespace QueryString
} // unnamed namespace

SlackClient::SlackClient()
{
    http.setTimeout(std::chrono::seconds(15));
}

void SlackClient::apiCall(
    const std::string& method,
    std::map<std::string, std::string> && paramsIn,
    std::function<void(std::string)> && callbackIn)
{
    auto params = std::move(paramsIn);

    assert(!token.empty());
    params["token"] = token;

    constexpr auto hostName = "https://slack.com";
    const auto postData = QueryString::stringify(params);

    const auto url = QueryString::concatRequestUrl(
        hostName,
        "/api/" + method,
        postData);

    http.get(url,
        [callback = std::move(callbackIn), this](bool error, std::vector<std::uint8_t> const& blob) {
            if (error) {
                this->emitError("Failed to call 'api.test'");
                return;
            }
            if (callback) {
                std::string json(reinterpret_cast<char const*>(blob.data()), blob.size());
                callback(json);
            }
        });

    http.waitAll();
}

void SlackClient::onError(std::function<void(std::string)> callback)
{
    errorCallback = callback;
}

void SlackClient::emitError(const std::string& errorMessage)
{
    if (errorCallback) {
        errorCallback(errorMessage);
    }
}

void SlackClient::setToken(const std::string& tokenIn)
{
    this->token = tokenIn;
}

void SlackClient::apiTest(std::function<void(std::string)> callback)
{
    apiCall("api.test", {}, std::move(callback));
}

void SlackClient::authTest(std::function<void(std::string)> callback)
{
    apiCall("auth.test", {}, std::move(callback));
}

void SlackClient::channelsList(std::function<void(std::vector<SlackChannel>)> callbackIn)
{
    apiCall("channels.list", {},
        [callback = std::move(callbackIn), this](const std::string& json) {
            rapidjson::Document doc;
            doc.Parse(json.c_str());

            if (doc.HasParseError()) {
                this->emitError("Failed to parse JSON");
                return;
            }

            if (!doc.HasMember("ok")
                || !doc["ok"].IsBool()
                || !doc.HasMember("channels")
                || !doc["channels"].IsArray()) {
                this->emitError("Invalid JSON");
                return;
            }

            if (!doc["ok"].GetBool()) {
                this->emitError("Invalid JSON");
                return;
            }

            std::vector<SlackChannel> channels;

            auto & channelsObject = doc["channels"];
            for (auto iter = channelsObject.Begin(); iter != channelsObject.End(); ++iter) {
                const auto& channelObject = (*iter);
                if (!channelObject.IsObject()
                    || !channelObject.HasMember("id") || !channelObject["id"].IsString()
                    || !channelObject.HasMember("name") || !channelObject["name"].IsString()
                    || !channelObject.HasMember("creator") || !channelObject["creator"].IsString()
                    || !channelObject.HasMember("created") || !channelObject["created"].IsUint()
                    || !channelObject.HasMember("is_archived") || !channelObject["is_archived"].IsBool()
                    || !channelObject.HasMember("is_general") || !channelObject["is_general"].IsBool()
                    || !channelObject.HasMember("is_member") || !channelObject["is_member"].IsBool()
                    ) {
                    // error
                    continue;
                }

                SlackChannel channel;
                channel.id = (*iter)["id"].GetString();
                channel.name = (*iter)["name"].GetString();
                channel.creator = (*iter)["creator"].GetString();
                channel.created = (*iter)["created"].GetInt();
                channel.is_archived = (*iter)["is_archived"].GetBool();
                channel.is_general = (*iter)["is_general"].GetBool();
                channel.is_member = (*iter)["is_member"].GetBool();
                channels.push_back(std::move(channel));
            }

            if (callback) {
                callback(std::move(channels));
            }
        });
}

void SlackClient::chatPostMessage(
    const std::string& channel,
    const std::string& text,
    const std::string& username,
    const std::string& icon_url,
    std::function<void(std::string)> callbackIn)
{
    std::map<std::string, std::string> params;
    params["channel"] = channel;
    params["text"] = text;
    if (!username.empty()) {
        params["username"] = username;
    }
    if (!icon_url.empty()) {
        params["icon_url"] = icon_url;
    }

    apiCall("chat.postMessage", std::move(params), std::move(callbackIn));
}

} // namespace somera
