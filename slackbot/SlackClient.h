// Copyright (c) 2015 mogemimi. Distributed under the MIT license.

#include "../somerachan/src/optional.h"
#include "HttpService.h"
#include <functional>
#include <string>
#include <vector>
#include <map>
#include <chrono>

namespace somera {

//struct SlackTopic {
//    std::string value;
//    std::string creator;
//    int last_set;
//};
//
//struct SlackPurpose {
//    std::string value;
//    std::string creator;
//    int last_set;
//};

struct SlackChannel {
//    std::vector<std::string> members;
//    SlackTopic topic;
//    SlackPurpose purpose;
    std::string id;
    std::string name;
    int created;
    std::string creator;
    bool is_archived;
    bool is_general;
    bool is_member;
};

struct SlackMessage {
    std::string type;
    std::string channel;
    std::string user;
    std::string text;
    std::string subtype;
    std::chrono::system_clock::time_point timestamp;
};

struct SlackHistory {
    std::vector<SlackMessage> messages;
    std::string latest;
    bool has_more;
};

class SlackClient final {
private:
    somera::HttpService http;
    std::string token;
    std::function<void(const std::string&)> errorCallback;

public:
    SlackClient();

    void setToken(const std::string& tokenIn);

    /// See https://api.slack.com/methods/api.test
    void apiTest(std::function<void(std::string)> callback);

    /// See https://api.slack.com/methods/auth.test
    void authTest(std::function<void(std::string)> callback);

    /// See https://api.slack.com/methods/channels.list
    void channelsList(std::function<void(std::vector<SlackChannel>)> callback);

    /// See https://api.slack.com/methods/chat.postMessage
    void chatPostMessage(
        const std::string& channel,
        const std::string& text,
        const std::string& username,
        const std::string& icon_url,
        std::function<void(std::string)> callback);

    /// See https://api.slack.com/methods/channels.history
    void channelsHistory(
        const std::string& channel,
        std::function<void(SlackHistory)> callback);

    void onError(std::function<void(std::string)> callback);

private:
    void apiCall(
        const std::string& method,
        std::map<std::string, std::string> && params,
        std::function<void(std::string)> && callback);

    void emitError(const std::string& errorMessage);
};

} // namespace somera
