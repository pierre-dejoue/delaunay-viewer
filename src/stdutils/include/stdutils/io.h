#pragma once

#include <functional>
#include <string_view>


namespace stdutils
{
namespace io
{

/**
 * IO error handling
 *
 *
 * Severity code:
 *   Negative: Non-recoverable, output should be ignored.
 *   Positive: Output is usable despite the errors.
 */
using SeverityCode = int;
struct Severity
{
    static constexpr SeverityCode FATAL = -2;
    static constexpr SeverityCode EXCPT = -1;
    static constexpr SeverityCode ERROR = 1;
    static constexpr SeverityCode WARNING = 2;
};

std::string_view str_severity_code(SeverityCode code);

using ErrorMessage = std::string_view;

using ErrorHandler = std::function<void(SeverityCode, ErrorMessage)>;

}
}