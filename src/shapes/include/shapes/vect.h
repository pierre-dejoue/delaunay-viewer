// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <shapes/comparison.h>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <ostream>

namespace shapes {

template <typename F>
struct Vect2d
{
    static constexpr int dim = 2;
    using scalar = F;
    constexpr Vect2d() : x(0), y(0) {}
    constexpr Vect2d(F x, F y) : x(x), y(y) {}
    bool operator==(const Vect2d<F>& other) const { return x == other.x && y == other.y; }
    F x;
    F y;
};

template <typename F>
struct Vect3d
{
    static constexpr int dim = 3;
    using scalar = F;
    constexpr Vect3d() : x(0), y(0), z(0) {}
    constexpr Vect3d(F x, F y, F z) : x(x), y(y), z(z) {}
    bool operator==(const Vect3d<F>& other) const { return x == other.x && y == other.y && z == other.z; }
    F x;
    F y;
    F z;
};

//
// Unit vectors
//
namespace unit_vect2d {

    template <typename F>
    constexpr Vect2d<F> u() { return Vect2d<F>(1, 0); }
    template <typename F>
    constexpr Vect2d<F> v() { return Vect2d<F>(0, 1); }

}
namespace unit_vect3d {

    template <typename F>
    constexpr Vect3d<F> u() { return Vect3d<F>(1, 0, 0); }
    template <typename F>
    constexpr Vect3d<F> v() { return Vect3d<F>(0, 1, 0); }
    template <typename F>
    constexpr Vect3d<F> w() { return Vect3d<F>(0, 0, 1); }

}

//
// Sanity checks
//
template <typename F>
constexpr inline bool isfinite(const Vect2d<F>& v)
{
    return std::isfinite(v.x) && std::isfinite(v.y);
}

template <typename F>
constexpr inline bool isfinite(const Vect3d<F>& v)
{
    return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);
}

//
// Null vector
//
template <typename F>
constexpr inline bool isnull(const Vect2d<F>& v)
{
    assert(isfinite(v));
    return (std::fpclassify(v.x) == FP_ZERO) && (std::fpclassify(v.y) == FP_ZERO);
}

template <typename F>
constexpr inline bool isnull(const Vect3d<F>& v)
{
    assert(isfinite(v));
    return (std::fpclassify(v.x) == FP_ZERO) && (std::fpclassify(v.y) == FP_ZERO) && (std::fpclassify(v.z) == FP_ZERO);
}

//
// IO
//
template <typename F>
std::ostream& operator<<(std::ostream& out, const Vect2d<F>& v);

template <typename F>
std::ostream& operator<<(std::ostream& out, const Vect3d<F>& v);

//
// Standard operations
//
template <typename F>
constexpr Vect2d<F> operator+(const Vect2d<F>& a, const Vect2d<F>& b)
{
    return Vect2d<F>(a.x + b.x, a.y + b.y);
}

template <typename F>
constexpr Vect2d<F> operator-(const Vect2d<F>& a, const Vect2d<F>& b)
{
    return Vect2d<F>(a.x - b.x, a.y - b.y);
}

template <typename F>
constexpr Vect2d<F> operator-(const Vect2d<F>& a)
{
    return Vect2d<F>(-a.x, -a.y);
}

template <typename F>
constexpr Vect2d<F> operator*(F s, const Vect2d<F>& a)
{
    return Vect2d<F>(s * a.x, s * a.y);
}

template <typename F>
constexpr Vect3d<F> operator+(const Vect3d<F>& a, const Vect3d<F>& b)
{
    return Vect3d<F>(a.x + b.x, a.y + b.y, a.z + b.z);
}

template <typename F>
constexpr Vect3d<F> operator-(const Vect3d<F>& a, const Vect3d<F>& b)
{
    return Vect3d<F>(a.x - b.x, a.y - b.y, a.z - b.z);
}

template <typename F>
constexpr Vect3d<F> operator-(const Vect3d<F>& a)
{
    return Vect3d<F>(-a.x, -a.y, -a.z);
}

template <typename F>
constexpr Vect3d<F> operator*(F s, const Vect3d<F>& a)
{
    return Vect3d<F>(s * a.x, s * a.y, s * a.z);
}

//
// Products
//
template <typename F>
constexpr F dot(const Vect2d<F>& a, const Vect2d<F>& b)
{
    return a.x * b.x + a.y * b.y;
}

template <typename F>
constexpr F dot(const Vect3d<F>& a, const Vect3d<F>& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

template <typename F>
constexpr F cross_product(const Vect2d<F>& a, const Vect2d<F>& b)
{
    // This function will return the signed norm of the cross product of the two vectors assuming their embedding in a 3D space.
    return a.x * b.y - a.y * b.x;
}

template <typename F>
constexpr Vect3d<F> cross_product(const Vect3d<F>& a, const Vect3d<F>& b)
{
    return Vect3d<F>(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

//
// Norms
//
template <typename F>
constexpr F norm(const Vect2d<F>& v)
{
    return std::sqrt(v.x * v.x + v.y * v.y);
}

template <typename F>
constexpr F sq_norm(const Vect2d<F>& v)
{
    return v.x * v.x + v.y * v.y;
}

template <typename F>
constexpr F inf_norm(const Vect2d<F>& v)
{
    return std::max(std::abs(v.x), std::abs(v.y));
}

template <typename F>
constexpr F norm(const Vect3d<F>& v)
{
    return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

template <typename F>
constexpr F sq_norm(const Vect3d<F>& v)
{
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

template <typename F>
constexpr F inf_norm(const Vect3d<F>& v)
{
    return std::max(std::abs(v.x), std::max(std::abs(v.y), std::abs(v.z)));
}

//
// Function objects
//
template <typename F, typename V>
struct dot_product
{
    constexpr F operator()(const V& lhs, const V& rhs) const { return dot(lhs, rhs); }
};

//
// Comparisons
//
template <typename F>
struct less<Vect2d<F>>
{
    constexpr bool operator()(const Vect2d<F>& lhs, const Vect2d<F>& rhs) const
    {
        return lhs.x < rhs.x || (lhs.x == rhs.x && lhs.y < rhs.y);
    }
};

template <typename F>
struct less<Vect3d<F>>
{
    constexpr bool operator()(const Vect3d<F>& lhs, const Vect3d<F>& rhs) const
    {
        return lhs.x < rhs.x || (lhs.x == rhs.x && lhs.y < rhs.y) || (lhs.x == rhs.x && lhs.y == rhs.y && lhs.z < rhs.z);
    }
};

} // namespace shapes
