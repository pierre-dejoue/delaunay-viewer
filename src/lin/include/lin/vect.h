// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <stdutils/span.h>

#include <array>
#include <cstddef>

namespace lin {

using index = std::size_t;

//
// Vectors
//

template <typename F, index N>
using vect = std::array<F, N>;

template <typename F>
using vect2 = vect<F, 2>;
template <typename F>
using vect3 = vect<F, 3>;
template <typename F>
using vect4 = vect<F, 4>;

using vect2f = vect2<float>;
using vect3f = vect3<float>;
using vect4f = vect4<float>;

using vect2d = vect2<double>;
using vect3d = vect3<double>;
using vect4d = vect4<double>;

//
// Vector maps
//
template <typename F, index N>
using vect_map = stdutils::span<F, N>;

template <typename F>
using vect_map2 = vect_map<F, 2>;
template <typename F>
using vect_map3 = vect_map<F, 3>;
template <typename F>
using vect_map4 = vect_map<F, 4>;

using vect_map2f = vect_map2<float>;
using vect_map3f = vect_map3<float>;
using vect_map4f = vect_map4<float>;

using vect_map2d = vect_map2<double>;
using vect_map3d = vect_map3<double>;
using vect_map4d = vect_map4<double>;

} // namespace lin
