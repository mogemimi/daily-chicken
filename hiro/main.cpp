// Copyright (c) 2015 mogemimi. Distributed under the MIT license.

#include "../daily/CommandLineParser.h"
#include "../daily/StringHelper.h"
#include <iostream>
#include <fstream>

using namespace somera;

namespace {

void setupCommandLineParser(CommandLineParser & parser)
{
    using somera::CommandLineArgumentType::Flag;
    using somera::CommandLineArgumentType::JoinedOrSeparate;
    parser.setUsageText("hiro [options ...] [source file ...]");
    parser.addArgument("-h", Flag, "Display available options");
    parser.addArgument("-help", Flag, "Display available options");
}

void refactorSourceCode(const std::string& path)
{
    std::ifstream input(path);
    if (!input) {
        return;
    }
    std::istreambuf_iterator<char> start(input);
    std::istreambuf_iterator<char> end;
    std::string text(start, end);
    input.close();

    std::ofstream output(path, std::ios::out | std::ios::trunc);
    if (!output) {
        return;
    }
    constexpr auto from =
        "// Copyright (c) 2013-2015 mogemimi.\n"
        "// Distributed under the MIT license. See LICENSE.md file for details.";
    constexpr auto to =
        "// Copyright (c) 2013-2015 mogemimi. Distributed under the MIT license.";
    text = StringHelper::replace(text, from, to);
    output << text;
}

} // unnamed namespace

int main(int argc, char *argv[])
{
    CommandLineParser parser;
    setupCommandLineParser(parser);
    parser.parse(argc, argv);

    if (parser.hasParseError()) {
        std::cerr << parser.getErrorMessage() << std::endl;
        return 1;
    }
    if (parser.exists("-h") || parser.exists("-help")) {
        std::cout << parser.getHelpText() << std::endl;
        return 0;
    }
    if (parser.getPaths().empty()) {
        std::cerr << "error: no input file" << std::endl;
        return 1;
    }

    for (auto & path : parser.getPaths()) {
        refactorSourceCode(path);
    }
    return 0;
}
