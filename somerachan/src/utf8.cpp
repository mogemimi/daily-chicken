// Copyright (c) 2015 mogemimi. Distributed under the MIT license.

#include "utf8.h"
#include <codecvt>
#include <locale>

namespace somera {

std::u32string toUtf32(const std::string& s)
{
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert;
    return convert.from_bytes(s);
}

std::string toUtf8(const std::u32string& s)
{
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert;
    return convert.to_bytes(s);
}

} // namespace somera
