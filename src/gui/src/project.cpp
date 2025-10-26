// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#include <project.h>

#include "project_defines.h"

namespace project {

std::string_view get_identifier()
{
    return THIS_PROJECT_ID;
}

std::string_view get_name()
{
    return THIS_PROJECT_NAME;
}

std::string_view get_version_string()
{
#ifndef THIS_PROJECT_OFFICIAL_RELEASE
    return THIS_PROJECT_VERSION_STRING "-dev";
#else
    return THIS_PROJECT_VERSION_STRING;
#endif
}

std::string_view get_owner()
{
    return THIS_PROJECT_OWNER;
}

std::string_view get_year_range()
{
    return THIS_PROJECT_YEAR_RANGE;
}

std::string_view get_short_license()
{
    return THIS_PROJECT_SHORT_LICENSE;
}

std::string_view get_short_copyright()
{
    return THIS_PROJECT_SHORT_COPYRIGHT;
}

std::string_view get_copyright()
{
    return THIS_PROJECT_COPYRIGHT;
}

std::string_view get_website()
{
    return THIS_PROJECT_WEBSITE;
}

} // namespace project
