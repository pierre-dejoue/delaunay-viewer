// Copyright (c) 2025 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#include <catch_amalgamated.hpp>

#include <stdutils/enum.h>

TEST_CASE("Test enum utility templates", "[enum]")
{
    enum class Enumeration
    {
        A = 0,                          // The first enum value must be 0
        B,
        C,
        _ENUM_SIZE_                     // The last enum value must be the special value _ENUM_SIZE_
    };

    CHECK(stdutils::enum_first_value<Enumeration>() == Enumeration::A);
    CHECK(stdutils::enum_last_value<Enumeration>()  == Enumeration::C);
    CHECK(stdutils::enum_size<Enumeration>() == 3);

    // enum_size can cast the return value to any unsigned integral type
    CHECK(stdutils::enum_size<Enumeration, unsigned short>() == 3);
    //CHECK(stdutils::enum_size<Enumeration, int>() == 3);          // Would trigger a static_assert
    //CHECK(stdutils::enum_size<Enumeration, float>() == 3.f);      // Would trigger a static_assert

    // enum_is_in_range check that the integer value of the enumeration is strictly lower than the enum size
    CHECK(stdutils::enum_is_in_range(Enumeration::A));
    CHECK(stdutils::enum_is_in_range(Enumeration::B));
    CHECK(stdutils::enum_is_in_range(Enumeration::C));
    CHECK(stdutils::enum_is_in_range(Enumeration::_ENUM_SIZE_) == false);
    CHECK(stdutils::enum_is_in_range(static_cast<Enumeration>(42)) == false);

    // The argument of enum_is_in_range must be an enumeration
    //CHECK(stdutils::enum_is_in_range(0));                         // Would trigger a static_assert
}


TEST_CASE("Test enum utility templates on a non-compliant enumeration", "[enum]")
{
    enum class Enumeration
    {
        A = 1,                          // The first enum value must be 0
        B,
        C,
        // Missing the special value _ENUM_SIZE
    };

    CHECK(stdutils::enum_first_value<Enumeration>() != Enumeration::A);
    CHECK(stdutils::enum_first_value<Enumeration>() == static_cast<Enumeration>(0));
    //CHECK(stdutils::enum_last_value<Enumeration>()  == Enumeration::C);   // Won't compile
    //CHECK(stdutils::enum_size<Enumeration>() == 3);                       // Won't compile
    //CHECK(stdutils::enum_is_in_range(Enumeration::A));                    // Won't compile
}
