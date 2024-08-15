// Copyright (c) 2024 Pierre DEJOUE
// This code is distributed under the terms of the MIT License

// Prevent a warning on std::localtime with MSVC
#define _CRT_SECURE_NO_WARNINGS

#include <stdutils/time.h>

#include <ctime>
#include <iomanip>
#include <sstream>

namespace stdutils {

std::string current_local_date_and_time()
{
    const auto now = std::time(nullptr);
    const auto* local_now = std::localtime(&now);
    if (local_now == nullptr) { return "No date"; }
    std::stringstream out;
    out << std::put_time(local_now, "%b %d %Y %H:%M:%S");
    return out.str();
}

} // namespace stdutils
