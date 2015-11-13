// Copyright (c) 2015 mogemimi. Distributed under the MIT license.

#include "HttpService.h"
#include "SlackClient.h"
#include "iTunesNowPlaying.h"
#include "TerminalHelper.h"
#include <iostream>
#include <thread>
#include <fstream>

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

struct CommandLineParser {
    CommandLineParser(int argc, char *argv[])
    {
        assert(argc > 0);
        assert(argv != nullptr);
        if (argc >= 1) {
            executablePath = argv[0];
        }
        for (int i = 1; i < argc; i++) {
            arguments.emplace_back(argv[i]);
        }
    }

    bool exists(const std::string& flag) const
    {
        auto iter = std::find(std::begin(arguments), std::end(arguments), flag);
        return iter != std::end(arguments);
    }

private:
    std::string executablePath;
    std::vector<std::string> arguments;
};

} // unnamed namespace

int main(int argc, char *argv[])
{
    CommandLineParser parser(argc, argv);

    if (parser.exists("-h") || parser.exists("-help")) {
        std::printf("%*s %s\n", 15,
            "-h", "Show help documents");
        std::printf("%*s %s\n", 15,
            "-help", "Show help documents");
        std::printf("%*s %s\n", 15,
            "-token-from-env", "Get the access token from environment path");
        std::printf("%*s %s\n", 15,
            "-token-from-file", "Get the access token from file");
        return 0;
    }

    std::string token;
    if (parser.exists("-token-from-env")) {
        token = ::getenv("SLACKBOT_ACCESS_TOKEN");
        if (token.empty()) {
            std::printf("Cannot find your access token.");
            return 1;
        }
    }
    else {
    //else if (parser.exists("-token-from-file")) {
        auto path = somera::getHomeDirectory() + "/.slackbot/access_token";
        std::ifstream stream(path);
        if (!stream) {
            std::printf("Cannot find file %s", path.c_str());
            return 1;
        }
        std::getline(stream, token);
        if (token.empty()) {
            std::printf("Invalid token from %s", path.c_str());
            return 1;
        }
    }

    somera::SlackClient slack;
    slack.setToken(token);

    slack.onError([](std::string message) {
        std::cout << message << std::endl;
    });

//    slack.apiTest([](std::string json) {
//        std::cout << json << std::endl;
//    });

//    slack.apiTest().then([](std::string json) {
//        std::cout << json << std::endl;
//    });

//    slack.authTest([](std::string json) {
//        std::cout << json << std::endl;
//    });

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

//    slack.channelsHistory(channelId, [](const somera::SlackHistory& history) {
//        for (auto & message : history.messages) {
//            std::cout << message.user << std::endl;
//            std::cout << message.text << std::endl;
//            std::cout << message.channel << std::endl;
//            std::cout << message.type << std::endl;
//            std::cout << message.ts << std::endl;
//            std::cout << "------" << std::endl;
//        }
//    });

    auto track = somera::iTunesNowPlaying::getCurrentTrack();
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
