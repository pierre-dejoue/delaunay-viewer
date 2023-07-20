
#include <stdutils/io.h>

#include <cassert>


namespace stdutils
{
namespace io
{

std::string_view str_severity_code(SeverityCode code)
{
    if (code == Severity::FATAL)
    {
        return "FATAL";
    }
    else if (code == Severity::EXCPT)
    {
        return "EXCPT";
    }
    else if (code == Severity::WARNING)
    {
        return "WARNING";
    }
    else if (code == Severity::ERROR)
    {
        return "ERROR";
    }
    else
    {
        return "UNKNOWN";
    }
}

}
}
