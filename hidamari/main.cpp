// Copyright (c) 2015 mogemimi. Distributed under the MIT license.

#include "CommandLineParser.h"
#include "FileSystem.h"
#include "XcodeProject.h"
#include "StringHelper.h"
#include "SubprocessHelper.h"
#include <iostream>
#include <vector>
#include <string>
#include <fstream>

using somera::CommandLineParser;
using somera::CommandLineArgumentType;
namespace FileSystem = somera::FileSystem;

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
        "The output formats to generate. Supported format\n"
        "are \"xcode\", \"msbuild\", \"cmake\", \"gyp\", or \"gn\".");
    parser.addArgument("-o", CommandLineArgumentType::JoinedOrSeparate,
        "Write output to <file>");
    parser.addArgument("-generator-output=", CommandLineArgumentType::JoinedOrSeparate,
        "Generate build files under the <dir>");
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

void sortByName(std::vector<std::string>& names)
{
    std::sort(std::begin(names), std::end(names),
        [](const auto& a, const auto& b){ return a < b; });
}

} // namespace somera

#include "XcodeProject.h"

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
        std::cout
            << "Usage: hidamari [options ...] [build file ...]" << std::endl
            << std::endl
            << "Options:" << std::endl
            << parser.getHelpText() << std::endl;
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

    for (auto & path : parser.getValues("-generator-output=")) {
        options.generatorOutputDirectory = path;
        break;
    }
    options.productName = "MyHoge";
    for (auto & path : parser.getValues("-o")) {
        options.productName = path;
        break;
    }
    options.targetName = options.productName;
    options.author = somera::SubprocessHelper::call("git config user.name");

    for (auto & path : options.includeSearchPaths) {
        path = FileSystem::relative(path, options.generatorOutputDirectory);
        std::cout << "[Path (Relative)] " << path << std::endl;
    }
    for (auto & path : options.librarySearchPaths) {
        path = FileSystem::relative(path, options.generatorOutputDirectory);
        std::cout << "[Path (Relative)] " << path << std::endl;
    }
    for (auto & path : options.libraries) {
        path = FileSystem::relative(path, options.generatorOutputDirectory);
        std::cout << "[Path (Relative)] " << path << std::endl;
    }
    for (auto & path : options.sources) {
        path = FileSystem::relative(path, options.generatorOutputDirectory);
        std::cout << "[Path (Relative)] " << path << std::endl;
    }
    somera::sortByName(options.libraries);
    somera::sortByName(options.sources);

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
