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
        std::string key;
        for (int i = 1; i < argc; i++) {
            if (key.empty() || (*argv[i]) == '-') {
                key = argv[i];
                arguments[key];
            }
            else {
                arguments[key] = argv[i];
            }
        }
    }

    bool exists(const std::string& flag) const
    {
        auto iter = arguments.find(flag);
        return iter != std::end(arguments);
    }

    std::string get(const std::string& flag) const
    {
        auto iter = arguments.find(flag);
        if (iter != std::end(arguments)) {
            return iter->second;
        }
        return "";
    }

private:
    std::string executablePath;
    std::map<std::string, std::string> arguments;
};

void chatNowPlayingMusic(somera::SlackClient & slack, const std::string& channelName)
{
    auto channel = slack.getChannelByName(channelName);
    if (!channel) {
        // error
        std::cout << "Could not find channel." << std::endl;
        return;
    }

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

    slack.chatPostMessage(
        somera::SlackChatPostMessageOptions {
            .channel = channel->id,
            .text = message,
            .as_user =  true,
        },
        [](std::string json) {
            std::cout << json << std::endl;
        });
}

void historyToMarkdown(
    somera::SlackClient & slack,
    const std::string& channelName,
    const std::string& markdownFileName)
{
    auto channel = slack.getChannelByName(channelName);
    if (!channel) {
        // error
        std::cout << "Could not find channel." << std::endl;
        return;
    }

    bool hasMore = true;
    somera::Optional<std::string> latest;

    std::vector<somera::SlackMessage> messages;

    while (hasMore) {
        slack.channelsHistory(
            somera::SlackChannelsHistoryOptions {
                .channel = channel->id,
                .latest = latest,
            },
            [&](somera::SlackHistory history) {
                messages.insert(
                    std::end(messages),
                    history.messages.begin(),
                    history.messages.end());
                hasMore = history.has_more;
                latest = history.messages.back().ts;
                if (hasMore) {
                    std::cout << "read more..." << std::endl;
                } else {
                    std::cout << "completed." << std::endl;
                }
            });
    }

    std::ofstream stream(markdownFileName);

    stream << "# " << channelName << std::endl;
    stream << std::endl;

    std::reverse(std::begin(messages), std::end(messages));
    std::chrono::system_clock::time_point prevTimestamp;
    if (!messages.empty()) {
        prevTimestamp = messages.front().timestamp;
    }

    for (auto & message : messages) {
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
    }
}

void showHelp()
{
    std::printf("%*s %s\n", 18,
        "-h", "Show help documents");
    std::printf("%*s %s\n", 18,
        "-help", "Show help documents");
    std::printf("%*s %s\n", 18,
        "-token-from-env", "Get the access token from environment path");
    std::printf("%*s %s\n", 18,
        "-token-from-file", "Get the access token from file");

    std::printf("%*s %s\n", 18,
        "-nowplaying", "Chat now playing music.");
    std::printf("%*s %s\n", 18,
        "-history-markdown", "Write channel history to markdown file");
    std::printf("%*s %s\n", 18,
        "-api-test", "Call 'api.test' method");
    std::printf("%*s %s\n", 18,
        "-auth-test", "Call 'auth.test' method");

    std::cout
        << std::endl
        << "### Example ###" << std::endl
        << "# ./slackbot -nowplaying -channel general" << std::endl
        << "# ./slackbot -history-markdown -channel general" << std::endl
        << "# ./slackbot -history-markdown -channel general -file readme.md" << std::endl
        << "# ./slackbot -api-test" << std::endl
        << "# ./slackbot -auth-test" << std::endl;
}

} // unnamed namespace

int main(int argc, char *argv[])
{
    CommandLineParser parser(argc, argv);

    if (parser.exists("-h") || parser.exists("-help")) {
        showHelp();
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

    if (parser.exists("-nowplaying")) {
        std::string channel = parser.get("-channel");
        if (!slack.getChannelByName(channel)) {
            std::printf("Channot find channel.");
        }
        chatNowPlayingMusic(slack, channel);
    }
    else if (parser.exists("-history-markdown")) {
        std::string channel = parser.get("-channel");
        if (!slack.getChannelByName(channel)) {
            std::printf("Channot find channel.");
        }

        std::string outputFile = channel + ".md";
        if (parser.exists("-file")) {
            outputFile = parser.get("-file");
        }
        historyToMarkdown(slack, channel, outputFile);
    }
    else if (parser.exists("-api-test")) {
        slack.apiTest([](const std::string& json) {
            std::cout << json << std::endl;
        });
    }
    else if (parser.exists("-auth-test")) {
        slack.authTest([](const std::string& json) {
            std::cout << json << std::endl;
        });
    }
    else {
        showHelp();
    }
    return 0;
}
