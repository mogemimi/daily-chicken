// Copyright (c) 2015 mogemimi. Distributed under the MIT license.

#include "SlackClient.h"
#include "HttpUtility.h"
#include <rapidjson/document.h>
#include <cstdint>
#include <sstream>
#include <utility>

namespace somera {

SlackClient::SlackClient()
{
    http.setTimeout(std::chrono::seconds(60));
}

void SlackClient::apiCall(
    const std::string& method,
    std::map<std::string, std::string> && paramsIn,
    std::function<void(std::string)> && callbackIn)
{
    auto params = std::move(paramsIn);

    assert(!token.empty());
    params["token"] = token;

    const auto postData = HttpUtility::stringify(params);

    somera::HttpRequestOptions options;
    options.hostname = "https://slack.com";
    options.path = "/api/" + method;
    options.postFields = postData;
    options.method = HttpRequestMethod::POST;

    http.request(options,
        [callback = std::move(callbackIn), this](bool error, const std::vector<std::uint8_t>& blob) {
            if (error) {
                this->emitError("Failed to call 'api.test'");
                return;
            }
            if (callback) {
                std::string json(reinterpret_cast<const char*>(blob.data()), blob.size());
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

            if (!doc.IsObject()
                || !doc.HasMember("ok") || !doc["ok"].IsBool()
                || !doc.HasMember("channels") || !doc["channels"].IsArray()
                ) {
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
    std::function<void(std::string)> callback)
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

    apiCall("chat.postMessage", std::move(params), std::move(callback));
}

void SlackClient::channelsHistory(
    const std::string& channel,
    std::function<void(SlackHistory)> callbackIn)
{
    assert(!channel.empty());

    std::map<std::string, std::string> params;
    params["channel"] = channel;

    auto callbackWrapper = [callback = std::move(callbackIn), this](const std::string& json) {
        rapidjson::Document doc;
        doc.Parse(json.c_str());

        if (doc.HasParseError()) {
            this->emitError("Failed to parse JSON");
            return;
        }

        if (!doc.IsObject()
            || !doc.HasMember("ok") || !doc["ok"].IsBool()) {
            this->emitError("Invalid JSON");
            return;
        }

        if (!doc["ok"].GetBool()) {
            std::string errorMessage = "Failed to call channelsHistory";
            if (doc.HasMember("error") && doc["error"].IsString()) {
                errorMessage = doc["error"].GetString();
            }
            this->emitError(errorMessage);
            return;
        }

        SlackHistory history;

        if (doc.HasMember("latest") && doc["latest"].IsString()) {
            history.latest = doc["latest"].GetString();
        }

        if (doc.HasMember("has_more") && doc["has_more"].IsBool()) {
            history.has_more = doc["has_more"].GetBool();
        } else {
            history.has_more = false;
        }

        if (doc.HasMember("messages") && doc["messages"].IsArray()) {
            auto & messagesObject = doc["messages"];
            for (auto iter = messagesObject.Begin(); iter != messagesObject.End(); ++iter) {
                const auto& channelObject = (*iter);
                if (!channelObject.IsObject()
                    || !channelObject.HasMember("type") || !channelObject["type"].IsString()
                    || !channelObject.HasMember("ts") || !channelObject["ts"].IsString()
                    ) {
                    // error
                    std::fprintf(stderr, "JSON parse error in %s, %d\n", __FILE__, __LINE__);
                    continue;
                }

                SlackMessage message;
                message.type = (*iter)["type"].GetString();
                {
                    auto timestamp = (*iter)["ts"].GetString();
                    using std::chrono::system_clock;
                    message.timestamp = system_clock::from_time_t(::atoi(timestamp));
                }

                if (channelObject.HasMember("user")
                    && channelObject["user"].IsString()) {
                    message.user = (*iter)["user"].GetString();
                }
                if (channelObject.HasMember("text")
                    && channelObject["text"].IsString()) {
                    message.text = (*iter)["text"].GetString();
                }
                if (channelObject.HasMember("channel")
                    && channelObject["channel"].IsString()) {
                    message.channel = (*iter)["channel"].GetString();
                }
                if (channelObject.HasMember("subtype")
                    && channelObject["subtype"].IsString()) {
                    message.subtype = (*iter)["subtype"].GetString();
                }

                history.messages.push_back(std::move(message));
            }
        }

        if (callback) {
            callback(std::move(history));
        }
    };

    apiCall("channels.history", std::move(params), std::move(callbackWrapper));
}

} // namespace somera
