#include <catch_amalgamated.hpp>

#include <lin/mat.h>

#include <vector>
#include <sstream>
#include <string>

namespace lin {

TEST_CASE("Test identity matrix", "[lin::mat]")
{
    constexpr index N = 4;

    auto id = identity<float, N>();
    for (index i = 0; i < N; i++)
        for (index j = 0; j < N; j++)
        {
            CAPTURE(i); CAPTURE(j);
            CHECK(id[i][j] == ((i == j) ? 1.f : 0.f));
        }
}

TEST_CASE("Test matrix memory layout is row-major", "[lin::mat]")
{
    const mat4d test = {
        1,  2,  3,  4,
        5,  6,  7,  8,
        9, 10, 11, 12,
        0,  0,  0,  1
    };

    CHECK(test.rows == 4);
    CHECK(test.cols == 4);
    CHECK(test[2][2] == 11);
    CHECK(test[2][0] == 9);
    CHECK(test[1][3] == 8);
}

} // namespace lin
