// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>
#include <ostream>
#include <type_traits>

namespace stdutils {

template <typename T>
struct Range
{
    static_assert(std::is_arithmetic_v<T>);
    using scalar = T;
    Range() : min(std::numeric_limits<T>::max()), max(std::numeric_limits<T>::lowest()) {}
    Range(T min, T max) : min(min), max(max) { assert(min <= max); }
    bool is_populated() const { return min <= max; }
    Range<T>& add(T v);
    Range<T>& add_border(T v);
    Range<T>& merge(const Range<T>& o);
    T length() const { return min <= max ? max - min : T(0); }
    T extent() const { return length(); }
    bool operator==(const Range<T>& o) const { return min == o.min && max == o.max; }
    bool intersect(const Range<T>& o) const { return max >= o.min && o.max >= min; }
    T min;
    T max;
};

// Conversion
template <typename T0, typename T1>
Range<T1> cast(const Range<T0>& range);

// Scale a range around its center
template <typename F>
void scale_around_center_in_place(Range<F>& range, F scale);
template <typename F>
Range<F> scale_around_center(const Range<F>& range, F scale);

// Conservative rounding of a range with floating-point type.
// Round the boundaries to the nearest integer value such that the output range fully contains the input one.
template <typename F, typename T = F>
Range<T> conservative_rounding(const Range<F>& range);

// IO
template <typename T>
std::ostream& operator<<(std::ostream& out, const Range<T>& range);


//
//
// Implementation
//
//


template <typename T>
Range<T>& Range<T>::add(T v)
{
    min = std::min(min, v);
    max = std::max(max, v);
    return *this;
}

template <typename T>
Range<T>& Range<T>::add_border(T v)
{
    assert(v >= 0);
    min -= v;
    max += v;
    return *this;
}

template <typename T>
Range<T>& Range<T>::merge(const Range<T>& o)
{
    min = std::min(min, o.min);
    max = std::max(max, o.max);
    return *this;
}

template <typename T0, typename T1>
Range<T1> cast(const Range<T0>& range)
{
    if (range.min <= range.max)
        return Range<T1>(static_cast<T1>(range.min), static_cast<T1>(range.max));
    else
        return Range<T1>();
}

template <typename F>
void scale_around_center_in_place(Range<F>& range, F scale)
{
    static_assert(std::is_floating_point_v<F>);
    assert(range.is_populated());
    const F center = (range.min + range.max) / F{2};
    range.min = center + (range.min - center) * scale;
    range.max = center + (range.max - center) * scale;
}

template <typename F>
Range<F> scale_around_center(const Range<F>& range, F scale)
{
    Range<F> result = range;
    scale_around_center_in_place(result, scale);
    return result;
}

template <typename F, typename T>
Range<T> conservative_rounding(const Range<F>& range)
{
    static_assert(std::is_floating_point_v<F>);
    assert(range.is_populated());
    return Range<T>(
        static_cast<T>(std::floor(range.min)),
        static_cast<T>(std::ceil(range.max))
    );
}

template <typename T>
std::ostream& operator<<(std::ostream& out, const Range<T>& range)
{
    if (range.is_populated())
        out << "[ " << range.min << ", " << range.max << " ]" ;
    else
        out << "[ empty ]";
    return out;
}

} // namespace stdutils
