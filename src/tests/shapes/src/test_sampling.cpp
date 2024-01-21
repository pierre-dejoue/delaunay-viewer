// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#include <catch_amalgamated.hpp>

#include "trace.h"

#include <shapes/io.h>
#include <shapes/sampling.h>
#include <shapes/path.h>
#include <stdutils/io.h>

#include <algorithm>
#include <cassert>
#include <iomanip>
#include <vector>
#include <sstream>
#include <string>
#include <type_traits>

namespace shapes {

template <typename F>
CubicBezierPath2d<F> test_cbp_2d_1()
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

template <typename F>
CubicBezierPath2d<F> test_cbp_2d_2()
{
    CubicBezierPath2d<F> out;
    out.closed = false;
    out.vertices.emplace_back(F{25}, F{5});
    out.vertices.emplace_back(F{28}, F{5});
    out.vertices.emplace_back(F{28.34375}, F{5.03125});
    out.vertices.emplace_back(F{31.63281}, F{6.33594});
    assert(valid_size(out));
    return out;
}


template <typename F>
CubicBezierPath2d<F> test_cbp_2d_3()
{
    // This CBP has a null derivate at the t=0 endpoint
    // It is harder to sample uniformly at this endpoint.
    // It is slightly adapted from asset icons8-homer-simpson.svg on which the issue was first observed
    CubicBezierPath2d<F> out;
    out.closed = false;
    out.vertices.emplace_back(F{25}, F{5});
    out.vertices.emplace_back(F{25}, F{5});
    out.vertices.emplace_back(F{28.34375}, F{5.03125});
    out.vertices.emplace_back(F{31.63281}, F{6.33594});
    assert(valid_size(out));
    return out;
}

template <typename F>
CubicBezierPath2d<F> test_cbp_2d_4()
{
    // This CBP has a null derivate at both endpoints.
    // It is harder to sample uniformly at those endpoints.
    // Visually it looks like a straight segment.
    CubicBezierPath2d<F> out;
    out.closed = false;
    out.vertices.emplace_back(F{25}, F{5});
    out.vertices.emplace_back(F{25}, F{5});
    out.vertices.emplace_back(F{31.63281}, F{6.33594});
    out.vertices.emplace_back(F{31.63281}, F{6.33594});
    assert(valid_size(out));
    return out;
}

template <typename F>
F sample_length_for_n_edges(F curve_length, std::size_t n)
{
    assert(n > 0);
    const F nf = static_cast<F>(n);
    return curve_length / nf * (F{1} + F{0.5} / nf);
}

TEST_CASE("Sample a CubicbezierCurve2d", "[sampling]")
{
    using FP = float;
    const auto cbp = test_cbp_2d_1<FP>();

    REQUIRE(cbp.vertices.size() == 4);
    REQUIRE(nb_edges(cbp) == 1);

    // Sampler
    UniformSamplingCubicBezier2d<FP>::InitTraceInfo trace_info;
    UniformSamplingCubicBezier2d<FP> sampler(cbp, &trace_info);
    CHECK(!trace_info.iterations.empty());

    // Curve length
    const auto curve_length = sampler.max_segment_length();
    CHECK_THAT(curve_length, Catch::Matchers::WithinRel(4.2184, 0.001));

    {
        const PointPath2d<FP> pp = sampler.sample(0.25f);
        CHECK(pp.closed == cbp.closed);
        CHECK(pp.vertices.size() == 18);
        CHECK(nb_edges(pp) == 17);
    }
    {
        const PointPath2d<FP> pp = sampler.sample(0.1f);
        CHECK(pp.closed == cbp.closed);
        CHECK(pp.vertices.size() == 44);
        CHECK(nb_edges(pp) == 43);
    }
    {
        const PointPath2d<FP> pp = sampler.sample(sample_length_for_n_edges(curve_length, 200));
        CHECK(pp.closed == cbp.closed);
        CHECK(pp.vertices.size() == 201);
        CHECK(nb_edges(pp) == 200);
    }
}

TEST_CASE("Sample cubic Bezier curves and measure accuracy", "[sampling]")
{
    auto trace_out = stdutils::trace_open_file("cbp_sampling_path_uniformity_stats.txt", "sampling");
    using FP = double;
    const std::vector<CubicBezierPath2d<FP>> curves = {
        test_cbp_2d_1<FP>(),
        test_cbp_2d_2<FP>(),
        test_cbp_2d_3<FP>(),
        test_cbp_2d_4<FP>()
    };

    const std::vector<std::size_t> requested_nb_edges = { 50, 100, 500, 1000, 2000 };

    // This test is specific to open curves composed of a unique cubic Bezier
    for (std::size_t idx = 0u; idx < curves.size(); idx++)
    {
        CAPTURE(idx);
        const auto& cbp = curves[idx];
        REQUIRE(cbp.vertices.size() == 4);
        REQUIRE(cbp.closed == false);
        REQUIRE(nb_edges(cbp) == 1);
        REQUIRE(is_valid(cbp));

        shapes::io::dat::save_shapes_as_oneliner_stream(trace_out, { cbp }, "END\n");

        // Sampler
        UniformSamplingCubicBezier2d<FP>::InitTraceInfo trace_info;
        UniformSamplingCubicBezier2d<FP> sampler(cbp, &trace_info);
        CHECK(!trace_info.iterations.empty());

        const auto curve_length = sampler.max_segment_length();

        for (const auto& nb_edges : requested_nb_edges)
        {
            CAPTURE(nb_edges);
            const PointPath2d<FP> pp = sampler.sample(sample_length_for_n_edges(curve_length, nb_edges));
            CHECK(shapes::nb_edges(pp) == nb_edges);
            REQUIRE(!pp.vertices.empty());
            const auto uniformity_stats = path_normalized_uniformity_stats(pp);
            stdutils::io::SaveNumericFormat save_fmt(trace_out);
            trace_out << "edges: " << std::setw(5) << nb_edges << " "
                      << std::setprecision(3) << std::scientific
                      << " normalized_range: " << uniformity_stats.range
                      << " normalized_stdev: " << uniformity_stats.stdev << std::endl;
            // Expect better than a 10% range spread on the segment lengths
            CHECK(uniformity_stats.range < 0.10);
        }
    }
}

} // namespace shapes
