// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <shapes/comparison.h>

#include <algorithm>
#include <cmath>
#include <ostream>

namespace shapes {

template <typename F>
struct Vect2d
{
    static constexpr int dim = 2;
    using scalar = F;
    Vect2d() : x(0), y(0) {}
    Vect2d(F x, F y) : x(x), y(y) {}
    bool operator==(const Vect2d<F>& other) const { return x == other.x && y == other.y; }
    F x;
    F y;
};

template <typename F>
struct Vect3d
{
    static constexpr int dim = 3;
    using scalar = F;
    Vect3d() : x(0), y(0), z(0) {}
    Vect3d(F x, F y, F z) : x(x), y(y), z(z) {}
    bool operator==(const Vect3d<F>& other) const { return x == other.x && y == other.y && z == other.z; }
    F x;
    F y;
    F z;
};

//
// Sanity checks
//
template <typename F>
inline bool isfinite(const Vect2d<F>& v)
{
    return std::isfinite(v.x) && std::isfinite(v.y);
}

template <typename F>
inline bool isfinite(const Vect3d<F>& v)
{
    return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);
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
Vect2d<F> operator+(const Vect2d<F>& a, const Vect2d<F>& b)
{
    return Vect2d<F>(a.x + b.x, a.y + b.y);
}

template <typename F>
Vect2d<F> operator-(const Vect2d<F>& a, const Vect2d<F>& b)
{
    return Vect2d<F>(a.x - b.x, a.y - b.y);
}

template <typename F>
Vect2d<F> operator*(F s, const Vect2d<F>& a)
{
    return Vect2d<F>(s * a.x, s * a.y);
}

template <typename F>
Vect3d<F> operator+(const Vect3d<F>& a, const Vect3d<F>& b)
{
    return Vect3d<F>(a.x + b.x, a.y + b.y, a.z + b.z);
}

template <typename F>
Vect3d<F> operator-(const Vect3d<F>& a, const Vect3d<F>& b)
{
    return Vect3d<F>(a.x - b.x, a.y - b.y, a.z - b.z);
}

template <typename F>
Vect3d<F> operator*(F s, const Vect3d<F>& a)
{
    return Vect3d<F>(s * a.x, s * a.y, s * a.z);
}

//
// Norms
//
template <typename F>
F norm(const Vect2d<F>& v)
{
    return std::sqrt(v.x * v.x + v.y * v.y);
}

template <typename F>
F inf_norm(const Vect2d<F>& v)
{
    return std::max(std::abs(v.x), std::abs(v.y));
}

template <typename F>
F norm(const Vect3d<F>& v)
{
    return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

template <typename F>
F inf_norm(const Vect3d<F>& v)
{
    return std::max(std::abs(v.x), std::max(std::abs(v.y), std::abs(v.z)));
}


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
