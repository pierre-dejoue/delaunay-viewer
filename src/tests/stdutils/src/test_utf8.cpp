// Copyright (c) 2024 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#include <catch_amalgamated.hpp>

#include <stdutils/utf8.h>

//
// CAUTION: This source file is saved with the following encoding: UTF8 with BOM. DO NOT CHANGE THE ENCODING!
//

// This test should pass on C++11, C++17 and C++20
TEST_CASE("u8 strings from u8 literals", "[stdutils::string]")
{
    //
    // u8 string literals are a convenient way added in C++11 to ensure a string is encoded in UTF8.
    // However is suffers from a backward compatibility issue. Until C++20, the underlying type was a char[].
    // Since C++20, it is a char8_t[] where char8_t is a new fundamental type (and a new keyword) similar
    // to an unsigned char. Some code compiling in C++17 such as:
    //
    //     const char* test = u8"Test";
    //
    // will become ill-formed in C++20.
    //
    // Utility functions stdutils::to_u8_c_str and stdutils::to_u8string are there to ensure a backward compatibility.
    //

    {
        // If the string is pure ascii no need to bother with u8 literals
        const stdutils::u8_string test = "Pure ASCII";
        CHECK(test.size() == 10);
    }

    {
        // Requires the source file to be explicitely UTF8 encoded
        const stdutils::u8_string test_raw = stdutils::to_u8_string(u8"ロケット燃料");        // Japanese for "rocket fuel"

        // Use escape characters
        const stdutils::u8_string test_esc = "\xE3\x83\xAD\xE3\x82\xB1\xE3\x83\x83\xE3\x83\x88\xE7\x87\x83\xE6\x96\x99";

        CHECK(test_raw == test_esc);
    }
}

