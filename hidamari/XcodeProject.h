// Copyright (c) 2015 mogemimi. Distributed under the MIT license.

#pragma once

#include <string>
#include <vector>

namespace somera {

struct GeneratorError {
    GeneratorError()
        : hasError(false)
    {
    }

    explicit GeneratorError(const std::string& desc)
        : description(desc)
        , hasError(false)
    {}

    std::string description;
    bool hasError = false;
};

namespace Xcode {

struct CompileOptions {
    std::string generatorOutputDirectory;
    std::string productName;
    std::string targetName;
    std::vector<std::string> sources;
    std::vector<std::string> libraries;
    std::vector<std::string> includeSearchPaths;
    std::vector<std::string> librarySearchPaths;
    std::vector<std::string> flags;
    std::string author;
};

GeneratorError generateXcodeProject(const CompileOptions& options);

} // namespace Xcode
} // namespace somera
