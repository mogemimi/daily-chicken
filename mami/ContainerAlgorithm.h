// Copyright (c) 2016 mogemimi. Distributed under the MIT license.

#pragma once

#include <algorithm>
#include <iterator>

namespace somera {

template <class Container, class Func>
void EraseIf(Container & c, Func f)
{
    c.erase(std::remove_if(std::begin(c), std::end(c), f), std::end(c));
}

} // namespace somera
