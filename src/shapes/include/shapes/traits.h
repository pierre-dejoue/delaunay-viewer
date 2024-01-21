// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <shapes/bounding_box.h>
#include <shapes/point.h>

namespace shapes {

template <typename F, int Dim>
struct Traits;

template <typename F>
struct Traits<F, 2>
{
    using Point = Point2d<F>;
    using BoundingBox = BoundingBox2d<F>;
};

template <typename F>
struct Traits<F, 3>
{
    using Point = Point3d<F>;
    using BoundingBox = BoundingBox3d<F>;
};

} // namespace shapes
