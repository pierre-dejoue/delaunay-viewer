// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License

// Prevent a warning on std::getenv with MSVC
#define _CRT_SECURE_NO_WARNINGS

#include <stdutils/platform.h>

#include <array>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <sstream>
#include <string_view>

namespace fs = std::filesystem;

#if defined(__APPLE__)
#include "platform_macos.h"
#endif

namespace stdutils {
namespace platform {

std::ostream& operator<<(std::ostream& out, OS os)
{
    switch(os)
    {
        case OS::UNKNOWN:
            out << "Unknown_os";
            break;

        case OS::LINUX:
            out << "Linux";
            break;

        case OS::MACOS:
            out << "macOS";
            break;

        case OS::WINDOWS:
            out << "Windows";
            break;

        default:
            assert(0);
            out << "Unknown_enum";
            break;
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, Arch arch)
{
    switch (arch)
    {
        case Arch::UNKNOWN:
            out << "Unknown_arch";
            break;

        case Arch::x86:
            out << "x86";
            break;

        case Arch::x86_64:
            out << "x86_64";
            break;

        case Arch::arm64:
            out << "arm64";
            break;

        default:
            assert(0);
            out << "Unknown_enum";
            break;
    }
    return out;
}

std::string_view architecture_user_friendly()
{
    constexpr auto arch = architecture();
    if constexpr (arch == Arch::x86 || arch == Arch::x86_64)
        return "Intel";
    else if constexpr (arch == Arch::arm64)
        return "ARM";
    else
    {
        assert(0);
        return "Unknown_enum";
    }
}

std::ostream& operator<<(std::ostream& out, Endianness endianness)
{
    switch (endianness)
    {
        case Endianness::UNKNOWN:
            out << "Unknown_endianness";
            break;

        case Endianness::LE:
            out << "LE";
            break;

        case Endianness::BE:
            out << "BE";
            break;

        default:
            assert(0);
            out << "Unknown_enum";
            break;
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, Compiler compiler)
{
    switch (compiler)
    {
        case Compiler::UNKNOWN:
            out << "Unknown_compiler";
            break;

        case Compiler::MSVC:
            out << "MSVC";
            break;

        case Compiler::GNU_C_CPP:
            out << "GNU GCC/G++";
            break;

        case Compiler::CLANG:
            out << "Clang";
            break;

        case Compiler::INTEL:
            out << "Intel C++";
            break;

        default:
            assert(0);
            out << "Unknown_enum";
            break;
    }
    return out;
}

std::string compiler_version()
{
    std::stringstream out;
    switch (compiler())
    {
        case Compiler::UNKNOWN:
            out << "Unknown_compiler_version";
            break;

        case Compiler::MSVC:
            #if defined(_MSC_VER)
            out << _MSC_FULL_VER;
            #endif
            break;

        case Compiler::GNU_C_CPP:
            #if defined(__GNUC__)
            out << __GNUC__ << "." << __GNUC_MINOR__ << "." << __GNUC_PATCHLEVEL__;
            #endif
            break;

        case Compiler::CLANG:
            #if defined(__clang__)
            out << __clang_major__ << "." << __clang_minor__ << "." << __clang_patchlevel__;
            #endif
            break;

        case Compiler::INTEL:
            #if defined(__INTEL_COMPILER)
            out << __INTEL_COMPILER;
            #endif
            break;

        default:
            assert(0);
            out << "Unknown_enum";
            break;
    }
    return out.str();
}

void stream_out_info(std::ostream& out, InfoFlags flags, InfoStyle style)
{
    const std::string_view sep = style == InfoStyle::Oneliner ? "; " : "\n";
    bool first = true;
    InfoFlag::type mask = 1;
    for (unsigned int shift = 0; shift < 32; shift++)
    {
        const InfoFlag::type flag_selection = flags & mask;
        mask <<= 1;
        if (flag_selection == 0) { continue; }
        if (!first) { out << sep; }
        first = false;
        switch (flag_selection)
        {
            case InfoFlag::OS:
                out << "OS: " << os();
                break;

            case InfoFlag::ARCH:
                out << "Arch: " << architecture();
                break;

            case InfoFlag::ENDIANNESS:
                out << "Endianness: " << endianness();
                break;

            case InfoFlag::COMPILER:
                out << "Compiler: " << compiler();
                if constexpr (compiler() != Compiler::UNKNOWN)
                    out << " " << compiler_version();
                break;

            case InfoFlag::CPP_STANDARD:
                out << "C++ Standard: " << __cplusplus;
                break;

            case InfoFlag::COMPILE_DATE:
                out << "Compilation date: " << __DATE__ << " " << __TIME__;
                break;

            default:
                assert(0);
                // unknown flag
                break;
        }
    }
    if (style == InfoStyle::Newline) { out << std::endl; }
}

fs::path get_executable_folder(int argc, const char * const * argv)
{
    return (argc > 0 && argv != nullptr && argv[0] != nullptr) ? std::filesystem::canonical(fs::path(argv[0]).parent_path()) : fs::path();
}

std::string get_executable_raw_path(int argc, const char * const * argv)
{
    return (argc > 0 && argv != nullptr && argv[0] != nullptr) ? std::string(argv[0]) : std::string();
}

fs::path get_local_app_data_path() noexcept
{
    try
    {
#if   defined(__linux__)
        // XDG Base Directory Specification: https://specifications.freedesktop.org/basedir-spec/latest/
        const char* env_path = std::getenv("HOME");
        return env_path != nullptr ? fs::path(env_path) / fs::path(".local/share") : fs::path();

#elif defined(__APPLE__)
        std::array<char, 1024> buffer;
        const bool success = __macos__get_local_app_data_path(buffer.data(), 1023);
        return success ? fs::path(buffer.data()) : fs::path();

#elif defined(_WIN32)
        const char* env_path = std::getenv("LOCALAPPDATA");
        return env_path != nullptr ? fs::path(env_path) : fs::path();

#else
        return fs::path();

#endif
    }
    catch (...)
    {
        return fs::path();
    }
}

} // namespace platform
} // namespace stdutils
