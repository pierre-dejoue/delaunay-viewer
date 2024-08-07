// Copyright (c) 2021 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <ostream>
#include <string>
#include <string_view>

namespace stdutils {
namespace string {

/**
 * String conversion functions
 */
std::string tolower(const std::string& in);

std::string toupper(const std::string& in);

std::string capitalize(const std::string& in);

/**
 * Current local date and time
 */
std::string current_local_date_and_time();

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
