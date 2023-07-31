#include <dt/dt_impl.h>

#if BUILD_POLY2TRI
    #include "impl_poly2tri.h"
#endif
#if BUILD_CDT
    #include "impl_cdt.h"
#endif


namespace delaunay
{
namespace
{
    bool static_register_all_implementations()
    {
        bool success = true;

#if BUILD_POLY2TRI
        // Poly2tri
        success &= register_impl<double, std::uint32_t>("Poly2tri", &get_poly2tri_impl<double, std::uint32_t>);
#endif

#if BUILD_CDT
        // CDT
        success &= register_impl<double, std::uint32_t>("CDT", &get_cdt_impl<double, std::uint32_t>);
#endif

        return success;
    }
}

bool register_all_implementations()
{
    static bool all_registered = static_register_all_implementations();
    return all_registered;
}

} // namespace delaunay
