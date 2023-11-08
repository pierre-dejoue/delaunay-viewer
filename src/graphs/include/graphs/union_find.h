#pragma once

#include <graphs/index.h>

#include <cassert>
#include <cstdint>
#include <exception>
#include <stdexcept>
#include <vector>

namespace graphs
{

/**
 * Union-Find is a classical data structure used to represent a partition of a set of elements.
 *
 * It offers fast (O(ln n)) methods to:
 *  - FIND the subset an element belongs to;
 *  - compute the UNION of two subsets.
 *
 * For a presentation of this data structure:
 *  - Skiena, S. (2020). Weighted Graph Algorithms, p. 198. In: The Algorithm Design Manual. 2nd Edition. Springer.
 */
template <typename I = std::uint32_t>
class UnionFind {
public:
    using index = I;

    UnionFind(I set_size) :
        m_graph(set_size)
    {
        assert(set_size <= IndexTraits<I>::max_valid_index());
        I idx = 0;
        for (auto& elt : m_graph)
        {
            elt.parent = idx++;
            elt.subset_size = 1;
        }
        assert(idx == set_size);
    }

    I size() const { return static_cast<I>(m_graph.size()); }

    I find(I i) const
    {
        I j{0};
        const std::size_t n = m_graph.size();
        std::size_t c = 0;
        while((j = m_graph.at(i).parent) != i && c < n) { i = j; }
        if (c == n)
            throw std::logic_error("Infinite loop detected in UnionFind::find()");
        assert(m_graph.at(i).parent == i);
        return i;
    }

    void subset_union(I i, I j)
    {
        I parent_i = find(i);
        I parent_j = find(j);
        if (parent_i == parent_j)
            return;     // Nothing to do

        Elt& root_i = m_graph[parent_i];
        assert(root_i.parent == parent_i);
        Elt& root_j = m_graph[parent_j];
        assert(root_j.parent == parent_j);

        if (root_i.subset_size < root_j.subset_size)
        {
            root_i.parent = parent_j;
            root_j.subset_size += root_i.subset_size;
        }
        else
        {
            root_j.parent = parent_i;
            root_i.subset_size += root_j.subset_size;
        }
    }

    I subset_size(I i) const
    {
        return m_graph.at(find(i)).subset_size;
    }

private:
    struct Elt
    {
        I parent;               // Graph root is such that parent index == element index
        I subset_size;          // Only correct if this element is a root
    };
    std::vector<Elt> m_graph;
};

} // namespace graphs
