#include <catch_amalgamated.hpp>

#include <shapes/sampling.h>
#include <shapes/path.h>

#include <cassert>
#include <vector>
#include <sstream>
#include <string>
#include <type_traits>

namespace shapes
{

template <typename F>
CubicBezierPath2d<F> test_cbp_2d()
{
    CubicBezierPath2d<F> out;
    out.closed = false;
    out.vertices.emplace_back(F{1}, F{0});
    out.vertices.emplace_back(F{3}, F{1});
    out.vertices.emplace_back(F{4}, F{-1});
    out.vertices.emplace_back(F{5}, F{0});
    assert(valid_size(out));
    return out;
}

TEST_CASE("Sample a CubicbezierCurve2d", "[sampling]")
{
    const auto cbp = test_cbp_2d<double>();

    REQUIRE(cbp.vertices.size() == 4);
    REQUIRE(nb_edges(cbp) == 1);

    UniformSamplingCubicBezier2d<double> sampler(cbp);

    // segment length ~ 4.2184
    const PointPath2d<double> pp = sampler.sample(0.25);

    CHECK(pp.closed == cbp.closed);
    CHECK(pp.vertices.size() == 18);
    CHECK(nb_edges(pp) == 17);
}

}
