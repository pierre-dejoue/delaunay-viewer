// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <shapes/point.h>
#include <stdutils/range.h>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>
#include <ostream>
#include <type_traits>

namespace shapes {

template <typename T>
using Range = stdutils::Range<T>;

template <typename T>
struct BoundingBox2d
{
    using scalar = T;
    bool is_populated() const { return rx.is_populated() && ry.is_populated(); }
    BoundingBox2d<T>& add(const Point2d<T>& p);
    BoundingBox2d<T>& add(T x, T y);
    BoundingBox2d<T>& add_border(const Vect2d<T>& v);
    BoundingBox2d<T>& add_border(T x, T y);
    BoundingBox2d<T>& add_border(T v);
    BoundingBox2d<T>& merge(const BoundingBox2d& o);
    Point2d<T> min() const;
    Point2d<T> max() const;
    Vect2d<T> extent() const { return { rx.length(), ry.length() }; }
    T width() const { return rx.length(); }
    T height() const { return ry.length(); }
    T diameter() const { const T w = width(); const T h = height(); return std::sqrt(w * w + h * h); }
    bool operator==(const BoundingBox2d<T>& o) const { return rx == o.rx && ry == o.ry; }
    bool intersect(const BoundingBox2d<T>& o) const { return rx.intersect(o.rx) && ry.intersect(o.ry); }
    Range<T> rx;
    Range<T> ry;
};

template <typename T>
struct BoundingBox3d
{
    using scalar = T;
    bool is_populated() const { return rx.is_populated() && ry.is_populated() && rz.is_populated(); }
    BoundingBox3d<T>& add(const Point3d<T>& p);
    BoundingBox3d<T>& add(T x, T y, T z);
    BoundingBox3d<T>& add_border(const Vect3d<T>& v);
    BoundingBox3d<T>& add_border(T x, T y, T z);
    BoundingBox3d<T>& add_border(T v);
    BoundingBox3d<T>& merge(const BoundingBox3d& o);
    Point3d<T> min() const;
    Point3d<T> max() const;
    Vect3d<T> extent() const { return { rx.length(), ry.length(), rz.length() }; }
    T width() const { return rx.length(); }
    T height() const { return ry.length(); }
    T depth() const { return rz.length(); }
    T diameter() const { const T w = width(); const T h = height(); const T d = depth(); return std::sqrt(w * w + h * h + d * d); }
    bool operator==(const BoundingBox2d<T>& o) const { return rx == o.rx && ry == o.ry && rz == o.rz; }
    bool intersect(const BoundingBox2d<T>& o) const { return rx.intersect(o.rx) && ry.intersect(o.ry) && rz.intersect(o.rz); }
    Range<T> rx;
    Range<T> ry;
    Range<T> rz;
};

// Conversions
template <typename T0, typename T1>
BoundingBox2d<T1> cast(const BoundingBox2d<T0>& bb);
template <typename T0, typename T1>
BoundingBox3d<T1> cast(const BoundingBox3d<T0>& bb);

// IO
template <typename T>
std::ostream& operator<<(std::ostream& out, const BoundingBox2d<T>& bb);
template <typename T>
std::ostream& operator<<(std::ostream& out, const BoundingBox3d<T>& bb);


//
//
// Implementation
//
//


template <typename T>
BoundingBox2d<T>& BoundingBox2d<T>::add(const Point2d<T>& p)
{
    rx.add(p.x);
    ry.add(p.y);
    return *this;
}

template <typename T>
BoundingBox2d<T>& BoundingBox2d<T>::add(T x, T y)
{
    rx.add(x);
    ry.add(y);
    return *this;
}

template <typename T>
BoundingBox2d<T>& BoundingBox2d<T>::add_border(const Vect2d<T>& v)
{
    rx.add_border(v.x);
    ry.add_border(v.y);
    return *this;
}

template <typename T>
BoundingBox2d<T>& BoundingBox2d<T>::add_border(T x, T y)
{
    rx.add_border(x);
    ry.add_border(y);
    return *this;
}

template <typename T>
BoundingBox2d<T>& BoundingBox2d<T>::add_border(T v)
{
    rx.add_border(v);
    ry.add_border(v);
    return *this;
}

template <typename T>
BoundingBox2d<T>& BoundingBox2d<T>::merge(const BoundingBox2d<T>& o)
{
    rx.merge(o.rx);
    ry.merge(o.ry);
    return *this;
}

template <typename T>
Point2d<T> BoundingBox2d<T>::min() const
{
    assert(is_populated());
    return Point2d<T>(rx.min, ry.min);
}

template <typename T>
Point2d<T> BoundingBox2d<T>::max() const
{
    assert(is_populated());
    return Point2d<T>(rx.max, ry.max);
}

template <typename T>
BoundingBox3d<T>& BoundingBox3d<T>::add(const Point3d<T>& p)
{
    rx.add(p.x);
    ry.add(p.y);
    rz.add(p.z);
    return *this;
}

template <typename T>
BoundingBox3d<T>& BoundingBox3d<T>::add(T x, T y, T z)
{
    rx.add(x);
    ry.add(y);
    rz.add(z);
    return *this;
}


template <typename T>
BoundingBox3d<T>& BoundingBox3d<T>::add_border(const Vect3d<T>& v)
{
    rx.add_border(v.x);
    ry.add_border(v.y);
    rz.add_border(v.z);
    return *this;
}

template <typename T>
BoundingBox3d<T>& BoundingBox3d<T>::add_border(T x, T y, T z)
{
    rx.add_border(x);
    ry.add_border(y);
    rz.add_border(z);
    return *this;
}

template <typename T>
BoundingBox3d<T>& BoundingBox3d<T>::add_border(T v)
{
    rx.add_border(v);
    ry.add_border(v);
    rz.add_border(v);
    return *this;
}

template <typename T>
BoundingBox3d<T>& BoundingBox3d<T>::merge(const BoundingBox3d<T>& o)
{
    rx.merge(o.rx);
    ry.merge(o.ry);
    rz.merge(o.rz);
    return *this;
}

template <typename T>
Point3d<T> BoundingBox3d<T>::min() const
{
    assert(is_populated());
    return Point3d<T>(rx.min, ry.min, rz.min);
}

template <typename T>
Point3d<T> BoundingBox3d<T>::max() const
{
    assert(is_populated());
    return Point3d<T>(rx.max, ry.max, rz.max);
}

template <typename T0, typename T1>
BoundingBox2d<T1> cast(const BoundingBox2d<T0>& bb)
{
    BoundingBox2d<T1> result;
    result.rx = cast<T0, T1>(bb.rx);
    result.ry = cast<T0, T1>(bb.ry);
    return result;
}

template <typename T0, typename T1>
BoundingBox3d<T1> cast(const BoundingBox3d<T0>& bb)
{
    BoundingBox3d<T1> result;
    result.rx = cast<T0, T1>(bb.rx);
    result.ry = cast<T0, T1>(bb.ry);
    result.rz = cast<T0, T1>(bb.rz);
    return result;
}

template <typename T>
std::ostream& operator<<(std::ostream& out, const BoundingBox2d<T>& bb)
{
    out << "{ rx: " << bb.rx << ", ry: " << bb.ry << " }";
    return out;
}

template <typename T>
std::ostream& operator<<(std::ostream& out, const BoundingBox3d<T>& bb)
{
    out << "{ rx: " << bb.rx << ", ry: " << bb.ry << ", rz: " << bb.rz << " }";
    return out;
}

} // namespace shapes
