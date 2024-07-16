// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <shapes/bounding_box.h>
#include <shapes/traits.h>

#include <cassert>
#include <cmath>
#include <limits>
#include <type_traits>

namespace shapes {

// A rough, fast to compute bounding box of any shape with vertices
template <typename S>
auto fast_bounding_box(const S& s);

// Utility functions to fix a degenerated bounding box
template <typename F>
void ensure_min_extent(stdutils::Range<F>& range);
template <typename F>
void ensure_min_extent(BoundingBox2d<F>& bb);
template <typename F>
void ensure_min_extent(BoundingBox3d<F>& bb);

// Scale a bounding box around its center
template <typename F>
void scale_around_center_in_place(BoundingBox2d<F>& bb, F scale);
template <typename F>
void scale_around_center_in_place(BoundingBox3d<F>& bb, F scale);
template <typename F>
BoundingBox2d<F> scale_around_center(const BoundingBox2d<F>& bb, F scale);
template <typename F>
BoundingBox3d<F> scale_around_center(const BoundingBox3d<F>& bb, F scale);

// Conservative rounding of a bounding box with floating-point type.
// Round the boundaries to the nearest integer value such that the output BB fully contains the input BB.
template <typename F, typename T = F>
BoundingBox2d<T> conservative_rounding(const BoundingBox2d<F>& bb_f);
template <typename F, typename T = F>
BoundingBox3d<T> conservative_rounding(const BoundingBox3d<F>& bb_f);


//
//
// Implementation
//
//


template <typename S>
auto fast_bounding_box(const S& s)
{
    typename Traits<typename S::scalar, S::dim>::BoundingBox bb;
    for (const auto& p: s.vertices)
    {
        bb.add(p);
    }
    return bb;
}

template <typename F>
void ensure_min_extent(Range<F>& range)
{
    static_assert(std::is_floating_point_v<F>);
    if (!range.is_populated())
    {
        range.min = F(0);
        range.max = std::numeric_limits<F>::epsilon();
    }
    else if (range.min == range.max)
    {
        const F v = range.min;
        if (std::abs(v) <= F(1))
            range.max = range.min + std::numeric_limits<F>::epsilon();
        else if (v > F(0))
            range.max = range.min * (F(1) + std::numeric_limits<F>::epsilon());
        else
            range.min = range.max * (F(1) + std::numeric_limits<F>::epsilon());
    }
    assert(std::fpclassify(range.length()) != FP_ZERO);
}

template <typename F>
void ensure_min_extent(BoundingBox2d<F>& bb)
{
    ensure_min_extent(bb.rx);
    ensure_min_extent(bb.ry);
}

template <typename F>
void ensure_min_extent(BoundingBox3d<F>& bb)
{
    ensure_min_extent(bb.rx);
    ensure_min_extent(bb.ry);
    ensure_min_extent(bb.rz);
}

template <typename F>
void scale_around_center_in_place(BoundingBox2d<F>& bb, F scale)
{
    scale_around_center_in_place(bb.rx, scale);
    scale_around_center_in_place(bb.ry, scale);
}

template <typename F>
void scale_around_center_in_place(BoundingBox3d<F>& bb, F scale)
{
    scale_around_center_in_place(bb.rx, scale);
    scale_around_center_in_place(bb.ry, scale);
    scale_around_center_in_place(bb.rz, scale);
}

template <typename F>
BoundingBox2d<F> scale_around_center(const BoundingBox2d<F>& bb, F scale)
{
    BoundingBox2d<F> result = bb;
    scale_around_center_in_place(result.rx, scale);
    scale_around_center_in_place(result.ry, scale);
    return result;
}

template <typename F>
BoundingBox3d<F> scale_around_center(const BoundingBox3d<F>& bb, F scale)
{
    BoundingBox3d<F> result = bb;
    scale_around_center_in_place(result.rx, scale);
    scale_around_center_in_place(result.ry, scale);
    scale_around_center_in_place(result.rz, scale);
    return result;
}

template <typename F, typename T>
BoundingBox2d<T> conservative_rounding(const BoundingBox2d<F>& bb_f)
{
    BoundingBox2d<T> bb_i;
    bb_i.rx = conservative_rounding<F, T>(bb_f.rx);
    bb_i.ry = conservative_rounding<F, T>(bb_f.ry);
    return bb_i;
}

template <typename F, typename T>
BoundingBox3d<T> conservative_rounding(const BoundingBox3d<F>& bb_f)
{
    BoundingBox3d<T> bb_i;
    bb_i.rx = conservative_rounding<F, T>(bb_f.rx);
    bb_i.ry = conservative_rounding<F, T>(bb_f.ry);
    bb_i.rz = conservative_rounding<F, T>(bb_f.rz);
    return bb_i;
}

} // namespace shapes
