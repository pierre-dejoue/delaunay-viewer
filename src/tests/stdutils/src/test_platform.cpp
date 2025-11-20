// Copyright (c) 2024 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#include <catch_amalgamated.hpp>

#include <stdutils/platform.h>

#include <iostream>
#include <sstream>

namespace {

stdutils::platform::Endianness endianness_at_runtime()
{
    auto uint32_mem = std::unique_ptr<std::uint32_t[]>(new std::uint32_t[1]);
    uint32_mem[0] = 0x0A0B0C0D;
    const auto* uint8_ptr = reinterpret_cast<const std::uint8_t*>(uint32_mem.get());
    if (*uint8_ptr == 0x0A)
    {
        return stdutils::platform::Endianness::BE;
    }
    else if (*uint8_ptr == 0x0D)
    {
        return stdutils::platform::Endianness::LE;
    }
    return stdutils::platform::Endianness::UNKNOWN;
}

} // namespace

TEST_CASE("The platform and architecture are known", "[platform]")
{
    constexpr auto os = stdutils::platform::os();
    CHECK(os != stdutils::platform::OS::UNKNOWN);

    constexpr auto arch = stdutils::platform::architecture();
    CHECK(arch != stdutils::platform::Arch::UNKNOWN);

    constexpr auto compiler = stdutils::platform::compiler();
    CHECK(compiler != stdutils::platform::Compiler::UNKNOWN);
}

TEST_CASE("Check architecture consistency", "[platform]")
{
    constexpr auto arch = stdutils::platform::architecture();
    REQUIRE(arch != stdutils::platform::Arch::UNKNOWN);
    constexpr auto sizeof_void_ptr = sizeof(void*);
    constexpr bool is_64bit_platform = (arch == stdutils::platform::Arch::arm64) | (arch == stdutils::platform::Arch::x86_64);
    CHECK(sizeof_void_ptr == (is_64bit_platform ? 8 : 4));
}

TEST_CASE("User friendly name for the architecture (Intel or ARM)", "[platform]")
{
    const auto arch_str = stdutils::platform::architecture_user_friendly();
    CHECK((arch_str == "Intel" || arch_str == "ARM"));
}

TEST_CASE("Endianness", "[platform]")
{
    constexpr auto endianness = stdutils::platform::endianness();
    CHECK(endianness == endianness_at_runtime());
}

TEST_CASE("System path for temp files", "[platform]")
{
    const std::filesystem::path temp_path = std::filesystem::temp_directory_path();
    if (temp_path.empty())
    {
        FAIL("Empty path for temps files");
    }
    std::cerr << ">>> Path for temp files: " << temp_path << " <<<" << std::endl;
}

TEST_CASE("System paths", "[platform]")
{
    namespace fs = std::filesystem;
    const fs::path local_app_data_path = stdutils::platform::get_local_app_data_path();
    if (local_app_data_path.empty())
    {
        FAIL("Empty local application data path");
    }
    std::cerr << ">>> Local application data path: " << local_app_data_path << " <<<" << std::endl;
}
