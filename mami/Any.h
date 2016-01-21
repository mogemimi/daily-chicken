// Copyright (c) 2016 mogemimi. Distributed under the MIT license.

#pragma once

#include <cassert>
#include <memory>
#include <typeindex>
#include <typeinfo>
#include <type_traits>
#include <utility>
#ifdef _HAS_EXCEPTIONS
#include <exception>
#endif

namespace somera {

class Any final {
private:
    struct HolderBase {
        virtual ~HolderBase() = default;
    };

    template <typename T>
    struct Holder final : public HolderBase {
        T Value;

        template <typename U>
        explicit Holder(U && valueIn)
            : Value(std::forward<U>(valueIn))
        {}

        static_assert(std::is_object<T>::value, "");
    };

    std::unique_ptr<HolderBase> data;
    std::type_index typeIndex;

public:
    Any() = delete;
    Any(Any const&) = delete;
    Any(Any &&) = default;

    Any& operator=(Any const&) = delete;
    Any& operator=(Any &&) = default;

    template <typename T>
    Any(T && value)
        : data(std::make_unique<Holder<std::remove_reference_t<T>>>(std::forward<T>(value)))
        , typeIndex(typeid(T))
    {}

    template <typename T>
    bool Is() const noexcept
    {
        return typeIndex == typeid(T);
    }

    template <typename T>
    T const& As() const
    {
        assert(Is<T>());
        if (!Is<T>()) {
#ifdef _HAS_EXCEPTIONS
            using BadAnyCast = std::runtime_error;
            throw BadAnyCast;
#endif
        }
        assert(data);
        auto derived = dynamic_cast<Holder<T>*>(data.get());
        assert(derived);
        return derived->Value;
    }

    template <typename T>
    T & As()
    {
        assert(Is<T>());
        if (!Is<T>()) {
#ifdef _HAS_EXCEPTIONS
            using BadAnyCast = std::runtime_error;
            throw BadAnyCast;
#endif
        }
        assert(data);
        auto derived = dynamic_cast<Holder<T>*>(data.get());
        assert(derived);
        return derived->Value;
    }

    std::type_index Type() const noexcept
    {
        return typeIndex;
    }
};

} // namespace somera
