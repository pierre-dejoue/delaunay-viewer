#include <project.h>

std::string_view project::get_identifier()
{
    return THIS_PROJECT_NAME;
}

std::string_view project::get_short_desc()
{
    return THIS_PROJECT_DESCRIPTION;
}

std::string_view project::get_version_string()
{
    return THIS_PROEJCT_VERSION_STRING;
}
