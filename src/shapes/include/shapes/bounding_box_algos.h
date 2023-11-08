// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <shapes/bounding_box.h>
#include <shapes/traits.h>

#include <cassert>
#include <limits>


namespace shapes
{

// A rough, fast to compute bounding box of any shape with vertices
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


// Utility functions to fix a degenerated bounding box
template <typename F>
void ensure_min_extent(Range<F>& range)
{
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
    assert(range.extent() != F(0));
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

// Scale a range / bounding box around its center
template <typename F>
void scale_around_center_in_place(Range<F>& range, F zoom)
{
    assert(range.is_populated());
    const F center = (range.min + range.max) / F{2};
    range.min = center + (range.min - center) / zoom;
    range.max = center + (range.max - center) / zoom;
}

template <typename F>
Range<F> scale_around_center(const Range<F>& range, F zoom)
{
    Range<F> result = range;
    scale_around_center_in_place(result, zoom);
    return result;
}

template <typename F>
BoundingBox2d<F> scale_around_center(const BoundingBox2d<F>& bb, F zoom)
{
    BoundingBox2d<F> result = bb;
    scale_around_center_in_place(result.rx, zoom);
    scale_around_center_in_place(result.ry, zoom);
    return result;
}

template <typename F>
BoundingBox3d<F> scale_around_center(const BoundingBox3d<F>& bb, F zoom)
{
    BoundingBox3d<F> result = bb;
    scale_around_center_in_place(result.rx, zoom);
    scale_around_center_in_place(result.ry, zoom);
    scale_around_center_in_place(result.rz, zoom);
    return result;
}

} // namespace shapes