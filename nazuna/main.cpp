// Copyright (c) 2015 mogemimi. Distributed under the MIT license.

#include "Defer.h"
#include <iostream>

int main()
{
    std::printf("%s", "hello");
    somera::Defer defer([] {
        std::printf("%s", "world\n");
    });
    std::printf("%s", ", ");
    return 0;
}
