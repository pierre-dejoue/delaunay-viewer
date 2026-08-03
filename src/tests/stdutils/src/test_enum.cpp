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

TEST_CASE("stdutils::clamp a regular class enum", "[enum]")
{
    bool clamped = false;
    enum class TestEnum
    {
        A = 0,
        B,
        C,
    };

    // Test the constexpr version
    static_assert(stdutils::clamp<TestEnum>(TestEnum::A,               TestEnum::A, TestEnum::C) == TestEnum::A);
    static_assert(stdutils::clamp<TestEnum>(TestEnum::B,               TestEnum::A, TestEnum::C) == TestEnum::B);
    static_assert(stdutils::clamp<TestEnum>(TestEnum::C,               TestEnum::A, TestEnum::C) == TestEnum::C);
    static_assert(stdutils::clamp<TestEnum>(static_cast<TestEnum>(12), TestEnum::A, TestEnum::C) == TestEnum::C);

    // Test the version with bool& clamped
    TestEnum en = TestEnum::A;
    en = stdutils::clamp<TestEnum>(TestEnum::C, TestEnum::A, TestEnum::C, clamped);
    CHECK(en == TestEnum::C);
    CHECK(!clamped);
    en = stdutils::clamp<TestEnum>(TestEnum::A, TestEnum::A, TestEnum::C, clamped);
    CHECK(en == TestEnum::A);
    CHECK(!clamped);
    en = stdutils::clamp<TestEnum>(static_cast<TestEnum>(12), TestEnum::A, TestEnum::C, clamped);
    CHECK(en == TestEnum::C);
    CHECK(clamped);

    // The clamp_enum functions won't compile with regular enums
    //en = stdutils::clamp_enum<TestEnum>(TestEnum::A);                     // Won't compile
}

TEST_CASE("stdutils::clamp_enum", "[enum]")
{
    bool clamped = false;
    enum class TestEnum
    {
        A = 0,                          // The first enum value must be 0
        B,
        C,
        _ENUM_SIZE_                     // The last enum value must be the special value _ENUM_SIZE_
    };
    REQUIRE(stdutils::enum_size<TestEnum>() == 3);

    // Test the constexpr version
    static_assert(stdutils::clamp_enum<TestEnum>(TestEnum::A) == TestEnum::A);
    static_assert(stdutils::clamp_enum<TestEnum>(TestEnum::B) == TestEnum::B);
    static_assert(stdutils::clamp_enum<TestEnum>(TestEnum::C) == TestEnum::C);
    static_assert(stdutils::clamp_enum<TestEnum>(static_cast<TestEnum>(12)) == TestEnum::C);

    // Test the version with bool& clamped
    TestEnum en = TestEnum::A;
    en = stdutils::clamp_enum<TestEnum>(TestEnum::C, clamped);
    CHECK(en == TestEnum::C);
    CHECK(!clamped);
    en = stdutils::clamp_enum<TestEnum>(TestEnum::A, clamped);
    CHECK(en == TestEnum::A);
    CHECK(!clamped);
    en = stdutils::clamp_enum<TestEnum>(static_cast<TestEnum>(12), clamped);
    CHECK(en == TestEnum::C);
    CHECK(clamped);
}
