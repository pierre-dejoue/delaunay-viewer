// Copyright (c) 2025 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

namespace stdutils {

/**
 * Constexpr enum to determine the endianness of the platform at compile time.
 *
 * TODO: Use std::endian instead in C++20
 *
 * Reference: https://en.cppreference.com/w/cpp/types/endian.html
 */
enum class endianness
{
#if defined(_MSC_VER) && !defined(__clang__)
    little = 0,
    big    = 1,
    native = little
#else
    little = __ORDER_LITTLE_ENDIAN__,
    big    = __ORDER_BIG_ENDIAN__,
    native = __BYTE_ORDER__
#endif
};

} // namespace stdutils
