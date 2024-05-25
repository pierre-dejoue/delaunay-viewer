#pragma once

#include <string_view>

namespace project {

// Get the project identifier (it is the CMAKE_PROJECT_NAME)
std::string_view get_identifier();

// Get the short description of the project
std::string_view get_short_desc();

// Get the version of the project
std::string_view get_version_string();

} // namespace project
