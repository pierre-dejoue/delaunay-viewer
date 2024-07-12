// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <lin/vect.h>

#include <algorithm>
#include <cassert>
#include <initializer_list>

namespace lin {

// NxM matrix (mathematical notation, meaning N = Rows and M = Cols)
template <typename F, dim_t N, dim_t M>
class mat
{
    using container = std::array<F, N * M>;

public:
    using scalar = F;
    static constexpr auto rows = N;
    static constexpr auto cols = M;

    constexpr mat();
    constexpr mat(std::initializer_list<F> init_list);

    container& values() { return m_values; }
    const container& values() const { return m_values; }

    F* data() { return m_values.data(); }
    const F* data() const { return m_values.data(); }

    vect_map<F, M> operator[](dim_t row_idx)             { assert(row_idx < N); return vect_map<F, M>(m_values.data() + row_idx * M); }
    vect_map<const F, M> operator[](dim_t row_idx) const { assert(row_idx < N); return vect_map<const F, M>(m_values.data() + row_idx * M); }

    static constexpr mat identity();

private:
    container m_values;     // Row-major layout
};

template <typename F>
using mat2 = mat<F, 2, 2>;
template <typename F>
using mat3 = mat<F, 3, 3>;
template <typename F>
using mat4 = mat<F, 4, 4>;

using mat2f = mat2<float>;
using mat3f = mat3<float>;
using mat4f = mat4<float>;

using mat2d = mat2<double>;
using mat3d = mat3<double>;
using mat4d = mat4<double>;

// Square identity matrix
template <typename F, dim_t N>
constexpr mat<F, N, N> identity();

// Compute the determinant
template <typename F>
F determinant(const mat2<F>& m);

// Matrix inverse. Check the determinant: If zero, then the inverse matrix is irrelevant and should not be used.
template <typename F>
mat2<F> get_inverse(const mat2<F>& m, F* det_ptr = nullptr);

// In place inverse
template <typename F>
mat2<F>& inverse(mat2<F>& m, F* det_ptr = nullptr);

template <typename F, dim_t N>
vect<F, N> operator*(const mat<F, N, N>& m, const vect<F, N>& x);


//
//
// Implementation
//
//


template <typename F, dim_t N, dim_t M>
constexpr mat<F, N, M>::mat()
    : m_values()
{
    for (auto& v: m_values)
        v = F{0};
}

template <typename F, dim_t N, dim_t M>
constexpr mat<F, N, M>::mat(std::initializer_list<F> init_list)
    : m_values()
{
    std::size_t idx = 0;
    for (const auto& v: init_list)
        m_values[idx++] = v;
}

template <typename F, dim_t N, dim_t M>
constexpr mat<F, N, M> mat<F, N, M>::identity()
{
    mat<F, N, M> result;
    const dim_t end_i = (N < M ? N : M);
    for (dim_t i = 0; i < end_i; i++)
        result.m_values[i * (M + 1)] = F{1};
    return result;
}

template <typename F, dim_t N>
constexpr mat<F, N, N> identity()
{
    return mat<F, N, N>::identity();
}

template <typename F>
F determinant(const mat2<F>& m)
{
    return m[0][0] * m[1][1] - m[0][1] * m[1][0];
}

template <typename F>
mat2<F> get_inverse(const mat2<F>& m, F* det_ptr)
{
    mat2<F> result = m;
    inverse(result, det_ptr);
    return result;
}

template <typename F>
mat2<F>& inverse(mat2<F>& m, F* det_ptr)
{
    const F det = determinant(m);
    if (det_ptr != nullptr) { *det_ptr = det; }
    std::swap(m[0][0], m[1][1]);
    m[0][1] = - m[0][1];
    m[1][0] = - m[1][0];
    if (det != F{0})
    {
        const F inv_det = F{1} / det;
        auto& vals = m.values();
        vals[0] *= inv_det;
        vals[1] *= inv_det;
        vals[2] *= inv_det;
        vals[3] *= inv_det;
    }
    return m;
}

template <typename F, dim_t N>
vect<F, N> operator*(const mat<F, N, N>& m, const vect<F, N>& x)
{
    vect<F, N> y;
    for (dim_t i = 0; i < N; i++)
    {
        y[i] = F{0};
        for (dim_t j = 0; j < N; j++)
            y[i] += m[i][j] * x[j];
    }
    return y;
}

} // namespace lin
