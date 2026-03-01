// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <algorithm>
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
    T def;
    T min;
    T max;

    const T& clamp(const T& v) const noexcept;
    T clamped_value_or_default(const std::optional<T>& opt_v) const noexcept;
    bool is_legit() const noexcept;
};

template <>
struct Limits<bool>
{
    bool def;

    bool clamped_value_or_default(const std::optional<bool>& opt_boolean) const noexcept;
};

inline constexpr Limits<bool> limits_true  { true  };
inline constexpr Limits<bool> limits_false { false };


//
//
// Implementation
//
//


template <typename T>
const T& Limits<T>::clamp(const T& v) const noexcept
{
    return std::clamp(v, min, max);
}

template <typename T>
T Limits<T>::clamped_value_or_default(const std::optional<T>& opt_v) const noexcept
{
    assert(min <= def && def <= max);
    return std::clamp(opt_v.value_or(def), min, max);
}

template <typename T>
bool Limits<T>::is_legit() const noexcept
{
    return min <= def && def <= max;
}

inline bool Limits<bool>::clamped_value_or_default(const std::optional<bool>& opt_boolean) const noexcept
{
    return opt_boolean.value_or(def);
}

template <typename U, typename T>
Limits<U> cast_limit_to(const Limits<T>& limit)
{
    return Limits<U>{ static_cast<U>(limit.def), static_cast<U>(limit.min), static_cast<U>(limit.max) };
}

} // namespace parameter
} // namespace stdutils
