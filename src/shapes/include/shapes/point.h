// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <shapes/vect.h>

#include <vector>

namespace shapes {

template<typename F>
using Point2d = Vect2d<F>;

template<typename F>
using Point3d = Vect3d<F>;

template<typename F>
using Points2d = std::vector<Point2d<F>>;

template<typename F>
using Points3d = std::vector<Point3d<F>>;

} // namespace shapes
