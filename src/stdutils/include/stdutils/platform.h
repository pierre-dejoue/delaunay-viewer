// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <filesystem>
#include <ostream>
#include <string>

namespace stdutils {
namespace platform {

enum class OS
{
    UNKNOWN = 0,
    LINUX,
    MACOS,
    WINDOWS
};

constexpr OS os();
std::ostream& operator<<(std::ostream& out, OS os);

enum class Arch
{
    UNKNOWN = 0,
    x86,
    x86_64,
    arm64
};

constexpr Arch architecture();
std::ostream& operator<<(std::ostream& out, Arch arch);

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

constexpr bool NO_ENDL = false;

void print_os(std::ostream& out, bool endl = true);
void print_cpp_standard(std::ostream& out, bool endl = true);
void print_compiler_info(std::ostream& out, bool endl = true);
void print_architecture_info(std::ostream& out, bool endl = true);
void print_compilation_date(std::ostream& out, bool endl = true);

void print_platform_info(std::ostream& out, bool endl = true);
void print_compiler_all_info(std::ostream& out, bool endl = true);

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
