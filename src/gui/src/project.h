#pragma once

#include <string_view>

namespace project {

// Get the project identifier
std::string_view get_identifier();

// Get the project name
std::string_view get_name();

// Get the version of the project
std::string_view get_version_string();

} // namespace project
