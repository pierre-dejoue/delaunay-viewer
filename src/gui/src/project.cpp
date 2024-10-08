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

} // namespace project
