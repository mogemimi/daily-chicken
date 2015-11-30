// Copyright (c) 2015 mogemimi. Distributed under the MIT license.

#include "MSBuild.h"
#include "XcodeProject.h"
#include "daily/CommandLineParser.h"
#include "daily/FileSystem.h"
#include "daily/StringHelper.h"
#include "daily/SubprocessHelper.h"
#include <iostream>
#include <vector>
#include <string>
#include <fstream>

using somera::CommandLineParser;
namespace FileSystem = somera::FileSystem;

namespace somera {

void setupCommandLineParser(CommandLineParser & parser)
{
    using somera::CommandLineArgumentType::Flag;
    using somera::CommandLineArgumentType::JoinedOrSeparate;
    parser.setUsageText("hidamari [options ...] [build file ...]");
    parser.addArgument("-h", Flag, "Display available options");
    parser.addArgument("-help", Flag, "Display available options");
    parser.addArgument("-I", JoinedOrSeparate,
        "Add directory to include search path");
    parser.addArgument("-L", JoinedOrSeparate,
        "Add directory to library search path");
    parser.addArgument("-l", JoinedOrSeparate,
        "Search the library when linking");
    parser.addArgument("-fno-exceptions", Flag, "no-exceptions");
    parser.addArgument("-generator=", JoinedOrSeparate,
        "The output formats to generate. Supported format\n"
        "are \"xcode\", \"msbuild\", \"cmake\", \"gyp\", or \"gn\".");
    parser.addArgument("-o", JoinedOrSeparate, "Write output to <file>");
    parser.addArgument("-generator-output=", JoinedOrSeparate,
        "Generate build files under the <dir>");
    parser.addArgument("-verbose", Flag,
        "Provide additional status output");
    parser.addArgument("-std=", JoinedOrSeparate,
        "Language standard to compile for");
    parser.addArgument("-stdlib=", JoinedOrSeparate,
        "C++ standard library to use");
}

#if 0

std::string fileToString(const std::string& source)
{
    std::ifstream input(source);
    std::string line;
    std::stringstream stream;
    while (input && std::getline(input, line)) {
        line = somera::StringHelper::replace(line, "\\", "\\\\");
        line = somera::StringHelper::replace(line, "\"", "\\\"");
        line = somera::StringHelper::replace(line, "\r", "\\r");
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
        [](const auto& a, const auto& b) {
            return StringHelper::toLower(a) < StringHelper::toLower(b);
        });
    names.erase(
        std::unique(std::begin(names), std::end(names)), std::end(names));
}

std::string getAuthorName()
{
    return somera::StringHelper::replace(
        somera::SubprocessHelper::call("git config user.name"), "\n", "");
}

template <class Container, typename Func>
auto eraseIf(Container & container, Func func)
{
    return container.erase(
        std::remove_if(std::begin(container), std::end(container), func),
        std::end(container));
}

} // namespace somera

int main(int argc, char *argv[])
{
    CommandLineParser parser;
    setupCommandLineParser(parser);
    parser.parse(argc, argv);

    auto printVerbose = [&](const std::string& text) {
        static const bool verbose = parser.exists("-verbose");
        if (verbose) {
            std::cout << text << std::endl;
        }
    };

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

    somera::CompileOptions options;

    for (auto & path : parser.getValues("-I")) {
        printVerbose("[-I] "+ path);
        options.includeSearchPaths.push_back(path);
    }
    for (auto & path : parser.getValues("-L")) {
        printVerbose("[-L] " + path);
        options.librarySearchPaths.push_back(path);
    }
    for (auto & path : parser.getValues("-l")) {
        printVerbose("[-l] " + path);
        options.libraries.push_back(path);
    }
    for (auto & path : parser.getPaths()) {
        printVerbose("[Path] " + path);
        options.sources.push_back(path);
    }

    somera::eraseIf(options.sources, [](const std::string& path) {
        return somera::StringHelper::startWith(path, "*");
    });

    if (auto path = parser.getValue("-generator-output=")) {
        options.generatorOutputDirectory = *path;
    }
    options.productName = "MyHoge";
    if (auto path = parser.getValue("-o")) {
        options.productName = *path;
    }
    options.targetName = options.productName;
    options.author = somera::getAuthorName();

    if (auto value = parser.getValue("-std=")) {
        options.buildSettings.emplace("-std=", *value);
    }
    if (auto value = parser.getValue("-stdlib=")) {
        options.buildSettings.emplace("-stdlib=", *value);
    }

    for (auto & path : options.includeSearchPaths) {
        path = FileSystem::relative(path, options.generatorOutputDirectory);
        printVerbose("[-I (Relative)] " + path);
    }
    for (auto & path : options.librarySearchPaths) {
        path = FileSystem::relative(path, options.generatorOutputDirectory);
        printVerbose("[-L (Relative)] " + path);
    }
    for (auto & path : options.libraries) {
        path = FileSystem::relative(path, options.generatorOutputDirectory);
        printVerbose("[-l (Relative)] " + path);
    }
    for (auto & path : options.sources) {
        path = FileSystem::relative(path, options.generatorOutputDirectory);
        printVerbose("[Path (Relative)] " + path);
    }
    somera::sortByName(options.libraries);
    somera::sortByName(options.sources);

    if (auto generator = parser.getValue("-generator=")) {
        if (*generator == "xcode") {
            auto error = somera::Xcode::generateXcodeProject(options);
            if (error.hasError) {
                std::cerr << error.description << std::endl;
                return 1;
            }
            std::cout << "Generated." << std::endl;
        }
        else if (*generator == "msbuild") {
            auto error = somera::MSBuild::generateMSBuildProject(options);
            if (error.hasError) {
                std::cerr << error.description << std::endl;
                return 1;
            }
            std::cout << "Generated." << std::endl;
        }
    }

    return 0;
}
