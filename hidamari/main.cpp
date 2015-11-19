// Copyright (c) 2015 mogemimi. Distributed under the MIT license.

#include "CommandLineParser.h"
#include "ProjectTemplate.h"
#include "StringHelper.h"
#include <iostream>
#include <vector>
#include <string>
#include <fstream>

using somera::CommandLineParser;
using somera::CommandLineArgumentType;

namespace somera {

void setupCommandLineParser(CommandLineParser & parser)
{
    parser.addArgument("-h", CommandLineArgumentType::Flag,
        "Display available options");
    parser.addArgument("-help", CommandLineArgumentType::Flag,
        "Display available options");
    parser.addArgument("-I", CommandLineArgumentType::JoinedOrSeparate,
        "Add directory to include search path");
    parser.addArgument("-L", CommandLineArgumentType::JoinedOrSeparate,
        "Add directory to library search path");
    parser.addArgument("-l", CommandLineArgumentType::JoinedOrSeparate,
        "Search the library when linking");
    parser.addArgument("-fno-exceptions", CommandLineArgumentType::Flag,
        "no-exceptions");
    parser.addArgument("-generator=", CommandLineArgumentType::JoinedOrSeparate,
        "Generate project for the project file format "
        "(xcode, msbuild, cmake or gyp)");
    parser.addArgument("-output=", CommandLineArgumentType::JoinedOrSeparate,
        "TODO");
}

#if 0

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

#endif

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

    somera::Xcode::CompileOptions options;

    for (auto & path : parser.getValues("-I")) {
        std::cout << "[-I] " << path << std::endl;
        options.includeSearchPaths.push_back(path);
    }
    for (auto & path : parser.getValues("-L")) {
        std::cout << "[-L] " << path << std::endl;
        options.librarySearchPaths.push_back(path);
    }
    for (auto & path : parser.getValues("-l")) {
        std::cout << "[-l] " << path << std::endl;
        options.libraries.push_back(path);
    }
    for (auto & path : parser.getPaths()) {
        std::cout << "[Path] " << path << std::endl;
        options.sources.push_back(path);
    }

    options.outputPath = "MyHoge";
    for (auto & path : parser.getValues("-output=")) {
        options.outputPath = path;
        break;
    }
    for (auto & path : parser.getValues("-generator=")) {
        if (path == "xcode") {
            auto error = somera::Xcode::generateXcodeProject(options);
            if (error.hasError) {
                std::cerr << error.description << std::endl;
                return 1;
            }
            std::cout << "Generated." << std::endl;
        }
        break;
    }
    return 0;
}
