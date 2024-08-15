// Copyright (c) 2021 Pierre DEJOUE
// This code is distributed under the terms of the MIT License

#include <stdutils/macros.h>
#include <stdutils/string.h>

#include <algorithm>

namespace stdutils {

namespace ascii {

bool isalpha(char c) noexcept { return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z'); }

bool isalnum(char c) noexcept { return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9'); }

bool isprint(char c) noexcept { return char{32} <= c && c < char{127}; }                    // All printable characters (Note the absence of '\t' = \x9)

bool isspace(char c) noexcept { return (char(9) <= c && c <= char{13}) || c == ' '; }       // Namely '\t', '\n', '\v', '\f', '\r' and the regular space

bool isupper(char c) noexcept { return ('A' <= c && c <= 'Z'); }

bool islower(char c) noexcept { return ('a' <= c && c <= 'z'); }

char tolower(char c) noexcept { return ('A' <= c && c <= 'Z') ? c + char{0x20} : c; }

char toupper(char c) noexcept { return ('a' <= c && c <= 'z') ? c - char{0x20} : c; }

std::ostream& operator<<(std::ostream& out, const HexEscape& hex_escape)
{
    char a = '0' + ((hex_escape.c & 0xF0) >> 4);
    char b = '0' + (hex_escape.c & 0x0F);
    if (a > 0x39) { a += 7; }               // For ABCDEF
    if (b > 0x39) { b += 7; }
    return out << "\\x" << a << b;
}

} // namespace ascii

namespace string {

std::string tolower(const std::string& in)
{
    std::string out(in);
    std::transform(in.cbegin(), in.cend(), out.begin(), [](char c) -> char { return ('A' <= c && c <= 'Z') ? c + char{0x20} : c; });
    return out;
}

std::string toupper(const std::string& in)
{
    std::string out(in);
    std::transform(in.cbegin(), in.cend(), out.begin(), [](char c) -> char { return ('a' <= c && c <= 'z') ? c - char{0x20} : c; });
    return out;
}

std::string capitalize(const std::string& in)
{
    std::string out = tolower(in);
    if (!out.empty()) { out.front() = ::stdutils::ascii::toupper(out.front()); }
    return out;
}

std::size_t strnlen(const char* str, std::size_t max_len)
{
    if (str == nullptr)
        return 0;
    std::size_t len = 0;
    while(*str++ && len < max_len) { len++; }
    return len;
}

bool is_null_terminated(const char* str, std::size_t max_len)
{
    return str != nullptr && strnlen(str, max_len) != max_len;
}

namespace {

struct PredicatePureASCII
{
    static bool eval(const char& c) { return c > 0; }
};

struct PredicateStrictlyPrintASCII
{
    static bool eval(const char& c) { return char{32} <= c && c < char{127}; }
};

struct PredicatePrintableASCII
{
    static bool eval(const char& c) { return (char(9) <= c && c <= char{13}) || (char{32} <= c && c < char{127}); }
};

template <typename PREDICATE>
bool is_cond_ascii(const char* str, std::size_t max_len)
{
    if (str == nullptr)
        return false;
    std::size_t len = 0;
    while(*str && len < max_len)
    {
        const char& c = *str++;
        if (!PREDICATE::eval(c)) { return false; }
        len++;
    }
    return len < max_len;
}

} // namespace

bool is_pure_ascii(const char* str, std::size_t max_len)
{
    return is_cond_ascii<PredicatePureASCII>(str, max_len);
}

bool is_pure_ascii(const std::string& str)
{
    return std::all_of(str.cbegin(), str.cend(), PredicatePureASCII::eval);
}

bool is_strictly_print_ascii(const char* str, std::size_t max_len)
{
     return is_cond_ascii<PredicateStrictlyPrintASCII>(str, max_len);
}

bool is_strictly_print_ascii(const std::string& str)
{
    return std::all_of(str.cbegin(), str.cend(), PredicateStrictlyPrintASCII::eval);
}

bool is_printable_ascii(const char* str, std::size_t max_len)
{
    return is_cond_ascii<PredicatePrintableASCII>(str, max_len);
}

bool is_printable_ascii(const std::string& str)
{
    return std::all_of(str.cbegin(), str.cend(), PredicatePrintableASCII::eval);
}

bool is_valid_id(Id id)
{
    constexpr unsigned int MAX_LEN = 65535;
    if (id == nullptr)
        return false;
    unsigned int len = 0;
    const char* c_ptr = id;
    if (!stdutils::ascii::isalpha(*c_ptr))      // If empty string or not starting with an alphabetical character
        return false;
    while (*c_ptr && len < MAX_LEN)
    {
        const char& c = *c_ptr++;
        if (!('a' <= c && c <= 'z') &&
            !('A' <= c && c <= 'Z') &&
            !('0' <= c && c <= '9') &&
            c != '#' &&
            c != '.' &&
            c != '-' &&
            c != '_')
        {
            return false;
        }
        len++;
    }
    return len < MAX_LEN;
}

} // namespace string

} // namespace stdutils
