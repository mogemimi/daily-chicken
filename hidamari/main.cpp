// Copyright (c) 2015 mogemimi. Distributed under the MIT license.

#include "CommandLineParser.h"
#include <iostream>
#include <vector>
#include <string>

using somera::CommandLineParser;
using somera::CommandLineArgumentType;

namespace somera {

struct CompileOptions {
    std::vector<std::string> sources;
    std::vector<std::string> includePaths;
    std::vector<std::string> libraies;
    std::vector<std::string> librarySearchPaths;
};

//-g

} // namespace somera

int main(int argc, char *argv[])
{
    CommandLineParser parser;
    parser.addArgument("-h", CommandLineArgumentType::Flag,
        "help");
    parser.addArgument("-help", CommandLineArgumentType::Flag,
        "help");
    parser.addArgument("-I", CommandLineArgumentType::StartWith,
        "include search path");
    parser.addArgument("-L", CommandLineArgumentType::StartWith,
        "library search path");
    parser.addArgument("-fno-exceptions", CommandLineArgumentType::Flag,
        "TODO");
    parser.parse(argc, argv);

    if (parser.hasParseError()) {
        std::cerr << parser.getErrorMessage() << std::endl;
        return 1;
    }

    if (parser.getPaths().empty()) {
        std::cerr << "error: no input file" << std::endl;
        return 1;
    }

    if (parser.exists("-h") || parser.exists("-help")) {
        std::cout << parser.getHelpText() << std::endl;
        return 0;
    }

    for (auto & path : parser.getValues("-I")) {
        std::cout << "[-I] " << path << std::endl;
    }
    for (auto & path : parser.getValues("-L")) {
        std::cout << "[-L] " << path << std::endl;
    }
    for (auto & path : parser.getPaths()) {
        std::cout << "[Path] " << path << std::endl;
    }

    std::cout << "finished" << std::endl;
    return 0;
}
