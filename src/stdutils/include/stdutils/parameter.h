// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <stdutils/minmax.h>

#include <cassert>
#include <optional>

/**
 * Parameter limits
 */
namespace stdutils {
namespace parameter {

template <typename T>
struct Limits
{
    T min;
    T def;
    T max;

    constexpr Limits() : min{}, def{}, max{} { }
    constexpr Limits(T min, T def, T max) : min(min), def(def), max(max) { assert(min <= def); assert(def <= max); }

    constexpr const T& clamp(const T& v) const noexcept;
    constexpr T clamped_value_or_default(const std::optional<T>& opt_v) const noexcept;
    constexpr bool is_legit() const noexcept;
};

template <>
struct Limits<bool>
{
    bool def;

    constexpr Limits() : def(false) {}
    constexpr Limits(bool def) : def(def) {}

    constexpr bool clamped_value_or_default(const std::optional<bool>& opt_boolean) const noexcept;
};

inline constexpr Limits<bool> limits_true  { true  };
inline constexpr Limits<bool> limits_false { false };

// Casting
template <typename U, typename T>
constexpr Limits<U> cast_limit_to(const Limits<T>& limit);


//
//
// Implementation
//
//


template <typename T>
constexpr const T& Limits<T>::clamp(const T& v) const noexcept
{
    return stdutils::clamp<T>(v, min, max);
}

template <typename T>
constexpr T Limits<T>::clamped_value_or_default(const std::optional<T>& opt_v) const noexcept
{
    assert(min <= def && def <= max);
    return stdutils::clamp<T>(opt_v.value_or(def), min, max);
}

template <typename T>
constexpr bool Limits<T>::is_legit() const noexcept
{
    return min <= def && def <= max;
}

constexpr inline bool Limits<bool>::clamped_value_or_default(const std::optional<bool>& opt_boolean) const noexcept
{
    return opt_boolean.value_or(def);
}

template <typename U, typename T>
constexpr Limits<U> cast_limit_to(const Limits<T>& limit)
{
    return Limits<U> { static_cast<U>(limit.min), static_cast<U>(limit.def), static_cast<U>(limit.max) };
}

} // namespace parameter
} // namespace stdutils
