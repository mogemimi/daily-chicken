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

} // namespace StringHelper

} // namespace somera
