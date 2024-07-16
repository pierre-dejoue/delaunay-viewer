// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <shapes/vect.h>

#include <cassert>
#include <cstdlib>
#include <cstring>

namespace shapes {

template <typename F>
class QuadraticBezierMap2d
{
public:
    static constexpr int dim = 2;
    using scalar = F;
    explicit QuadraticBezierMap2d(const F* control_points) : p(control_points) { assert(p); }
    explicit QuadraticBezierMap2d(const Vect2d<F>* control_points);
    Vect2d<F> at(float t) const;
    Vect2d<F> first() const;
    Vect2d<F> last() const;
    const F* cps() const { return p; }

private:
    const F* const p;
};

template <typename F>
class QuadraticBezierMap3d
{
public:
    static constexpr int dim = 3;
    using scalar = F;
    explicit QuadraticBezierMap3d(const F* control_points) : p(control_points) { assert(p); }
    explicit QuadraticBezierMap3d(const Vect3d<F>* control_points);
    Vect3d<F> at(float t) const;
    Vect3d<F> first() const;
    Vect3d<F> last() const;
    const F* cps() const { return p; }

private:
    const F* const p;
};

template <typename F>
class CubicBezierMap2d
{
public:
    static constexpr int dim = 2;
    using scalar = F;
    explicit CubicBezierMap2d(const F* control_points) : p(control_points) { assert(p); }
    explicit CubicBezierMap2d(const Vect2d<F>* control_points);
    Vect2d<F> at(float t) const;
    Vect2d<F> first() const;
    Vect2d<F> last() const;
    const F* cps() const { return p; }

private:
    const F* const p;
};

template <typename F>
class CubicBezierMap3d
{
public:
    static constexpr int dim = 3;
    using scalar = F;
    explicit CubicBezierMap3d(const F* control_points) : p(control_points) { assert(p); }
    explicit CubicBezierMap3d(const Vect3d<F>* control_points);
    Vect3d<F> at(float t) const;
    Vect3d<F> first() const;
    Vect3d<F> last() const;
    const F* cps() const { return p; }

private:
    const F* const p;
};

/**
 * Casteljau algorithm: Split a Bezier curve into two parts
 */
template <typename F>
class CasteljauCubicBezier2d
{
public:
    static constexpr int dim = 2;
    using scalar = F;
    explicit CasteljauCubicBezier2d(const CubicBezierMap2d<F>& bezier, float t = 0.5f);
    CubicBezierMap2d<F> bezier() const;
    CubicBezierMap2d<F> split0() const;
    CubicBezierMap2d<F> split1() const;
    Vect2d<F> split_point() const;

private:
    std::array<F, 4 * dim> cps;
    std::array<F, 7 * dim> split_cps;
};


//
//
// Implementation
//
//


template <typename F>
QuadraticBezierMap2d<F>::QuadraticBezierMap2d(const Vect2d<F>* control_points)
    : QuadraticBezierMap2d(reinterpret_cast<const F*>(control_points))
{
    static_assert(sizeof(Vect2d<F>) == 2 * sizeof(F));
    assert(control_points);
}

template <typename F>
Vect2d<F> QuadraticBezierMap2d<F>::at(float t) const
{
    assert(0.f <= t && t <= 1.f);
    const F w0 = static_cast<F>((1 - t) * (1 - t));
    const F w1 = static_cast<F>(2.f * t * (1 - t));
    const F w2 = static_cast<F>(t * t);
    return Vect2d<F>(
        w0 * p[0] + w1 * p[2] + w2 * p[4],
        w0 * p[1] + w1 * p[3] + w2 * p[5]
    );
}

template <typename F>
Vect2d<F> QuadraticBezierMap2d<F>::first() const
{
    return Vect2d<F>(p[0], p[1]);
}

template <typename F>
Vect2d<F> QuadraticBezierMap2d<F>::last() const
{
    return Vect2d<F>(p[4], p[5]);
}

template <typename F>
QuadraticBezierMap3d<F>::QuadraticBezierMap3d(const Vect3d<F>* control_points)
    : QuadraticBezierMap3d(reinterpret_cast<const F*>(control_points))
{
    static_assert(sizeof(Vect3d<F>) == 3 * sizeof(F));
    assert(control_points);
}

template <typename F>
Vect3d<F> QuadraticBezierMap3d<F>::at(float t) const
{
    assert(0.f <= t && t <= 1.f);
    const F w0 = static_cast<F>((1 - t) * (1 - t));
    const F w1 = static_cast<F>(2.f * t * (1 - t));
    const F w2 = static_cast<F>(t * t);
    return Vect3d<F>(
        w0 * p[0] + w1 * p[3] + w2 * p[6],
        w0 * p[1] + w1 * p[4] + w2 * p[7],
        w0 * p[2] + w1 * p[5] + w2 * p[8]
    );
}

template <typename F>
Vect3d<F> QuadraticBezierMap3d<F>::first() const
{
    return Vect3d<F>(p[0], p[1], p[2]);
}

template <typename F>
Vect3d<F> QuadraticBezierMap3d<F>::last() const
{
    return Vect3d<F>(p[6], p[7], p[8]);
}

template <typename F>
CubicBezierMap2d<F>::CubicBezierMap2d(const Vect2d<F>* control_points)
    : CubicBezierMap2d(reinterpret_cast<const F*>(control_points))
{
    static_assert(sizeof(Vect2d<F>) == 2 * sizeof(F));
    assert(control_points);
}

template <typename F>
Vect2d<F> CubicBezierMap2d<F>::at(float t) const
{
    assert(0.f <= t && t <= 1.f);
    const float sq0 = (1 - t) * (1 - t);
    const float sq1 = t * t;
    const F w0 = static_cast<F>((1 - t) * sq0);
    const F w1 = static_cast<F>(3.f * sq0 * t);
    const F w2 = static_cast<F>(3.f * (1 - t) * sq1);
    const F w3 = static_cast<F>(t * sq1);
    return Vect2d<F>(
        w0 * p[0] + w1 * p[2] + w2 * p[4] + w3 * p[6],
        w0 * p[1] + w1 * p[3] + w2 * p[5] + w3 * p[7]
    );
}

template <typename F>
Vect2d<F> CubicBezierMap2d<F>::first() const
{
    return Vect2d<F>(p[0], p[1]);
}

template <typename F>
Vect2d<F> CubicBezierMap2d<F>::last() const
{
    return Vect2d<F>(p[6], p[7]);
}

template <typename F>
CubicBezierMap3d<F>::CubicBezierMap3d(const Vect3d<F>* control_points)
    : CubicBezierMap3d(reinterpret_cast<const F*>(control_points))
{
    static_assert(sizeof(Vect3d<F>) == 3 * sizeof(F));
    assert(control_points);
}

template <typename F>
Vect3d<F> CubicBezierMap3d<F>::at(float t) const
{
    assert(0.f <= t && t <= 1.f);
    const float sq0 = (1 - t) * (1 - t);
    const float sq1 = t * t;
    const F w0 = static_cast<F>((1 - t) * sq0);
    const F w1 = static_cast<F>(3.f * sq0 * t);
    const F w2 = static_cast<F>(3.f * (1 - t) * sq1);
    const F w3 = static_cast<F>(t * sq1);
    return Vect3d<F>(
        w0 * p[0] + w1 * p[3] + w2 * p[6] + w3 * p[9],
        w0 * p[1] + w1 * p[4] + w2 * p[7] + w3 * p[10],
        w0 * p[2] + w1 * p[5] + w2 * p[8] + w3 * p[11]
    );
}

template <typename F>
Vect3d<F> CubicBezierMap3d<F>::first() const
{
    return Vect3d<F>(p[0], p[1], p[2]);
}

template <typename F>
Vect3d<F> CubicBezierMap3d<F>::last() const
{
    return Vect3d<F>(p[9], p[10], p[11]);
}

template <typename F>
CasteljauCubicBezier2d<F>::CasteljauCubicBezier2d(const CubicBezierMap2d<F>& bezier, float t)
    : cps()
    , split_cps()
{
    static_assert(sizeof(Vect2d<F>) == dim * sizeof(F));
    const F* p = bezier.cps();
    assert(p);
    assert(0.f <= t && t <= 1.f);
    std::memcpy(cps.data(), p, 4 * dim * sizeof(F));

    // P0, P1, P2, P3
    const F* p0 = &p[0 * dim];
    const F* p1 = &p[1 * dim];
    const F* p2 = &p[2 * dim];
    const F* p3 = &p[3 * dim];
    split_cps[0] = p0[0];
    split_cps[1] = p0[1];
    split_cps[6 * dim + 0] = p3[0];
    split_cps[6 * dim + 1] = p3[1];

    // Q0, Q1, Q2
    Vect2d<F> q1_tmp;
    F* q0 = &split_cps[1 * dim];
    F* q1 = &q1_tmp.x;
    F* q2 = &split_cps[5 * dim];

    // R0, R1
    F* r0 = &split_cps[2 * dim];
    F* r1 = &split_cps[4 * dim];

    // S0
    F* s0 = &split_cps[3 * dim];

    const F u = static_cast<F>(1.0f - t);
    const F v = static_cast<F>(t);
    for (auto i = 0; i < dim; i++)
    {
        q0[i] = u * p0[i] + v * p1[i];
        q1[i] = u * p1[i] + v * p2[i];
        q2[i] = u * p2[i] + v * p3[i];
        r0[i] = u * q0[i] + v * q1[i];
        r1[i] = u * q1[i] + v * q2[i];
        s0[i] = u * r0[i] + v * r1[i];
    }
}

template <typename F>
CubicBezierMap2d<F> CasteljauCubicBezier2d<F>::bezier() const
{
    return CubicBezierMap2d<F>(&cps[0]);
}

template <typename F>
CubicBezierMap2d<F> CasteljauCubicBezier2d<F>::split0() const
{
    return CubicBezierMap2d<F>(&split_cps[0]);
}

template <typename F>
CubicBezierMap2d<F> CasteljauCubicBezier2d<F>::split1() const
{
    return CubicBezierMap2d<F>(&split_cps[3 * dim]);
}

template <typename F>
Vect2d<F> CasteljauCubicBezier2d<F>::split_point() const
{
    const F* s = &split_cps[3 * dim];
    return Vect2d<F>(s[0], s[1]);
}

} // namespace shapes
