// Copyright (c) 2021 Pierre DEJOUE
// This code is distributed under the terms of the MIT License

// Prevent a warning on std::localtime with MSVC
#define _CRT_SECURE_NO_WARNINGS

#include <stdutils/string.h>

#include <algorithm>
#include <cctype>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace stdutils {
namespace string {

std::string tolower(const std::string& in)
{
    std::string out(in);
    // Conversion from char to unsigned char is important
    // See STL doc of std::tolower, https://en.cppreference.com/w/cpp/string/byte/tolower
    std::transform(in.cbegin(), in.cend(), out.begin(), [](char c) { return static_cast<char>(std::tolower(static_cast<unsigned char>(c))); });
    return out;
}

std::string toupper(const std::string& in)
{
    std::string out(in);
    // Conversion from char to unsigned char is important
    // See STL doc of std::toupper, https://en.cppreference.com/w/cpp/string/byte/toupper
    std::transform(in.cbegin(), in.cend(), out.begin(), [](char c) { return static_cast<char>(std::toupper(static_cast<unsigned char>(c))); });
    return out;
}

std::string capitalize(const std::string& in)
{
    std::string out = tolower(in);
    // Conversion from char to unsigned char is important
    // See STL doc of std::toupper, https://en.cppreference.com/w/cpp/string/byte/toupper
    if (!out.empty()) { out.front() = static_cast<char>(std::toupper(static_cast<unsigned char>(out.front()))); }
    return out;
}

std::string current_local_date_and_time()
{
    const auto now = std::time(nullptr);
    const auto* local_now = std::localtime(&now);
    if (local_now == nullptr) { return "No date"; }
    std::stringstream out;
    out << std::put_time(local_now, "%b %d %Y %H:%M:%S");
    return out.str();
}

} // namespace string
} // namespace stdutils
