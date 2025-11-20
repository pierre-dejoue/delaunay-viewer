// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <stdutils/endianness.h>

#include <cstdint>
#include <filesystem>
#include <ostream>
#include <string>
#include <string_view>

namespace stdutils {
namespace platform {

/**
 * OS
 */
enum class OS
{
    UNKNOWN = 0,
    LINUX,
    MACOS,
    WINDOWS
};

constexpr OS os();
std::ostream& operator<<(std::ostream& out, OS os);

/**
 * Architecture
 */
enum class Arch
{
    UNKNOWN = 0,
    x86,
    x86_64,
    arm64
};

constexpr Arch architecture();
std::ostream& operator<<(std::ostream& out, Arch arch);
std::string_view architecture_user_friendly();              // Intel or ARM

/**
 * Endianness
 */
enum class Endianness
{
    UNKNOWN = 0,
    LE,
    BE
};

constexpr Endianness endianness();
std::ostream& operator<<(std::ostream& out, Endianness endianness);

/**
 * Compilation
 */
enum class Compiler
{
    UNKNOWN = 0,
    MSVC,
    GNU_C_CPP,
    CLANG,
    INTEL
};

constexpr Compiler compiler();
std::ostream& operator<<(std::ostream& out, Compiler compiler);
std::string compiler_version();

/**
 * Stream out information related to the platform and the compilation
 */
struct InfoFlag {
    using type = std::uint32_t;
    static constexpr type OS            = 1 << 0;
    static constexpr type ARCH          = 1 << 1;
    static constexpr type ENDIANNESS    = 1 << 2;
    static constexpr type COMPILER      = 1 << 3;
    static constexpr type CPP_STANDARD  = 1 << 4;
    static constexpr type COMPILE_DATE  = 1 << 5;
    static constexpr type ALL_INFO      =(1 << 6) - 1;
};
using InfoFlags = InfoFlag::type;

enum class InfoStyle : bool
{
    Oneliner,
    Newline
};

void stream_out_info(std::ostream& out, InfoFlags flags, InfoStyle style);

/**
 * Return the executable folder
 *
 * Typical example:
 *
 * main(int argc, char *argv[])
 * {
 *      const auto exe_folder = get_executable_folder(argc, argv);
 *      if (exe_folder.empty())
 *          return EXIT_FAILURE;
 *      return EXIT_SUCCESS;
 * }
 */
std::filesystem::path get_executable_folder(int argc, const char * const * argv);

/**
 * Return the executable path from the command line arguments
 *
 * Basically, just wrap argv[0] in a std::string
 */
std::string get_executable_raw_path(int argc, const char * const * argv);

/**
 * Return the path to a writable folder on the system to store application data.
 *
 * This is %LOCALAPPDATA% on Windows, and an equivalent path on the other platforms.
 *
 * Return an empty path in case of failure.
 */
std::filesystem::path get_local_app_data_path() noexcept;


//
//
// Implementation
//
//


constexpr OS os()
{
    // See: https://github.com/cpredef/predef/blob/master/OperatingSystems.md
    #if   defined(__linux__)
        return OS::LINUX;
    #elif defined(__APPLE__)
        return OS::MACOS;
    #elif defined(_WIN32)
        return OS::WINDOWS;
    #else
        return OS::UNKNOWN;
    #endif
}

constexpr Arch architecture()
{
    // Source: https://abseil.io/docs/cpp/platforms/macros
    #if   defined(__arm64__) || defined(__aarch64__)
        return Arch::arm64;
    #elif defined(__x86_64__) || defined(_M_X64)
        return Arch::x86_64;
    #elif defined(__i386__) || defined(_M_IX32)
        return Arch::x86;
    #else
        return Arch::UNKNOWN;
    #endif
}

constexpr Endianness endianness()
{
    if constexpr      (stdutils::endianness::native == stdutils::endianness::little)
        return Endianness::LE;
    else if constexpr (stdutils::endianness::native == stdutils::endianness::big)
        return Endianness::BE;
    else
        return Endianness::UNKNOWN;
}

constexpr Compiler compiler()
{
    // NB: Test __GNUC__ last, because that macro is sometimes defined by other compilers than the "true" GCC
    #if   defined(_MSC_VER)
        return Compiler::MSVC;
    #elif defined(__clang__)
        return Compiler::CLANG;
    #elif defined(__INTEL_COMPILER)
        return Compiler::INTEL;
    #elif defined(__GNUC__)
        return Compiler::GNU_C_CPP;
    #else
        return Compiler::UNKNOWN;
    #endif
}

} // namespace platform
} // namespace stdutils
