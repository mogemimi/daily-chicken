// Copyright (c) 2015 mogemimi. Distributed under the MIT license.

#include "StringHelper.h"
#include <algorithm>
#include <utility>

namespace somera {

bool StringHelper::startWith(const std::string& text, const std::string& prefix)
{
    return (text.size() >= prefix.size())
        && (text.compare(0, prefix.size(), prefix) == 0);
}

std::string StringHelper::toLower(const std::string& source)
{
    std::string output = source;
    std::transform(output.begin(), output.end(), output.begin(), ::towlower);
    return std::move(output);
}

std::string StringHelper::replace(
    const std::string& source,
    const std::string& from,
    const std::string& to)
{
    if (from.empty()) {
        return source;
    }
    auto result = source;
    std::string::size_type start = 0;
    while ((start = result.find(from, start)) != std::string::npos) {
        result.replace(start, from.length(), to);
        start += to.length();
    }
    return std::move(result);
}

} // namespace somera
