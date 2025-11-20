// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#include <stdutils/io.h>

#include <stdutils/span.h>

#include <array>
#include <cassert>
#include <cstddef>
#include <ostream>
#include <string>
#include <sstream>

namespace stdutils {
namespace io {

namespace {

constexpr unsigned int SEV_CODE_MAX_IDX = 7;
std::array<std::string_view, SEV_CODE_MAX_IDX + 1> str_severity_code_lookup {
    "FATAL",
    "EXCPT",
    "ZERO",
    "ERROR",
    "WARNING",
    "INFO",
    "TRACE",
    "UNKNOWN"
};

} // namespace

std::string_view str_severity_code(SeverityCode code) noexcept
{
    unsigned int code_idx = static_cast<unsigned int>(code - Severity::FATAL);
    if (code_idx > SEV_CODE_MAX_IDX) { code_idx = SEV_CODE_MAX_IDX; }
    assert(code_idx < str_severity_code_lookup.size());
    return str_severity_code_lookup[code_idx];
}

std::string dump_txt_file_to_memory(const std::filesystem::path& filepath, const stdutils::io::ErrorHandler& err_handler) noexcept
{
    return open_and_parse_txt_file<std::string, char>(filepath, [](std::istream& in, const stdutils::io::ErrorHandler&) {
        std::ostringstream out;
        in >> out.rdbuf();
        return out.str();
    }, err_handler);
}

FixedByteBuffer dump_bin_file_to_memory(const std::filesystem::path& filepath, const stdutils::io::ErrorHandler& err_handler) noexcept
{
    using BufferT = stdutils::FixedByteBuffer;
    return open_and_parse_bin_file<BufferT, char>(filepath, [](std::istream& in, const stdutils::io::ErrorHandler&) {
        in.seekg(0, std::ios_base::end);    // Jump to the end of the file
        const auto size = in.tellg();
        in.seekg(0, std::ios_base::beg);    // Rewind
        BufferT buf(static_cast<std::size_t>(size));
        in.read(reinterpret_cast<char*>(buf.data()), size);
        return buf;
    }, err_handler);
}

bool dump_to_txt_file(const std::filesystem::path& filepath, std::string_view txt, const stdutils::io::ErrorHandler& err_handler) noexcept
{
    const auto verbatim = [](std::ostream& out, const std::string_view& txt, const stdutils::io::ErrorHandler&) { out << txt; };
    return save_txt_file<std::string_view, char>(filepath, verbatim, txt, err_handler);
}

bool dump_to_bin_file(const std::filesystem::path& filepath, const std::byte* buffer, std::size_t sz, const stdutils::io::ErrorHandler& err_handler) noexcept
{
    if (!buffer)
    {
        if (err_handler) { err_handler(stdutils::io::Severity::ERR, "dump_to_bin_file: null buffer pointer"); }
        return false;
    }
    using byte_buffer_view = stdutils::Span<const std::byte>;
    const auto verbatim = [](std::ostream& out, const byte_buffer_view& buf, const stdutils::io::ErrorHandler&) {
        out.write(reinterpret_cast<const char*>(buf.data()), static_cast<std::streamsize>(buf.size()));
    };
    return save_bin_file<byte_buffer_view, char>(filepath, verbatim, byte_buffer_view(buffer, sz), err_handler);
}

} // namespace io
} // namespace stdutils
