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

TEST_CASE("Remove spaces", "[stdutils::string]")
{
    CHECK(stdutils::string::remove_all_spaces("All Those Moments ")  == "AllThoseMoments");
}

TEST_CASE("strnlen", "[stdutils::string]")
{
    CHECK(stdutils::strnlen(nullptr) == 0);

    const char* test = "All Those MOMENTS";
    CHECK(stdutils::strnlen(test)     == 17);
    CHECK(stdutils::strnlen(test, 10) == 10);
    CHECK(stdutils::strnlen(test, 17) == 17);
    CHECK(stdutils::strnlen(test, 18) == 17);
}

TEST_CASE("is_null_terminated", "[stdutils::string]")
{
    CHECK(stdutils::string::is_null_terminated(nullptr) == false);

    const char* test = "will be lost in time";
    CHECK(stdutils::strnlen(test) == 20);
    CHECK(stdutils::string::is_null_terminated(test)     == true);
    CHECK(stdutils::string::is_null_terminated(test, 10) == false);
    CHECK(stdutils::string::is_null_terminated(test, 20) == false);
    CHECK(stdutils::string::is_null_terminated(test, 21) == true);
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

TEST_CASE("UTF8 utilities", "[stdutils::string]")
{
    // Null string
    CHECK(stdutils::string::is_well_formed_utf8(nullptr) == false);
    CHECK(stdutils::string::count_utf8_chars(nullptr) == 0);

    // Valid UTF8 string
    std::size_t error_loc = 0;
    {
        // Japanese string with 6 characters (="ロケット燃料")
        const stdutils::u8_c_str test_valid = "\xE3\x83\xAD\xE3\x82\xB1\xE3\x83\x83\xE3\x83\x88\xE7\x87\x83\xE6\x96\x99";
        CHECK(stdutils::string::is_well_formed_utf8(test_valid, &error_loc) == true);
        CHECK(error_loc == 0);
        CHECK(stdutils::string::is_well_formed_utf8(stdutils::u8_string(test_valid)) == true);
        CHECK(error_loc == 0);
        CHECK(stdutils::string::count_utf8_chars(test_valid) == 6);
        CHECK(stdutils::string::count_utf8_chars(stdutils::u8_string(test_valid)) == 6);
    }

    // Several examples of ill-formed UTF8 strings
    {
        //                                               v
        const stdutils::u8_c_str test_wrong = "\xE3\x83\xDD\xE3\x82\xB1\xE3\x83\x83\xE3\x83\x88\xE7\x87\x83\xE6\x96\x99";
        CHECK(stdutils::string::is_well_formed_utf8(test_wrong, &error_loc) == false);
        CHECK(error_loc == 2);
    }
    {
        //                                                   v
        const stdutils::u8_c_str test_wrong = "\xE3\x83\xAD\xA3\x82\xB1\xE3\x83\x83\xE3\x83\x88\xE7\x87\x83\xE6\x96\x99";
        CHECK(stdutils::string::is_well_formed_utf8(stdutils::to_u8_string(test_wrong), &error_loc) == false);
        CHECK(error_loc == 3);
    }
    {
        //                                                   v
        const stdutils::u8_c_str test_wrong = "\xE3\x83\xAD\xFB\x82\xB1\xE3\x83\x83\xE3\x83\x88\xE7\x87\x83\xE6\x96\x99";
        CHECK(stdutils::string::is_well_formed_utf8(test_wrong, &error_loc) == false);
        CHECK(error_loc == 3);
    }
    {
        //                                                                     v
        const stdutils::u8_c_str test_wrong = "\xE3\x83\xAD\xE3\x82\xB1\xE3\x83";
        CHECK(stdutils::string::is_well_formed_utf8(stdutils::to_u8_string(test_wrong), &error_loc) == false);
        CHECK(error_loc == 8);
    }
    {
        //                                                                   v
        const stdutils::u8_c_str test_wrong = "\xE3\x83\xAD\xE3\x82\xB1\x33\x83\x83\xE3\x83\x88\xE7\x87\x83\xE6\x96\x99";
        CHECK(stdutils::string::is_well_formed_utf8(stdutils::to_u8_string(test_wrong), &error_loc) == false);
        CHECK(error_loc == 7);
    }
    {
        //                                       v
        const stdutils::u8_c_str test_wrong = "\x83\x83\xAD\xE3\x82\xB1\xE3\x83\x83\xE3\x83\x88\xE7\x87\x83\xE6\x96\x99";
        CHECK(stdutils::string::is_well_formed_utf8(test_wrong, &error_loc) == false);
        CHECK(error_loc == 0);
    }
}

TEST_CASE("Indentation", "[stdutils::string]")
{
    const stdutils::string::Indent indent(4);       // My indentation is 4 spaces
    CHECK_FALSE(indent.empty());
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
        CHECK(out.str().size() == 0);
    }
}

