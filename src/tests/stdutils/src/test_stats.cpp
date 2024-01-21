// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#include <catch_amalgamated.hpp>

#include <stdutils/numbers.h>
#include <stdutils/stats.h>

#include <array>
#include <cmath>
#include <sstream>
#include <vector>

namespace {
    template <typename F>
    std::vector<F> gen_circle_1d_distrib(std::size_t n, F r, F offset = F{0})
    {
        std::vector<F> result;
        result.reserve(n);
        F inv_n = F{1} / static_cast<F>(n);
        F two_pi = F{2} * stdutils::numbers::pi_v<F>;
        for (std::size_t idx = 0; idx < n; idx++)
        {
            result.emplace_back(offset + r * std::cos(static_cast<F>(idx) * inv_n * two_pi));
        }
        return result;
    }
}

TEST_CASE("CumulSamples basic", "[stats]")
{
    stdutils::stats::CumulSamples<float> samples;

    CHECK(samples.empty() == true);
    CHECK(samples.get_result().n == 0);
    CHECK(samples.get_result().mean == 0.f);

    samples.add_sample(3.f);

    CHECK(samples.empty() == false);
    CHECK(samples.get_result().n == 1);
    CHECK(samples.get_result().min == 3.f);
    CHECK(samples.get_result().max == 3.f);
    CHECK(samples.get_result().mean == 3.f);
    CHECK(samples.get_result().variance == 0.f);
    CHECK(samples.get_result().stdev == 0.f);

    samples.add_sample(7.f);

    CHECK(samples.get_result().n == 2);
    CHECK(samples.get_result().min == 3.f);
    CHECK(samples.get_result().max == 7.f);
    CHECK(samples.get_result().mean == 5.f);
    CHECK(samples.get_result().variance == 4.f);
    CHECK(samples.get_result().stdev == 2.f);

    std::array<float, 6> more_samples = { 1.f, 8.f, 5.f, 2.f, 4.f, 6.f };
    samples.add_samples(std::cbegin(more_samples), std::cend(more_samples));

    CHECK(samples.get_result().n == 8);
    CHECK(samples.get_result().min == 1.f);
    CHECK(samples.get_result().max == 8.f);
    CHECK(samples.get_result().mean == 4.5f);
    CHECK(samples.get_result().variance == 5.25f);
    CHECK_THAT(samples.get_result().stdev, Catch::Matchers::WithinRel(2.2913f, 0.0001f));
}

TEST_CASE("CumulSamples on circle distribution", "[stats]")
{
    stdutils::stats::CumulSamples<float> samples;

    const auto distrib = gen_circle_1d_distrib<float>(30, 2.f);
    samples.add_samples(std::cbegin(distrib), std::cend(distrib));

    CHECK(samples.empty() == false);
    CHECK(samples.get_result().n == 30);
    CHECK_THAT(samples.get_result().mean, Catch::Matchers::WithinAbs(0.0, 0.0001));
    CHECK_THAT(samples.get_result().stdev, Catch::Matchers::WithinRel(1.4142f, 0.0001f));
}

TEST_CASE("median", "[stats]")
{
    const std::array<float, 10> samples { 1.f, 1.f, 2.f, 1.f, 2.f, 3.f, 1.f, 2.f, 3.f, 4.f };

    CHECK(stdutils::stats::median<float>(samples.data(), samples.data()) == 0.f);
    CHECK(stdutils::stats::median<float>(samples.data(), samples.data() + 1) == 1.f);
    CHECK(stdutils::stats::median<float>(samples.data(), samples.data() + 2) == 1.f);
    CHECK(stdutils::stats::median<float>(samples.data(), samples.data() + 6) == 1.f);
    CHECK(stdutils::stats::median<float>(samples.data(), samples.data() + 9) == 2.f);
    CHECK(stdutils::stats::median<float>(samples.data(), samples.data() + 10) == 2.f);
}

TEST_CASE("ostream", "[stats]")
{
    std::stringstream out;
    out << stdutils::stats::Result<float>();
    CHECK(!out.str().empty());
}
