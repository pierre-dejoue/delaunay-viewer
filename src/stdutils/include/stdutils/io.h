#pragma once

#include <exception>
#include <filesystem>
#include <functional>
#include <istream>
#include <fstream>
#include <sstream>
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
    static constexpr SeverityCode ERR   = 1;
    static constexpr SeverityCode WARN  = 2;
};

std::string_view str_severity_code(SeverityCode code);

using ErrorMessage = std::string_view;

using ErrorHandler = std::function<void(SeverityCode, ErrorMessage)>;

/**
 * File to std::basic_istream
 */
template <typename Ret, typename CharT>
using StreamParser = std::function<Ret(std::basic_istream<CharT, std::char_traits<CharT>>&, const stdutils::io::ErrorHandler&)>;

template <typename Ret, typename CharT>
Ret parse_file_generic(const StreamParser<Ret, CharT>& stream_parser, std::filesystem::path filepath, const stdutils::io::ErrorHandler& err_handler) noexcept
{
    static_assert(std::is_nothrow_default_constructible_v<Ret>);
    try
    {
        std::basic_ifstream<CharT> inputstream(filepath);
        if (inputstream.is_open())
        {
            return stream_parser(inputstream, err_handler);
        }
        else
        {
            std::stringstream oss;
            oss << "Cannot open file " << filepath;
            err_handler(stdutils::io::Severity::FATAL, oss.str());
        }
    }
    catch(const std::exception& e)
    {
        std::stringstream oss;
        oss << "Exception: " << e.what();
        err_handler(stdutils::io::Severity::EXCPT, oss.str());
    }
    return Ret();
}

} // namespace stdutils
} // namespace io
