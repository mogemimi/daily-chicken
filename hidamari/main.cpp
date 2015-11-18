// Copyright (c) 2015 mogemimi. Distributed under the MIT license.

#include "CommandLineParser.h"
#include <iostream>
#include <vector>
#include <string>

#include "StringHelper.h"
#include <fstream>

using somera::CommandLineParser;
using somera::CommandLineArgumentType;

namespace somera {

struct CompileOptions {
    std::vector<std::string> sources;
    std::vector<std::string> includePaths;
    std::vector<std::string> libraies;
    std::vector<std::string> librarySearchPaths;
};

void setupCommandLineParser(CommandLineParser & parser)
{
    parser.addArgument("-h", CommandLineArgumentType::Flag,
        "Display available options");
    parser.addArgument("-help", CommandLineArgumentType::Flag,
        "Display available options");
    parser.addArgument("-I", CommandLineArgumentType::StartWith,
        "Add directory to include search path");
    parser.addArgument("-L", CommandLineArgumentType::StartWith,
        "Add directory to library search path");
    parser.addArgument("-l", CommandLineArgumentType::StartWith,
        "Search the library when linking");
    parser.addArgument("-fno-exceptions", CommandLineArgumentType::Flag,
        "no-exceptions");
    parser.addArgument("-generate=", CommandLineArgumentType::StartWith,
        "Generate project for the project file format "
        "(xcode, msbuild, cmake or gyp)");
}

std::string fileToString(const std::string& source)
{
    std::ifstream input(source);
    std::string line;
    std::stringstream stream;
    while (input && std::getline(input, line)) {
        line = somera::StringHelper::replace(line, "\"", "\\\"");
        line = somera::StringHelper::replace(line, "\t", "  ");
        stream << "\"";
        stream << line;
        stream << "\\n\"\n";
    }
    return stream.str();
}

} // namespace somera

#include "ProjectTemplate.h"

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

    for (auto & path : parser.getValues("-I")) {
        std::cout << "[-I] " << path << std::endl;
    }
    for (auto & path : parser.getValues("-L")) {
        std::cout << "[-L] " << path << std::endl;
    }
    for (auto & path : parser.getValues("-l")) {
        std::cout << "[-l] " << path << std::endl;
    }
    for (auto & path : parser.getPaths()) {
//        std::cout << "[Path] " << path << std::endl;

        std::cout << std::endl;
        std::cout << somera::fileToString(path);
        std::cout << std::endl;
    }

    return 0;
}
