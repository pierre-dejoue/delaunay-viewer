// Copyright (c) 2025 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#include <catch_amalgamated.hpp>

#include <stdutils/type_traits.h>

#include <array>
#include <cstddef>

TEST_CASE("array_size", "[type_traits]")
{
    constexpr std::size_t N = 3;
    using T = int;
    T build_in_arr[N];
    std::array<T, N> std_arr;
    static_assert(stdutils::array_size_v<decltype(build_in_arr)> == N);
    static_assert(stdutils::array_size_v<decltype(std_arr)>      == N);
    CHECK(sizeof(build_in_arr) == N * sizeof(T));
    CHECK(std_arr.size() == N);
}
