// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#include <catch_amalgamated.hpp>

#include <stdutils/algorithm.h>
#include <stdutils/span.h>

#include <locale>
#include <string>
#include <vector>

TEST_CASE("std::erase on a vector", "[iterator]")
{
    std::vector<int> vect { 0, 1, 2, 6, 0, 0, 5, 0, 1 };
    const auto erased = stdutils::erase(vect, 0);
    CHECK(erased == 4);
    CHECK(vect == std::vector<int> { 1, 2, 6, 5, 1 });
}

TEST_CASE("std::erase_if on a string", "[iterator]")
{
    std::string test = "Confederation Mondiale des Activites Subaquatiques";
    const std::size_t test_original_sz = test.size();
    const std::locale c_loc("C");
    const auto erased = stdutils::erase_if(test, [&c_loc](const auto& c) { return !std::isupper(c, c_loc); });
    CHECK(erased + 4 == test_original_sz);
    CHECK(test == "CMAS");
}

TEST_CASE("index_find", "[iterator]")
{
    std::vector<int> vect { 3, 1, 4, 1, 5, 9, 2, 6 };
    const int look_for = 9;
    const auto index = stdutils::index_find(vect, std::size_t{0}, vect.size(), look_for);
    CHECK(index == 5);
}

TEST_CASE("index_find_if", "[iterator]")
{
    std::string test = "STL Utils";
    const std::locale c_loc("C");
    const auto look_for = [&c_loc](const auto& c) { return std::islower(c, c_loc); };       // Look for the first lower case char
    const auto index = stdutils::index_find_if(test, std::size_t{0}, test.size(), look_for);
    CHECK(index == 5);
}

TEST_CASE("Use index_find with stdutils::span", "[iterator]")
{
    std::vector<int> vect { 3, 1, 4, 1, 5, 9, 2, 6 };
    const int look_for = 9;
    {
        // Start search at index 0
        const auto index = stdutils::index_find(stdutils::span<int>(&vect[0], 4u), 0, 4, look_for);
        CHECK(index == 4);      // Not found
    }
    {
        // Start search at index 4
        const auto index = stdutils::index_find(stdutils::span<int>(&vect[4], 4u), 0, 4, look_for);
        CHECK(index == 1);      // Index in the original vector: 4 + 1
    }
}
