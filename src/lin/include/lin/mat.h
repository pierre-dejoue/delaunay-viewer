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

    mat();
    mat(std::initializer_list<F> init_list);

    container& values() { return m_values; }
    const container& values() const { return m_values; }

    F* data() { return m_values.data(); }
    const F* data() const { return m_values.data(); }

    vect_map<F, M> operator[](dim_t row_idx)             { assert(row_idx < N); return vect_map<F, M>(m_values.data() + row_idx * M); }
    vect_map<const F, M> operator[](dim_t row_idx) const { assert(row_idx < N); return vect_map<const F, M>(m_values.data() + row_idx * M); }

private:
    container m_values;     // Row-major layout
};

template <typename F, dim_t N>
mat<F, N, N> identity();

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

// Compute the determinant
template <typename F>
F determinant(const mat2<F>& m);

// Matrix inverse. Check the determinant: If zero, then the inverse matric is irrelevant and should not be used.
template <typename F>
mat2<F> get_inverse(const mat2<F>& m, F* det_ptr = nullptr);

// In place inverse
template <typename F>
mat2<F>& inverse(mat2<F>& m, F* det_ptr = nullptr);


//
//
// Implementation
//
//


template <typename F, dim_t N, dim_t M>
mat<F, N, M>::mat()
    : m_values()
{
    for (auto& v: m_values)
        v = F{0};
}

template <typename F, dim_t N, dim_t M>
mat<F, N, M>::mat(std::initializer_list<F> init_list)
    : m_values()
{
    std::size_t idx = 0;
    for (const auto& v: init_list)
        m_values[idx++] = v;
}

template <typename F, dim_t N>
mat<F, N, N> identity()
{
    mat<F, N, N>  result;
    for (dim_t i = 0; i < N; i++)
        result[i][i] = F{1};
    return result;
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

} // namespace lin
