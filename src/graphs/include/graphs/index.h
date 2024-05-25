// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <limits>
#include <type_traits>

namespace graphs {

// The following traits define what a proper vertex index is and its properties
//
// An index must be an unsigned integral type.
// The max value of that integral type is reserved and should not be used as a vertex index.
template <typename I>
struct IndexTraits
{
    static constexpr bool is_valid()        noexcept { return std::is_integral_v<I> && std::is_unsigned_v<I>; }
    static constexpr I    undef()           noexcept { return std::numeric_limits<I>::max(); }
    static constexpr I    max_valid_index() noexcept { return std::numeric_limits<I>::max() - I{1}; }
};

template <typename I, std::enable_if_t<IndexTraits<I>::is_valid(), bool> = true>
constexpr bool is_defined(I index) noexcept
{
    return index != IndexTraits<I>::undef();
}

} // namespace graphs
