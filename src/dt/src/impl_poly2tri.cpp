#include "impl_poly2tri.h"

#include <dt/dt_impl.h>

#include <cstdint>


namespace delaunay
{
namespace
{
    bool register_all()
    {
        bool result = true;
        result &= register_impl<double, std::uint32_t>("Poly2tri", &get_poly2tri_impl<double, std::uint32_t>);
        return result;
    }
}
const bool poly2tri_is_registered = register_all();

} // namespace delaunay