TEST_CASE("Zero-length indentation", "[stdutils::string]")
{
    const stdutils::string::Indent indent(0);       // Indentation is empty
    CHECK(indent.empty());
    {
        std::stringstream out;
        out << indent;                              // Output 1 indentation
        CHECK(out.str().size() == 0);
    }
    {
        std::stringstream out;
        out << indent(2);                           // Output 2 indentations
        CHECK(out.str().size() == 0);
    }
    {
        std::stringstream out;
        out << indent(0);                           // Output zero indentation
        CHECK(out.str().size() == 0);
    }
}

TEST_CASE("Split strings", "[stdutils::string]")
{
    const std::string_view empty_str;
    const auto parts_0 = stdutils::string::split(empty_str, ' ');
    CHECK(parts_0.size() == 1);
    CHECK((!parts_0.empty() && parts_0.at(0).empty()));

    const std::string_view test_str_1 = "a,b,c";
    const auto l_split_1 = stdutils::string::l_split(test_str_1, ',');
    CHECK(l_split_1.first == "a");
    CHECK(l_split_1.second == "b,c");
    const auto r_split_1 = stdutils::string::r_split(test_str_1, ',');
    CHECK(r_split_1.first == "a,b");
    CHECK(r_split_1.second == "c");

    const std::string_view test_str_2 = "They,took,from,their,surroundings,what,was,needed,,,and,made,of,it,something,more";
    const auto parts_1 = stdutils::string::split(test_str_2, ',');
    CHECK(parts_1.size() == 16);
    const auto parts_2 = stdutils::string::split(test_str_2, ' ');
    CHECK(parts_2.size() == 1);

    // Trailing delimiter does not affect the result of the split.
    const std::string_view test_str_3 = "Some,fungus,";
    const auto parts_3 = stdutils::string::split(test_str_3, ',');
    CHECK(parts_3.size() == 3);
}

TEST_CASE("Split strings: split vs split_skip_empty", "[stdutils::string]")
{
    const std::string_view test_str_1 = "a,b,c";
    const std::string_view test_str_2 = "a,,b,c";
    const std::string_view test_str_3 = ",a,b,c,";

    const std::vector<std::string_view> abc { "a", "b", "c" };

    CHECK(stdutils::string::split(test_str_1, ',') == abc);
    CHECK(stdutils::string::split(test_str_2, ',') == std::vector<std::string_view>{ "a", "", "b", "c" });
    CHECK(stdutils::string::split(test_str_3, ',') == std::vector<std::string_view>{ "", "a", "b", "c", "" });

    CHECK(stdutils::string::split_skip_empty(test_str_1, ',') == abc);
    CHECK(stdutils::string::split_skip_empty(test_str_2, ',') == abc);
    CHECK(stdutils::string::split_skip_empty(test_str_3, ',') == abc);
}

