#include <version.h>


std::string_view get_version_string()
{
    return std::string_view(DELAUNAY_VIEWER_VERSION_STRING);
}

