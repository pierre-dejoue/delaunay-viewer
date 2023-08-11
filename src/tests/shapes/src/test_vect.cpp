#include <catch_amalgamated.hpp>

#include <shapes/vect.h>

#include <vector>
#include <sstream>
#include <string>

template <>
struct Catch::StringMaker<shapes::Vect2d<float>>
{
    static std::string convert(const shapes::Vect2d<float>& v)
    {
        std::stringstream out;
        out << "[ " << v.x << ", " << v.y << " ]";
        return out.str();
    }
};

template <>
struct Catch::StringMaker<shapes::Vect3d<float>>
{
    static std::string convert(const shapes::Vect3d<float>& v)
    {
        std::stringstream out;
        out << "[ " << v.x << ", " << v.y << ", " << v.z << " ]";
        return out.str();
    }
};

namespace shapes
{

TEST_CASE("Test Vect2d operations", "[vect2d]")
{
    const Vect2d<float> a(2.f, 3.f);
    const Vect2d<float> b(5.f, 8.f);

    CHECK(a + b == Vect2d<float>(7.f, 11.f));
    CHECK(b - a == Vect2d<float>(3.f, 5.f));
    CHECK(3.f * b == Vect2d<float>(15.f, 24.f));
}

TEST_CASE("Test Vect3d operations", "[vect3d]")
{
    const Vect3d<float> a(2.f, 3.f, -1.f);
    const Vect3d<float> b(5.f, 8.f, 6.f);

    CHECK(a + b == Vect3d<float>(7.f, 11.f, 5.f));
    CHECK(b - a == Vect3d<float>(3.f, 5.f, 7.f));
    CHECK(3.f * b == Vect3d<float>(15.f, 24.f, 18.f));
}

}
