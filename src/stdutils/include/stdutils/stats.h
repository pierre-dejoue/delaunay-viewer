// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <stdutils/algorithm.h>
#include <stdutils/io.h>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <ostream>
#include <stdexcept>
#include <vector>

namespace stdutils {
namespace stats {

// F: a scalar
template <typename F>
struct Result
{
    Result()
        : n(0u)
        , min{}, max{}, range{}
        , mean{}
        , variance{}, stdev{}
    { }

    Result normalize_to(F unit) const;
    Result normalize_to_mean() const;

    std::size_t n;
    F min;
    F max;
    F range;
    F mean;
    F variance;
    F stdev;
};

template <typename F>
std::ostream& operator<<(std::ostream& out, const Result<F>& result);

template <typename F>
class CumulSamples
{
public:
    CumulSamples() : m_sum{}, m_sum_sq{}, m_result(), m_prev_n{0u} { }

    void add_sample(const F& val);

    template <typename InputIt>
    void add_samples(InputIt begin, InputIt end);

    void add_samples(const CumulSamples& other);

    CumulSamples& operator+=(const CumulSamples& other);

    std::size_t nb_samples() const;

    const Result<F>& get_result() const;

    bool empty() const;

private:
    F m_sum;
    F m_sum_sq;
    mutable Result<F> m_result;
    mutable std::size_t m_prev_n;
};

// The median, which cannot be computed in a cumulative way
template <typename F, typename InputIt>
F median(InputIt begin, InputIt end);


//
//
// Implementation
//
//


template <typename F>
Result<F> Result<F>::normalize_to(F unit) const
{
    auto cpy = *this;
    if (std::fpclassify(unit) == FP_ZERO || !std::isfinite(unit) || unit < F{0})
        throw std::invalid_argument("The unit value is not a finite value or is <= 0");
    const F inv_unit = 1 / unit;
    cpy.min *= inv_unit;
    cpy.max *= inv_unit;
    cpy.range = cpy.max - cpy.min;
    cpy.mean *= inv_unit;
    cpy.variance *= inv_unit * inv_unit;
    cpy.stdev *= inv_unit;
    return cpy;
}

template <typename F>
Result<F> Result<F>::normalize_to_mean() const
{
    return normalize_to(mean);
}

template <typename F>
void CumulSamples<F>::add_sample(const F& val)
{
    if (m_result.n == 0u)
    {
        m_result.min = val;
        m_result.max = val;
    }
    else
    {
        min_update(m_result.min, val);
        max_update(m_result.max, val);
    }
    m_result.n++;
    m_sum += val;
    m_sum_sq += val * val;
}

template <typename F>
template <typename InputIt>
void CumulSamples<F>::add_samples(InputIt begin, InputIt end)
{
    for (auto it = begin; it != end; ++it)
        add_sample(*it);
}

template <typename F>
void CumulSamples<F>::add_samples(const CumulSamples<F>& other)
{
    if (m_result.n == 0u)
    {
        m_result.min = other.m_result.min;
        m_result.max = other.m_result.max;
    }
    else if (other.m_result.n != 0u)
    {
        min_update(m_result.min, other.m_result.min);
        max_update(m_result.max, other.m_result.max);
    }
    m_result.n += other.m_result.n;
    m_sum += other.m_sum;
    m_sum_sq += other.m_sum_sq;
}

template <typename F>
CumulSamples<F>& CumulSamples<F>::operator+=(const CumulSamples<F>& other)
{
    add_samples(other);
    return *this;
}

template <typename F>
std::size_t CumulSamples<F>::nb_samples() const
{
    return m_result.n;
}

template <typename F>
const Result<F>& CumulSamples<F>::get_result() const
{
    if (m_result.n != m_prev_n)
    {
        assert(m_result.n > 0u);
        // min, max are already up-to-date
        m_result.range = m_result.max - m_result.min;
        m_result.mean = m_sum / static_cast<F>(m_result.n);
        m_result.variance = m_sum_sq / static_cast<F>(m_result.n) - (m_result.mean * m_result.mean);
        m_result.stdev = std::sqrt(m_result.variance);
        m_prev_n = m_result.n;
    }
    assert(m_result.n == m_prev_n);
    return m_result;
}

template <typename F>
bool CumulSamples<F>::empty() const
{
    return m_result.n == 0u;
}

template <typename F, typename InputIt>
F median(InputIt begin, InputIt end)
{
    std::vector<F> samples(begin, end);
    std::sort(samples.begin(), samples.end());
    return samples.empty() ? F{} : samples[(samples.size() - 1u) / 2u];
}

template <typename F>
std::ostream& operator<<(std::ostream& out, const Result<F>& result)
{
    stdutils::io::SaveNumericFormat save_fmt(out);
    out << "samples: " << result.n
        << std::setprecision(3) << std::scientific
        << ", min: " << result.min
        << ", max: " << result.max
        << ", mean: " << result.mean
        << ", stdev: " << result.stdev;
    return out;
}

} // namespace stats
} // namespace stdutils
