// Copyright (c) 2015 mogemimi. Distributed under the MIT license.

#include "StringHelper.h"
#include <algorithm>
#include <cassert>
#include <cstdarg>
#include <utility>

namespace somera {
namespace {

std::string UnsafeToFormatString(char const* format, std::va_list arg)
{
    std::va_list copiedArguments;
    va_copy(copiedArguments, arg);
#ifdef _MSC_VER
    char buffer[2048];
    std::memset(buffer, 0, sizeof(buffer));
    auto const length = vsnprintf_s(buffer, _countof(buffer), format, copiedArguments);
    static_assert(std::is_signed<decltype(length)>::value, "");
    va_end(copiedArguments);
    assert(length > 0);
    std::string result(buffer, length);
    assert(result.size() == static_cast<std::size_t>(length));
#else
#if __cplusplus >= 201103L
    using std::vsnprintf;
#endif
    auto const length = vsnprintf(nullptr, 0, format, copiedArguments);
    static_assert(std::is_signed<decltype(length)>::value, "");
    va_end(copiedArguments);
    assert(length > 0);
    std::string result(length + 1, '\0');
    vsnprintf(&result.front(), result.size(), format, arg);
    assert(result.back() == '\0');
    result.resize(length);
#endif
    return std::move(result);
}

} // unnamed namespace

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

std::string StringHelper::format(char const* formatText, ...)
{
    std::va_list arg;
    va_start(arg, formatText);
    auto result = UnsafeToFormatString(formatText, arg);
    va_end(arg);
    return std::move(result);
}

} // namespace somera
