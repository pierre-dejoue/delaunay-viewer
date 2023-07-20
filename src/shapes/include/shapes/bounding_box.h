#pragma once

#include <shapes/point.h>

#include <algorithm>
#include <cassert>
#include <limits>
#include <ostream>


namespace shapes
{

template <typename F>
struct Range
{
    Range() : min(std::numeric_limits<F>::max()), max(std::numeric_limits<F>::lowest()) {}
    Range(F min, F max) : min(min), max(max) { assert(min <= max); }
    bool is_populated() const { return min <= max; }
    Range<F>& add(F v);
    Range<F>& merge(const Range<F>& o);
    F extent() const { return min <= max ? max - min : F(0); }
    F min;
    F max;
};

template <typename F>
struct BoundingBox2d
{
    bool is_populated() const { return rx.is_populated() && ry.is_populated(); }
    BoundingBox2d<F>& add(const Point2d<F>& p);
    BoundingBox2d<F>& merge(const BoundingBox2d& o);
    Point2d<F> min() const;
    Point2d<F> max() const;
    F width() const { return rx.extent(); }
    F height() const { return ry.extent(); }
    Range<F> rx;
    Range<F> ry;
};

template <typename F>
struct BoundingBox3d
{
    bool is_populated() const { return rx.is_populated() && ry.is_populated() && rz.is_populated(); }
    BoundingBox3d<F>& add(const Point3d<F>& p);
    BoundingBox3d<F>& merge(const BoundingBox3d& o);
    Point3d<F> min() const;
    Point3d<F> max() const;
    Range<F> rx;
    Range<F> ry;
    Range<F> rz;
};

template <typename F>
std::ostream& operator<<(std::ostream& out, const Range<F>& range);
template <typename F>
std::ostream& operator<<(std::ostream& out, const BoundingBox2d<F>& bb);
template <typename F>
std::ostream& operator<<(std::ostream& out, const BoundingBox3d<F>& bb);


//
// Implementations
//

template <typename F>
Range<F>& Range<F>::add(F v)
{
    min = std::min(min, v);
    max = std::max(max, v);
    return *this;
}

template <typename F>
Range<F>& Range<F>::merge(const Range<F>& o)
{
    min = std::min(min, o.min);
    max = std::max(max, o.max);
    return *this;
}

template <typename F>
BoundingBox2d<F>& BoundingBox2d<F>::add(const Point2d<F>& p)
{
    rx.add(p.x);
    ry.add(p.y);
    return *this;
}

template <typename F>
BoundingBox2d<F>& BoundingBox2d<F>::merge(const BoundingBox2d<F>& o)
{
    rx.merge(o.rx);
    ry.merge(o.ry);
    return *this;
}

template <typename F>
Point2d<F> BoundingBox2d<F>::min() const
{
    assert(is_populated());
    return Point2d<F>(rx.min, ry.min);
}

template <typename F>
Point2d<F> BoundingBox2d<F>::max() const
{
    assert(is_populated());
    return Point2d<F>(rx.max, ry.max);
}

template <typename F>
BoundingBox3d<F>& BoundingBox3d<F>::add(const Point3d<F>& p)
{
    rx.add(p.x);
    ry.add(p.y);
    rz.add(p.z);
    return *this;
}

template <typename F>
BoundingBox3d<F>& BoundingBox3d<F>::merge(const BoundingBox3d<F>& o)
{
    rx.merge(o.rx);
    ry.merge(o.ry);
    rz.merge(o.rz);
    return *this;
}

template <typename F>
Point3d<F> BoundingBox3d<F>::min() const
{
    assert(is_populated());
    return Point3d<F>(rx.min, ry.min, rz.min);
}

template <typename F>
Point3d<F> BoundingBox3d<F>::max() const
{
    assert(is_populated());
    return Point3d<F>(rx.max, ry.max, rz.max);
}

template <typename F>
std::ostream& operator<<(std::ostream& out, const Range<F>& range)
{
    if (range.is_populated())
        out << "[ " << range.min << ", " << range.max << " ]" ;
    else
        out << "[ empty ]";
    return out;
}

template <typename F>
std::ostream& operator<<(std::ostream& out, const BoundingBox2d<F>& bb)
{
    out << "{ rx: " << bb.rx << ", ry: " << bb.ry << " }";
    return out;
}

template <typename F>
std::ostream& operator<<(std::ostream& out, const BoundingBox3d<F>& bb)
{
    out << "{ rx: " << bb.rx << ", ry: " << bb.ry << ", rz: " << bb.rz << " }";
    return out;
}

} // namespace shapes