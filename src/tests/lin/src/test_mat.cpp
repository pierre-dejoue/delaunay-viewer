#include <catch_amalgamated.hpp>

#include <lin/mat.h>

#include <vector>
#include <sstream>
#include <string>

namespace lin {

TEST_CASE("Test identity matrix", "[mat]")
{
    // Generic identity
    {
        constexpr dim_t N = 7;
        const auto id = identity<float, N>();
        for (dim_t i = 0; i < N; i++)
        {
            for (dim_t j = 0; j < N; j++)
            {
                CAPTURE(i); CAPTURE(j);
                CHECK(id[i][j] == ((i == j) ? 1.f : 0.f));
            }
        }
    }
    // Use pre-defined types
    {
        const auto id = mat3f::identity();
        for (dim_t i = 0; i < 3; i++)
        {
            for (dim_t j = 0; j < 3; j++)
            {
                CAPTURE(i); CAPTURE(j);
                CHECK(id[i][j] == ((i == j) ? 1.f : 0.f));
            }
        }
    }
}

TEST_CASE("Test matrix memory layout is row-major", "[mat]")
{
    const mat4d test {
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

TEST_CASE("Test matrix copy", "[mat]")
{
    mat4d test {
        1,  2,  3,  4,
        5,  6,  7,  8,
        9, 10, 11, 12,
        0,  0,  0,  1
    };

    const mat4d copy_test(test);

    CHECK(copy_test.rows == 4);
    CHECK(copy_test.cols == 4);
    test[2][2] = 42;
    CHECK(copy_test[2][2] == 11);
}

} // namespace lin
