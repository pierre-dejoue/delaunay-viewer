// Copyright (c) 2024 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#include <catch_amalgamated.hpp>

#include <stdutils/platform.h>

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
