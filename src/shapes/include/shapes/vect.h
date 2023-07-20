#pragma once

namespace shapes
{

template <typename F>
struct Vect2d
{
    static constexpr int dim = 2;
    using scalar = F;
    Vect2d() : x(F(0)), y(F(0)) {}
    Vect2d(F x, F y) : x(x), y(y) {}
    F x;
    F y;
};

template <typename F>
struct Vect3d
{
    static constexpr int dim = 3;
    using scalar = F;
    Vect3d() : x(F(0)), y(F(0)), z(F(0)) {}
    Vect3d(F x, F y, F z) : x(x), y(y), z(z) {}
    F x;
    F y;
    F z;
};

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


} // namespace shapes
