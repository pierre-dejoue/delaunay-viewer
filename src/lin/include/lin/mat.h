#pragma once

#include <lin/vect.h>

#include <cassert>
#include <initializer_list>


namespace lin
{

// NxM matrix (mathematical notation, meaning N = Rows and M = Cols)
template <typename F, index N, index M>
class mat
{
    using container = std::array<F, N * M>;
public:
    using scalar = F;
    static constexpr auto rows = N;
    static constexpr auto cols = M;

    mat();
    mat(std::initializer_list<F> init_list);

    //explicit mat(const container& arr) : m_values(arr) {}
    //explicit mat(container&& arr) : m_values(std::move(arr)) {}
    //mat operator=(const container& arr) { m_values = arr; }
    //mat operator=(container&& arr) { m_values = std::move(arr); }

    container& values() { return m_values; }
    const container& values() const { return m_values; }

    F* data() { return m_values.data(); }
    const F* data() const { return m_values.data(); }

    vect_map<F, M> operator[](index row_idx)             { assert(row_idx < N); return vect_map<F, M>(m_values.data() + row_idx * M); }
    vect_map<const F, M> operator[](index row_idx) const { assert(row_idx < N); return vect_map<const F, M>(m_values.data() + row_idx * M); }

private:
    container m_values;     // Row-major layout
};

template <typename F, index N>
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

//
// Implementations
//
template <typename F, index N, index M>
mat<F, N, M>::mat()
    : m_values()
{
    for (auto& v: m_values)
        v = F{0};
}

template <typename F, index N, index M>
mat<F, N, M>::mat(std::initializer_list<F> init_list)
    : m_values()
{
    std::size_t idx = 0;
    for (const auto& v: init_list)
        m_values[idx++] = v;
}

template <typename F, index N>
mat<F, N, N> identity()
{
    mat<F, N, N>  result;
    for (index i = 0; i < N; i++)
        result[i][i] = F{1};
    return result;
}

} // namespace lin
