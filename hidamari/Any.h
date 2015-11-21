// Copyright (c) 2015 mogemimi. Distributed under the MIT license.

#pragma once

#include <cassert>
#include <memory>
#include <utility>
#include <typeindex>
#include <typeinfo>
#include <type_traits>

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
        : data(std::make_unique<Holder<typename std::remove_reference<T>::type>
            >(std::forward<T>(value)))
        , typeIndex(typeid(T))
    {}

    template <typename T>
    bool is() const
    {
        return typeIndex == typeid(T);
    }

    template <typename T>
    T const& as() const
    {
        assert(typeIndex == typeid(T));
        if (!is<T>()) {
            //throw BadAnyCast;
        }
        assert(data);
        auto derived = dynamic_cast<Holder<T>*>(data.get());
        assert(derived);
        return derived->Value;
    }

    template <typename T>
    T & as()
    {
        assert(typeIndex == typeid(T));
        if (!is<T>()) {
            //throw BadAnyCast;
        }
        assert(data);
        auto derived = dynamic_cast<Holder<T>*>(data.get());
        assert(derived);
        return derived->Value;
    }

    std::type_index type() const
    {
        return typeIndex;
    }
};

} // namespace somera
