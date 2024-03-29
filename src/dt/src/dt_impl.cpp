// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#include <dt/dt_impl.h>

#if BUILD_POLY2TRI
    #include "impl_poly2tri.h"
#endif
#if BUILD_CDT
    #include "impl_cdt.h"
#endif
#if BUILD_TRIANGLE
    #include "impl_triangle.h"
#endif

namespace delaunay {
namespace {

bool static_register_all_implementations()
{
    bool success = true;

#if BUILD_TRIANGLE
    // Shewchuk's Triangle
    success &= register_impl<double, std::uint32_t>("Triangle", 4, &get_triangle_impl<double, std::uint32_t>);
#endif

#if BUILD_POLY2TRI
    // Poly2tri (the library only supports double)
    success &= register_impl<double, std::uint32_t>("Poly2tri", 3, &get_poly2tri_impl<double, std::uint32_t>);
#endif

#if BUILD_CDT
    // CDT double
    success &= register_impl<double, std::uint32_t>("CDT_fp64", 2, &get_cdt_impl<double, double, std::uint32_t>);
    // CDT float
    success &= register_impl<double, std::uint32_t>("CDT_fp32", 1, &get_cdt_impl<float, double, std::uint32_t>);
#endif

    return success;
}

} // namespace

bool register_all_implementations()
{
    static bool all_registered = static_register_all_implementations();
    return all_registered;
}

} // namespace delaunay
