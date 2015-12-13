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

    Defer(Defer && other)
    {
        if (this->func) {
            this->func();
        }
        this->func = std::move(other.func);
    }

    Defer & operator=(Defer && other)
    {
        if (this->func) {
            this->func();
        }
        this->func = std::move(other.func);
        return *this;
    }

    void cancel()
    {
        std::function<void()> f;
        std::swap(func, f);
    }
};

} // namespace somera
