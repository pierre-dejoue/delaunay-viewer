// Copyright (c) 2024 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <cstdlib>
#include <type_traits>

namespace stdutils {

/**
 * Assuming an enumeration declaration follows the convention of being a range from 0 to the special value _ENUM_SIZE_,
 * then the following functions can be used to retrieve the range of the valid enumeration values
 *
 * e.g.
 *
 *    enum class Enumeration
 *    {
 *        A = 0,
 *        B,
 *        C,
 *        _ENUM_SIZE_
 *    };
 *
 *    enum_first_value<Enumeration>() returns Enumeration::A;
 *    enum_last_value<Enumeration>()  returns Enumeration::C;
 *
 *    and
 *
 *    enum_size<Enumeration>() returns 3;
 */
template <typename E>
constexpr E enum_first_value() noexcept
{
    static_assert(std::is_enum_v<E>);
    return static_cast<E>(0);
}

template <typename E>
constexpr E enum_last_value() noexcept
{
    static_assert(std::is_enum_v<E>);
    using T = std::underlying_type_t<E>;
    static_assert(static_cast<T>(E::_ENUM_SIZE_) > 0);
    return static_cast<E>(static_cast<T>(E::_ENUM_SIZE_) - T{1});
}

template <typename E>
constexpr std::size_t enum_size() noexcept
{
    static_assert(std::is_enum_v<E>);
    return static_cast<std::size_t>(E::_ENUM_SIZE_);
}

} // namespace stdutils
