// Copyright (c) 2015 mogemimi. Distributed under the MIT license.

#pragma once

#include <string>
#include <tuple>

namespace somera {
namespace FileSystem {

std::string join(std::string const& path1, std::string const& path2);

std::string getBaseName(const std::string& path);

std::string getDirectoryName(const std::string& path);

std::tuple<std::string, std::string> split(const std::string& path);

std::tuple<std::string, std::string> splitExtension(const std::string& path);

bool createDirectory(const std::string& path);

bool createDirectories(const std::string& path);

bool exists(const std::string& path);

bool isDirectory(const std::string& path);

} // namespace FileSystem
} // namespace somera
