// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#include <catch_amalgamated.hpp>

#include <stdutils/string.h>

#include <limits>
#include <sstream>
#include <string>

namespace {

template <typename F, typename charT = char>
std::string ascii_filter(const F& f)
{
    std::stringstream out;
    for (charT c = std::numeric_limits<charT>::min(); c < std::numeric_limits<charT>::max(); c++)
    {
        if (f(c)) { out << c; }
    }
    return out.str();
}

std::string hex_escape_as_string(char c)
{
    std::stringstream out;
    out << stdutils::ascii::HexEscape{c};
    return out.str();
}

std::string convert_all_chars_to_hex_escape(const char* c_ptr)
{
    std::stringstream out;
    while(char c = *c_ptr++)
        out << stdutils::ascii::HexEscape{c};
    return out.str();
}

} // namespace

TEST_CASE("ASCII manipulation", "[stdutils::ascii]")
{
    const auto all_alpha = ascii_filter(stdutils::ascii::isalpha);
    CHECK(all_alpha == "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
    const auto all_alnum = ascii_filter(stdutils::ascii::isalnum);
    CHECK(all_alnum == "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
    const auto all_print = ascii_filter(stdutils::ascii::isprint);
    CHECK(all_print == " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~");
    const auto all_lower = ascii_filter(stdutils::ascii::islower);
    CHECK(all_lower == "abcdefghijklmnopqrstuvwxyz");
    const auto all_upper = ascii_filter(stdutils::ascii::isupper);
    CHECK(all_upper == "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
}

TEST_CASE("Hexadecimal escape sequence", "[stdutils::ascii]")
{
    CHECK(hex_escape_as_string(0)    == "\\x00");
    CHECK(hex_escape_as_string('0')  == "\\x30");
    CHECK(hex_escape_as_string('\t') == "\\x09");
    CHECK(hex_escape_as_string('\n') == "\\x0A");
    const char* kana = "\xE3\x82\xB1";  // UTF8 encoding of the japanese character 'ケ'
    const auto kana_esc = convert_all_chars_to_hex_escape(kana);
    CHECK(kana_esc.size() == 12);
    CHECK(kana_esc == "\\xE3\\x82\\xB1");
}

TEST_CASE("strings tolower, toupper and capitalize", "[stdutils::string]")
{
    CHECK(stdutils::string::tolower("All Those MOMENTS")  == "all those moments");
    CHECK(stdutils::string::toupper("will be lost in time") == "WILL BE LOST IN TIME");
    CHECK(stdutils::string::capitalize("like tears in rain.") == "Like tears in rain.");
}

TEST_CASE("strnlen", "[stdutils::string]")
{
    CHECK(stdutils::string::strnlen(nullptr) == 0);

    const char* test = "All Those MOMENTS";
    CHECK(stdutils::string::strnlen(test) == 17);
    CHECK(stdutils::string::strnlen(test, 10) == 10);
}

TEST_CASE("is_null_terminated", "[stdutils::string]")
{
    CHECK(stdutils::string::is_null_terminated(nullptr) == false);

    const char* test = "will be lost in time";
    CHECK(stdutils::string::is_null_terminated(test) == true);
    CHECK(stdutils::string::is_null_terminated(test, 10) == false);
}

TEST_CASE("is_pure_ascii", "[stdutils::string]")
{
    CHECK(stdutils::string::is_pure_ascii(nullptr) == false);
    {
        const char* test = "will be lost in time";
        CHECK(stdutils::string::is_pure_ascii(test) == true);
        CHECK(stdutils::string::is_pure_ascii(test, 10) == false);
    }
    {
        const char* test = "AB\xC3Z";
        CHECK(stdutils::string::is_pure_ascii(test) == false);
        CHECK(stdutils::string::is_pure_ascii(test, 1) == false);
    }
}

TEST_CASE("is printable ASCII", "[stdutils::string]")
{
    CHECK(stdutils::string::is_strictly_print_ascii(nullptr) == false);
    CHECK(stdutils::string::is_printable_ascii(nullptr) == false);
    {
        const char* test = "will [be] lost in %time";
        CHECK(stdutils::string::is_strictly_print_ascii(test) == true);
        CHECK(stdutils::string::is_printable_ascii(test) == true);
    }
    {
        // Tabulation is not printable in the strict sense ('\t' is a control character, see documentation of std::isprint)
        const char* test = "a\tb\tc";
        CHECK(stdutils::string::is_strictly_print_ascii(test) == false);
        CHECK(stdutils::string::is_printable_ascii(test) == true);
    }
    {
        // A multiline is not printable in the strict sense (eol characters are control characters)
        const char* test = "A,B,C\na,b,c\n";
        CHECK(stdutils::string::is_strictly_print_ascii(test) == false);
        CHECK(stdutils::string::is_printable_ascii(test) == true);
    }
}

TEST_CASE("is_valid_id", "[stdutils::string]")
{
    // Valid
    CHECK(stdutils::string::is_valid_id("abc") == true);
    CHECK(stdutils::string::is_valid_id("ABC") == true);
    CHECK(stdutils::string::is_valid_id("rocket-fuel") == true);
    CHECK(stdutils::string::is_valid_id("Andromeda_galaxy") == true);
    CHECK(stdutils::string::is_valid_id("mat4x4") == true);
    CHECK(stdutils::string::is_valid_id("a.b.c") == true);
    CHECK(stdutils::string::is_valid_id("param_#01") == true);

    // Invalid
    CHECK(stdutils::string::is_valid_id(nullptr) == false);       // Must be non-null, non-empty
    CHECK(stdutils::string::is_valid_id("") == false);
    CHECK(stdutils::string::is_valid_id("1abc") == false);        // Must begin with an alphabetical char
    CHECK(stdutils::string::is_valid_id("#abc") == false);
    CHECK(stdutils::string::is_valid_id("_abc") == false);
    CHECK(stdutils::string::is_valid_id("a b c") == false);       // Must not contain any whitespace character
    CHECK(stdutils::string::is_valid_id("a\tb\tc") == false);
    CHECK(stdutils::string::is_valid_id("a,b,c") == false);       // Must contain only the ASCII alphanumerical characters, '#', '.', '-', and '_'
    CHECK(stdutils::string::is_valid_id("a;b;c") == false);
    CHECK(stdutils::string::is_valid_id(u8"àbç") == false);
    CHECK(stdutils::string::is_valid_id("4x4mat") == false);
}

TEST_CASE("Indentation", "[stdutils::string]")
{
    const stdutils::string::Indent indent(4);       // My indentation is 4 spaces
    {
        std::stringstream out;
        out << indent;                              // Output 1 indentation
        CHECK(out.str().size() == 4);
    }
    {
        std::stringstream out;
        out << indent(2);                           // Output 2 indentations
        CHECK(out.str().size() == 8);
    }
    {
        std::stringstream out;
        out << indent(0);                           // Output zero indentation
        CHECK(out.str().empty());
    }
}
