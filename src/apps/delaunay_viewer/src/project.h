// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <string_view>

namespace project {

// Get the project identifier
std::string_view get_identifier();

// Get the project name
std::string_view get_name();

// Get the version of the project
std::string_view get_version_string();

// Get the owner of the project
std::string_view get_owner();

// Get a range comprising of the year the project started and (optionaly) the year it was last maintained
// The format of the output string is: YYYY[-YYYY]
std::string_view get_year_range();

// Get the short license description (e.g. "MIT License", or "All rights reserved", etc.)
std::string_view get_short_license();

// Get the short copyright notice (Copyright (c) <year> <vendor>)
std::string_view get_short_copyright();

// Get the project copyright notice (short copyright + short license)
std::string_view get_copyright();

// Get the project website
std::string_view get_website();

} // namespace project
