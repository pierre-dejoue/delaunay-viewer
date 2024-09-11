// Copyright (c) 2024 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#include <dt/dt_interface.h>

#include <cassert>

namespace delaunay {

std::ostream& operator<<(std::ostream& out, TriangulationPolicy policy)
{
    switch (policy)
    {
        case TriangulationPolicy::PointCloud:
            out << "Point Cloud";
            break;

        case TriangulationPolicy::CDT:
            out << "Constrained Delaunay";
            break;

        default:
            assert(0);
            out << "Unknown enum";
    }
    return out;
}

} // namespace delaunay
