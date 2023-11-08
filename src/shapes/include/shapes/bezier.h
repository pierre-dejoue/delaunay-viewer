// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <shapes/vect.h>

namespace shapes
{

template <typename F>
class QuadraticBezierMap2d
{
public:
    QuadraticBezierMap2d(const F * control_points) : p(control_points) {}
    Vect2d<F> at(float t);

private:
    F const * const p;
};

template <typename F>
class QuadraticBezierMap3d
{
public:
    QuadraticBezierMap3d(const F * control_points) : p(control_points) {}
    Vect3d<F> at(float t);

private:
    F const * const p;
};

template <typename F>
class CubicBezierMap2d
{
public:
    CubicBezierMap2d(const F * control_points) : p(control_points) {}
    Vect2d<F> at(float t);

private:
    F const * const p;
};

template <typename F>
class CubicBezierMap3d
{
public:
    CubicBezierMap3d(const F * control_points) : p(control_points) {}
    Vect3d<F> at(float t);

private:
    F const * const p;
};

//
// Implementations
//
template <typename F>
Vect2d<F> QuadraticBezierMap2d<F>::at(float t)
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
Vect3d<F> QuadraticBezierMap3d<F>::at(float t)
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
Vect2d<F> CubicBezierMap2d<F>::at(float t)
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
Vect3d<F> CubicBezierMap3d<F>::at(float t)
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

} // namespace shapes
