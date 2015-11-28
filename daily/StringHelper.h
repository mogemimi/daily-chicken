// Copyright (c) 2015 mogemimi. Distributed under the MIT license.

#pragma once

#include <string>

namespace somera {
namespace StringHelper {

bool startWith(const std::string& source, const std::string& prefix);

std::string toLower(const std::string& source);

std::string replace(
    const std::string& source,
    const std::string& from,
    const std::string& to);

std::string format(char const* formatText, ...)
#if defined(__has_attribute)
#if __has_attribute(format)
__attribute__((__format__(printf, 1, 2)));
#endif
#elif __GNUC__ >= 4
__attribute__((__format__(printf, 1, 2)));
#else
;
#endif

} // namespace StringHelper
} // namespace somera
