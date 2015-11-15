// Copyright (c) 2015 mogemimi. Distributed under the MIT license.

#include "HttpService.h"
#include "SlackClient.h"
#include "iTunesNowPlaying.h"
#include "TerminalHelper.h"
#include <iostream>
#include <thread>
#include <fstream>
#include <regex>

namespace {
namespace StringHelper {

std::string replace(const std::string& source, const std::string& from, const std::string& to)
{
    if (from.empty()) {
        return source;
    }
    auto result = source;
    std::string::size_type start = 0;
    while ((start = result.find(from, start)) != std::string::npos) {
        result.replace(start, from.length(), to);
        start += to.length();
    }
    return std::move(result);
}

} // namespace StringHelper

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

#if 0
void chatNowPlaying(somera::SlackClient & slack, const std::string& channelId)
{
    auto track = somera::iTunesNowPlaying::getCurrentTrack();
    if (!track) {
        std::cout << "Your iTunes is not enabled." << std::endl;
        return;
    }

    std::string message;
    message += u8":saxophone: ";
    message += "*" + track->trackName + "*" + "  ";
    message += u8":simple_smile: ";
    message += "_" + track->artistName + "_" + "  ";
    message += u8":cd: ";
    message += "_" + track->albumName + "_";

    constexpr auto botName = u8"Somerachan";
    constexpr auto iconImage = "https://example.com/a.jpg";

    slack.chatPostMessage(
        channelId,
        message,
        botName,
        iconImage,
        [](std::string json) {
            std::cout << json << std::endl;
        });
}
#endif

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

    somera::SlackClient slack(token);

    slack.onError([](std::string message) {
        std::cout << message << std::endl;
    });

    slack.login();

    const std::string channelName = u8"general";
    auto channel = slack.getChannelByName(channelName);

    if (!channel) {
        // error
        std::cout << "Could not find channel." << std::endl;
        return 1;
    }

    slack.channelsHistory(channel->id, [&](somera::SlackHistory history) {
        std::reverse(std::begin(history.messages), std::end(history.messages));

        std::ofstream stream("nyan.md");

        stream << "# " << channelName << std::endl;
        stream << std::endl;

        std::chrono::system_clock::time_point prevTimestamp;
        if (!history.messages.empty()) {
            prevTimestamp = history.messages.front().timestamp;
        }

        for (auto & message : history.messages) {
            if (message.type != "message") {
                continue;
            }
            if (message.subtype == "channel_join") {
                continue;
            }
            if (message.subtype == "file_share") {
                continue;
            }
            if (message.timestamp - prevTimestamp > std::chrono::minutes(5)) {
                stream << "----" << std::endl;
                stream << std::endl;
            }

            // NOTE: Insertion linebreak for markdown's list
            std::regex re(R"((^[^\-][^\n]*\n)(\-\s[^\n]+))");
            auto result = std::regex_replace(message.text, re, "$1\n$2");
            result = StringHelper::replace(result, "&gt;", ">");
            result = StringHelper::replace(result, "&lt;", "<");

            stream << result << std::endl;
            stream << std::endl;
            prevTimestamp = message.timestamp;
//            std::cout << message.user << std::endl;
//            std::cout << message.text << std::endl;
//            std::cout << message.channel << std::endl;
//            std::cout << message.type << std::endl;
//            std::cout << message.ts << std::endl;
//            std::cout << "------" << std::endl;
        }
        std::cout << "success." << std::endl;
    });

    return 0;
}
