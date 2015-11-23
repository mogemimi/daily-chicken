// Copyright (c) 2015 mogemimi. Distributed under the MIT license.

#include "CommandLineParser.h"
#include "StringHelper.h"
#include <cassert>
#include <utility>
#include <algorithm>

namespace somera {

void CommandLineParser::addArgument(
    const std::string& flag,
    CommandLineArgumentType type,
    const std::string& help)
{
    assert(!flag.empty());
    CommandLineArgumentHint hint;
    hint.name = flag;
    hint.help = help;
    hint.type = type;
    hints.push_back(std::move(hint));

    using Hint = const CommandLineArgumentHint &;
    std::sort(std::begin(hints), std::end(hints),
        [](Hint a, Hint b){ return StringHelper::toLower(a.name) < StringHelper::toLower(b.name); });
}

std::string CommandLineParser::getHelpText() const
{
    constexpr auto indent = "  ";
    const std::string spaces = "                        ";
    std::stringstream stream;
    for (auto & hint : hints) {
        stream << indent;
        auto option = hint.name;
        if (hint.type == CommandLineArgumentType::JoinedOrSeparate) {
            option += " <value>";
        }

        bool needToIndent = false;
        stream << option;
        if (option.size() < spaces.size()) {
            stream << spaces.substr(option.size());
        } else {
            stream << '\n';
            needToIndent = true;
        }

        std::stringstream helpStream(hint.help);
        std::string helpLine;
        while (std::getline(helpStream, helpLine)) {
            if (needToIndent) {
                stream << indent << spaces;
            }
            stream << helpLine << '\n';
            needToIndent = true;
        }
    }
    return stream.str();
}

void CommandLineParser::parse(int argc, const char *argv[])
{
    assert(argc > 0);
    assert(argv != nullptr);

    if (argc >= 1) {
        executablePath = argv[0];
    }

    for (int i = 1; i < argc; i++) {
        std::string argument = argv[i];
        if (!StringHelper::startWith(argument, "-")) {
            paths.push_back(std::move(argument));
            continue;
        }

        bool found = false;
        for (auto & hint : hints) {
            if (hint.type == CommandLineArgumentType::Flag) {
                if (hint.name == argument) {
                    flags.insert(std::move(argument));
                    found = true;
                    break;
                }
            }
            else if (hint.type == CommandLineArgumentType::JoinedOrSeparate) {
                if (StringHelper::startWith(argument, hint.name)) {
                    hint.values.push_back(argument.substr(hint.name.size()));
                    found = true;
                    break;
                }
            }
        }
        if (!found) {
            if (StringHelper::startWith(argument, "--")) {
                errorMessage << "error: unsupported option: '" << argument << "'\n";

            } else {
                errorMessage << "error: unknown argument: '" << argument << "'\n";
            }
        }
    }
}

bool CommandLineParser::hasParseError() const
{
    return !errorMessage.str().empty();
}

std::string CommandLineParser::getErrorMessage() const
{
    return errorMessage.str();
}

std::string CommandLineParser::getExecutablePath() const
{
    return executablePath;
}

bool CommandLineParser::exists(const std::string& flag) const
{
    auto iter = flags.find(flag);
    return iter != std::end(flags);
}

std::vector<std::string> CommandLineParser::getValues(const std::string& name) const
{
    auto iter = std::find_if(hints.begin(), hints.end(),
        [&](const CommandLineArgumentHint& hint){ return hint.name == name; });
    if (iter == hints.end()) {
        return {};
    }
    return iter->values;
}

std::vector<std::string> CommandLineParser::getPaths() const
{
    return paths;
}

} // namespace somera
