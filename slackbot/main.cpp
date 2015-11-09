// Copyright (c) 2015 mogemimi. Distributed under the MIT license.

#include "HttpService.h"
#include "SlackClient.h"
#include "iTunesNowplaying.h"
#include <iostream>
#include <thread>

namespace {

template <typename Container, typename Func, typename T>
auto findIf(const Container& container, Func thunk, const T& value)
    -> typename Container::const_iterator
{
    return std::find_if(std::begin(container), std::end(container),
            [&](const typename Container::value_type& v) {
                return thunk(v) == value;
            });
}

auto findChannel(const std::vector<somera::SlackChannel>& channels, const std::string& name)
    -> decltype(channels.begin())
{
    return findIf(channels,
        [](const somera::SlackChannel& channel){ return channel.name; },
        name);
}

} // unnamed namespace

int main(int argc, char *argv[])
{
//    const auto token = ::getenv("SLACK_BOT_ACCESS_TOKEN");
//    if (token == nullptr) {
//        std::printf("Cannot find your access token.");
//        return 1;
//    }
    const auto token = "xxxx";

    somera::SlackClient slack;
    slack.setToken(token);

    slack.onError([](std::string message) {
        std::cout << message << std::endl;
    });

    slack.apiTest([](std::string json) {
        std::cout << json << std::endl;
    });

//    slack.apiTest().then([](std::string json) {
//        std::cout << json << std::endl;
//    });

    slack.authTest([](std::string json) {
        std::cout << json << std::endl;
    });

    std::string channelId;
    slack.channelsList([&](std::vector<somera::SlackChannel> channels) {
        auto iter = findChannel(channels, u8"sandbox");
        if (iter != channels.end()) {
            channelId = iter->id;
        }
    });

    if (channelId.empty()) {
        // error
        std::cout << "Could not find channel." << std::endl;
        return 1;
    }

    auto track = somera::iTunesNowplaying::getCurrentTrack();
    if (!track) {
        std::cout << "Your iTunes is not enabled." << std::endl;
        return 0;
    }

    std::string message;
    message += u8":saxophone: ";
    message += "*" + track->trackName + "*" + "  ";
    message += u8":simple_smile: ";
    message += "_" + track->artistName + "_" + "  ";
    message += u8":cd: ";
    message += "_" + track->albumName + "_";

    slack.chatPostMessage(
        channelId,
        message,
        u8"Somera-chan",
        "https://example.com/a.jpg",
        [](std::string json) {
            std::cout << json << std::endl;
        });

    return 0;
}
