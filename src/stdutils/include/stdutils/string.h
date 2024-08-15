// Copyright (c) 2021 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <cstdlib>
#include <ostream>
#include <string>
#include <string_view>

namespace stdutils {

namespace ascii {

/**
 * ASCII characters manipulation
 *
 * Contrary to the std::isalpha, etc. functions, the locale is ignored.
 * Only the ASCII characters are considered.
 */
bool isalpha(char c) noexcept;
bool isalnum(char c) noexcept;
bool isprint(char c) noexcept;
bool isspace(char c) noexcept;
bool isupper(char c) noexcept;
bool islower(char c) noexcept;
char tolower(char c) noexcept;
char toupper(char c) noexcept;

// Escape sequence, e.g. \x0A for '\t'
struct HexEscape
{
    char c;
};
std::ostream& operator<<(std::ostream& out, const HexEscape& hex_escape);

} // namesapce ascii

namespace string {

/**
 * String conversion functions
 */
std::string tolower(const std::string& in);
std::string toupper(const std::string& in);
std::string capitalize(const std::string& in);

/**
 * String length with max size
 */
constexpr std::size_t DEFAULT_MAX_LEN = 1048576u;       // 2^20

std::size_t strnlen(const char* str, std::size_t max_len = DEFAULT_MAX_LEN);

/**
 * Validate that a string is null terminated
 */
bool is_null_terminated(const char* str, std::size_t max_len = DEFAULT_MAX_LEN);

/**
 * Validate that a string is pure ASCII (1-127), and null terminated
 */
bool is_pure_ascii(const char* str, std::size_t max_len = DEFAULT_MAX_LEN);
bool is_pure_ascii(const std::string& str);

/**
 * Validate that a string is made of printable ASCII chars in the strict sense (32-127), and null terminated
 */
bool is_strictly_print_ascii(const char* str, std::size_t max_len = DEFAULT_MAX_LEN);
bool is_strictly_print_ascii(const std::string& str);

/**
 * Validate that a string is made of printable ASCII chars (32-127) and whitespaces (including '\n'), and null terminated
 */
bool is_printable_ascii(const char* str, std::size_t max_len = DEFAULT_MAX_LEN);
bool is_printable_ascii(const std::string& str);

/**
 * A string identifier
 *
 * It shall only contain the characters: 'a-z', 'A-Z', '0-9', '#', '.', '-', '_'
 * It shall not be empty and begin with an alphabetical character
 */
using Id = const char*;

bool is_valid_id(Id id);

/**
 * Indent: Utility class to easily output indentation to a stream
 *
 * Example usage:
 *
 *      const Indent indent(4);     // My indentation is 4 spaces
 *
 *      out << indent;              // Output 1 indentation
 *      out << indent(2);           // Output 2 indentations
 */
template <typename CharT>
class BasicIndent
{
public:
    class Multi;

    BasicIndent(std::size_t count, CharT ch = ' ') : m_str(count, ch) {}

    Multi operator()(std::size_t factor) const { return Multi(*this, factor); }

    friend std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& out, const BasicIndent<CharT>& indent)
    {
        return out << indent.m_str;
    }

private:
    std::basic_string<CharT> m_str;
};

template <typename CharT>
class BasicIndent<CharT>::Multi
{
public:
    Multi(const BasicIndent& indent, std::size_t factor) : m_indent(indent), m_factor(factor) {}

    // NB: Read https://isocpp.org/wiki/faq/templates#template-friends regarding templated friend functions
    friend std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& out, const Multi& multi_indent)
    {
        for (auto c = 0u; c < multi_indent.m_factor; c++)
            out << multi_indent.m_indent;
        return out;
    }
private:
    const BasicIndent<CharT>& m_indent;
    std::size_t m_factor;
};

using Indent = BasicIndent<char>;

} // namespace string

} // namespace stdutils
