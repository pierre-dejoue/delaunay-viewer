#pragma once

#include <shapes/path.h>


namespace shapes
{

template <typename P>
PointPath<P> extract_endpoints(const CubicBezierPath<P>& cbp)
{
    PointPath<P> pp;
    const std::size_t segs = nb_edges(cbp);
    pp.vertices.reserve(segs + 1);
    for (unsigned int idx = 0; idx < segs; idx++)
    {
        pp.vertices.emplace_back(cbp.vertices[3 * idx]);
    }
    if (!cbp.closed)
    {
        assert(cbp.vertices.size() == 3 * segs + 1);
        pp.vertices.emplace_back(cbp.vertices.back());
    }
    pp.closed = cbp.closed;
    return pp;
}

} // namespace shapes
