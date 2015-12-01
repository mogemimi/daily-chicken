// Copyright (c) 2015 mogemimi. Distributed under the MIT license.

#include "daily/CommandLineParser.h"
#include "daily/FileSystem.h"
#include "daily/Optional.h"
#include "daily/StringHelper.h"
#include <iostream>
#include <fstream>

using somera::CommandLineParser;
using somera::Optional;
using somera::NullOpt;
namespace StringHelper = somera::StringHelper;

namespace {

void setupCommandLineParser(CommandLineParser & parser)
{
    using somera::CommandLineArgumentType::Flag;
    using somera::CommandLineArgumentType::JoinedOrSeparate;
    parser.setUsageText("miyako [options ...] [build file ...]");
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

    std::cout << text << std::endl;
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
