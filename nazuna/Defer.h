// Copyright (c) 2015 mogemimi. Distributed under the MIT license.

#pragma once

#include <functional>
#include <utility>

namespace somera {

class Defer final {
    std::function<void()> func;
public:
    explicit Defer(const std::function<void()>& funcIn)
        : func(funcIn)
    {}

    ~Defer()
    {
        if (this->func) {
            this->func();
        }
    }

    Defer(const Defer&) = delete;
    Defer & operator=(const Defer&) = delete;

    Defer(Defer && d)
    {
        if (this->func) {
            this->func();
        }
        this->func = std::move(d.func);
    }

    Defer & operator=(Defer && d)
    {
        this->func();
        this->func = std::move(d.func);
        return *this;
    }
};

} // namespace somera
