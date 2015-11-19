// Copyright (c) 2015 mogemimi. Distributed under the MIT license.

#include "FileSystem.h"
#include <cassert>

#ifdef _MSC_VER
#define SOMERA_IS_WINDOWS
#endif

#ifdef SOMERA_IS_WINDOWS

#else
#include <array>
#include <algorithm>
#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace somera {

std::string FileSystem::join(std::string const& path1, std::string const& path2)
{
    std::string result = path1;
#if defined(SOMERA_IS_WINDOWS)
    if (!result.empty() && '\\' == result.back()) {
        result.erase(std::prev(std::end(result)));
        result += '/';
    }
#endif
    if (!result.empty() && '/' != result.back()) {
        if (!path2.empty() && '/' != path2.front()) {
            result += '/';
        }
    }
    result += path2;
    return std::move(result);
}

std::string FileSystem::getBaseName(const std::string& path)
{
    const auto lastIndex = path.find_last_of('/');
    if (std::string::npos != lastIndex) {
        return path.substr(lastIndex + 1);
    }
    return path;
}

std::string FileSystem::getDirectoryName(const std::string& path)
{
    if (!path.empty() && path.back() == '/') {
        return path;
    }
    const auto lastIndex = path.find_last_of('/');
    if (std::string::npos != lastIndex) {
        return path.substr(0, lastIndex);
    }
    return {};
}

std::tuple<std::string, std::string>
FileSystem::split(const std::string& path)
{
    std::tuple<std::string, std::string> result;
    auto lastIndex = path.find_last_of('/');
    if (std::string::npos != lastIndex) {
        std::get<0>(result) = path.substr(0, lastIndex);
        std::get<1>(result) = path.substr(lastIndex + 1);
    } else {
        std::get<1>(result) = path;
    }
    return std::move(result);
}

std::tuple<std::string, std::string>
FileSystem::splitExtension(const std::string& path)
{
    std::tuple<std::string, std::string> result;
    auto lastIndex = path.find_last_of('.');
    if (std::string::npos != lastIndex) {
        std::get<0>(result) = path.substr(0, lastIndex);
        std::get<1>(result) = path.substr(lastIndex + 1);
    } else {
        std::get<0>(result) = path;
    }
    return std::move(result);
}

bool FileSystem::createDirectory(const std::string& path)
{
#ifdef SOMERA_IS_WINDOWS
#else
    assert(!path.empty());
    struct stat st;
    if (::stat(path.c_str(), &st) != -1) {
        return false;
    }
    return ::mkdir(path.c_str(), S_IRWXU) == 0;
#endif
}

bool FileSystem::createDirectories(const std::string& path)
{
#ifdef SOMERA_IS_WINDOWS

#else
    assert(!path.empty());
    if (path.empty()) {
        return false;
    }

    auto tmp = path;
    if (tmp.back() == '/') {
        tmp.pop_back();
    }
    assert(!tmp.empty());
    if (tmp.empty()) {
        return false;
    }

    auto iter = std::next(std::begin(tmp), 1);
    for (; iter != std::end(tmp); iter++) {
        if (*iter == '/') {
            *iter = 0;
            ::mkdir(tmp.c_str(), S_IRWXU);
            *iter = '/';
        }
    }
    return ::mkdir(tmp.c_str(), S_IRWXU) == 0;
#endif
}

bool FileSystem::exists(const std::string& path)
{
#ifdef SOMERA_IS_WINDOWS

#else
    assert(!path.empty());
    return ::access(path.c_str(), F_OK) != -1;
#endif
}

bool FileSystem::isDirectory(const std::string& path)
{
#ifdef SOMERA_IS_WINDOWS

#else
    assert(!path.empty());
    struct stat st;
    if (::stat(path.c_str(), &st) != -1) {
        return false;
    }
    return S_ISDIR(st.st_mode);
#endif
}

} // namespace somera
