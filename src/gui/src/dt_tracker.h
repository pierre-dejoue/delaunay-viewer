#pragma once

#include <dt/dt_impl.h>

#include <cstdlib>
#include <limits>
#include <vector>

/**
 * List the implementations of the Delaunay triangulation algorithm.
 * Track which are enable/disable in the GUI.
 */
template <typename F, typename I = std::uint32_t>
class DtTracker
{
public:
    struct DelaunayAlgo
    {
        DelaunayAlgo(const delaunay::RegisteredImpl<F, I>& impl) : impl(std::move(impl)), active(true) {}
        const delaunay::RegisteredImpl<F, I> impl;
        bool active;
    };
    using DelaunayAlgos = std::vector<DelaunayAlgo>;

    DtTracker()
    {
        for (const auto& algo : delaunay::get_impl_list<F>().algos)
        {
            m_dt_algos.emplace_back(algo);
        }
    }

    const DelaunayAlgos& list_algos() const { return m_dt_algos; }
    DelaunayAlgos& list_algos() { return m_dt_algos; }

    std::size_t state_signature() const
    {
        assert(m_dt_algos.size() <= std::numeric_limits<std::size_t>::digits);
        std::size_t result = 0;
        std::size_t incr = 1;
        for (const auto& algo : m_dt_algos)
        {
            if (algo.active) { result += incr; }
            incr <<= 1;
        }
        return result;
    }

private:
    DelaunayAlgos m_dt_algos;
};