TEST_CASE("Replace one word in a string", "[stdutils::string]")
{
    constexpr std::string_view src_str = "Attack ships on fire off the shoulder of {0}.";
    constexpr std::size_t src_sz = src_str.size();

    // In place
    {
        std::string test_str(src_str);
        const auto success = stdutils::string::replace_first_in_place(test_str, "{0}", "Orion");
        CHECK(success);
        CHECK(test_str.size() == src_sz + 2);
        CHECK(test_str.find("Orion") != std::string::npos);
    }
    {
        // Fail to replace
        std::string test_str(src_str);
        const auto success = stdutils::string::replace_first_in_place(test_str, "{1}", "Orion");
        CHECK_FALSE(success);
        CHECK(test_str == src_str);
    }
    {
        // Corner case: empty 'from' replace pattern
        std::string test_str(src_str);
        const auto success = stdutils::string::replace_first_in_place(test_str, "", "Orion");
        CHECK_FALSE(success);
        CHECK(test_str == src_str);
    }

    // Allocate a new std::string
    {
        bool success = false;
        const std::string result = stdutils::string::replace_first(src_str, "{0}", "Orion", success);
        CHECK(success);
        CHECK(result.size() == src_sz + 2);
        CHECK(result.find("Orion") != std::string::npos);
    }
    {
        // Fail to replace
        bool success = false;
        const std::string result = stdutils::string::replace_first(src_str, "{1}", "Orion", success);
        CHECK_FALSE(success);
        CHECK(result == src_str);
    }

    // Replace in output stream
    {
        std::stringstream out;
        bool success = stdutils::string::replace_first(out, src_str, "{0}", "Orion");
        CHECK(success);
        std::string result = out.str();
        CHECK(result.size() == src_sz + 2);
        CHECK(result.find("Orion") != std::string::npos);
    }
    {
        // Fail to replace
        std::stringstream out;
        bool success = stdutils::string::replace_first(out, src_str, "{1}", "Orion");
        CHECK_FALSE(success);
        CHECK(out.str() == src_str);
    }
    {
        // Corner case: empty 'from' replace pattern
        std::stringstream out;
        bool success = stdutils::string::replace_first(out, src_str, "", "Orion");
        CHECK_FALSE(success);
        CHECK(out.str() == src_str);
    }
}

TEST_CASE("Replace all occuurence of a word in a string", "[stdutils::string]")
{
    constexpr std::string_view src_str = "AABBCCCCAABBCCCCDDCCCCBBAA";

    // Replace in output stream
    {
        std::stringstream out;
        stdutils::string::replace_all(out, src_str, "ZZ", "YY");
        std::string result = out.str();
        CHECK(result == src_str);
    }
    {
        std::stringstream out;
        stdutils::string::replace_all(out, src_str, "AA", "ZZ");
        std::string result = out.str();
        CHECK(result == "ZZBBCCCCZZBBCCCCDDCCCCBBZZ");
    }
    {
        std::stringstream out;
        stdutils::string::replace_all(out, src_str, "CC", "Z");
        std::string result = out.str();
        CHECK(result == "AABBZZAABBZZDDZZBBAA");
    }
    {
        // Corner case: to = from
        std::stringstream out;
        stdutils::string::replace_all(out, src_str, "BB", "BB");
        std::string result = out.str();
        CHECK(result == src_str);
    }
    {
        // Corner case: from = ""
        std::stringstream out;
        stdutils::string::replace_all(out, src_str, "", "ZZ");
        std::string result = out.str();
        CHECK(result == src_str);
    }
    {
        // Coner case: 'from' is a substring of 'to'.
        // In that case we do not want to recursively replace, which could lead to an infinite loop.
        std::stringstream out;
        stdutils::string::replace_all(out, src_str, "AA", "AAAAA");
        std::string result = out.str();
        CHECK(result == "AAAAABBCCCCAAAAABBCCCCDDCCCCBBAAAAA");
    }
}
