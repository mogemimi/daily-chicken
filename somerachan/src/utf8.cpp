// Copyright (c) 2015 mogemimi. Distributed under the MIT license.

#include "utf8.h"
#include <codecvt>
#include <locale>
#include <utility>

#if defined(_MSC_VER)
#include <clocale>
#include <cuchar>
#include <vector>
#include <cassert>
#endif

namespace somera {

std::u32string toUtf32(const std::string& s)
{
    if (s.empty()) {
        return {};
    }

    //static_assert(__STDC_UTF_16__ == 1, "");
    //static_assert(__STDC_UTF_32__ == 1, "");

#if defined(_MSC_VER)
    //mbstate_t state;
    //std::setlocale(LC_ALL, "en_US.UTF-8");
    ////std::mbsinit(&state);

    //char32_t c32 = 0;
    //auto pointer = s.data();
    //const auto end = pointer + s.size();
    //std::u32string result;

    //while (end - pointer > 0) {
    //    assert(pointer != end);
    //    assert(pointer < end);
    //    assert(end - pointer > 0);
    //    size_t rc = std::mbrtoc32(&c32, pointer, end - pointer, &state);
    //    if (rc == static_cast<size_t>(-2)) {
    //        // error
    //        break;
    //    }
    //    if (rc == static_cast<size_t>(-1)) {
    //        // error
    //        break;
    //    }
    //    if (rc == static_cast<size_t>(-3)) {
    //        // error
    //        break;
    //    }
    //    if (rc == 0) {
    //        // null character
    //        break;
    //    }
    //    assert(rc <= (end - pointer));
    //    result += c32;
    //    pointer += rc;
    //}
    //return std::move(result);
#else
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert;
    return convert.from_bytes(s);
#endif
}

std::string toUtf8(const std::u32string& s)
{
#if defined(_MSC_VER)
    //std::setlocale(LC_ALL, "en_US.UTF-8");
    //mbstate_t state;
    //std::mbsinit(&state);

    //std::vector<char> out(MB_CUR_MAX);
    //std::string result;

    //for (auto & c32 : s) {
    //    int rc = std::c32rtomb(out.data(), c32, &state);
    //    if (rc < 0) {
    //        // error
    //        break;
    //    }
    //    if (rc == 0) {
    //        // null character
    //        break;
    //    }
    //    assert(rc < out.size());
    //    result.append(out.data(), rc);
    //}
    //return std::move(result);
#else
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert;
    return convert.to_bytes(s);
#endif
}

} // namespace somera
