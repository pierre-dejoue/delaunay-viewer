// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#include <catch_amalgamated.hpp>

#include <stdutils/algorithm.h>
#include <stdutils/span.h>
#include <stdutils/testing.h>

#include <cstdint>
#include <functional>
#include <list>
#include <locale>
#include <queue>
#include <stack>
#include <string>
#include <vector>

TEST_CASE("stdutils::clamp", "[algorithm]")
{
    bool clamped = false;
    int r = 0;
    {
        r = stdutils::clamp<int>(-129, INT8_MIN, INT8_MAX, clamped);
        CHECK(r == INT8_MIN);
        CHECK(clamped);
        r = stdutils::clamp<int>(-128, INT8_MIN, INT8_MAX, clamped);
        CHECK(r == INT8_MIN);
        CHECK(!clamped);
        r = stdutils::clamp<int>(42, INT8_MIN, INT8_MAX, clamped);
        CHECK(r == 42);
        CHECK(!clamped);
        r = stdutils::clamp<int>(127, INT8_MIN, INT8_MAX, clamped);
        CHECK(r == 127);
        CHECK(!clamped);
        r = stdutils::clamp<int>(128, INT8_MIN, INT8_MAX, clamped);
        CHECK(r == 127);
        CHECK(clamped);
    }
    {
        r = stdutils::clamp<int>(-129, INT8_MIN, INT8_MAX, std::less<int>(), clamped);
        CHECK(r == INT8_MIN);
        CHECK(clamped);
        r = stdutils::clamp<int>(-128, INT8_MIN, INT8_MAX, std::less<int>(), clamped);
        CHECK(r == INT8_MIN);
        CHECK(!clamped);
        r = stdutils::clamp<int>(42, INT8_MIN, INT8_MAX, std::less<int>(), clamped);
        CHECK(r == 42);
        CHECK(!clamped);
    }
}

TEST_CASE("stdutils::erase on a vector", "[algorithm]")
{
    std::vector<int> vect { 0, 1, 2, 6, 0, 0, 5, 0, 1 };
    const auto erased = stdutils::erase(vect, 0);
    CHECK(erased == 4);
    CHECK(vect == std::vector<int> { 1, 2, 6, 5, 1 });
}

TEST_CASE("stdutils::erase_if on a string", "[algorithm]")
{
    std::string test = "Confederation Mondiale des Activites Subaquatiques";
    const std::size_t test_original_sz = test.size();
    const std::locale c_loc("C");
    const auto erased = stdutils::erase_if(test, [&c_loc](const auto& c) { return !std::isupper(c, c_loc); });
    CHECK(erased + 4 == test_original_sz);
    CHECK(test == "CMAS");
}

TEST_CASE("index_find", "[algorithm]")
{
    std::vector<int> vect { 3, 1, 4, 1, 5, 9, 2, 6 };
    const int look_for = 9;
    const auto index = stdutils::index_find(vect, std::size_t{0}, vect.size(), look_for);
    CHECK(index == 5);
}

TEST_CASE("index_find_if", "[algorithm]")
{
    std::string test = "STL Utils";
    const std::locale c_loc("C");
    const auto look_for = [&c_loc](const auto& c) { return std::islower(c, c_loc); };       // Look for the first lower case char
    const auto index = stdutils::index_find_if(test, std::size_t{0}, test.size(), look_for);
    CHECK(index == 5);
}

TEST_CASE("Use index_find with stdutils::Span", "[algorithm]")
{
    std::vector<int> vect { 3, 1, 4, 1, 5, 9, 2, 6 };
    const int look_for = 9;
    {
        // Start search at index 0
        const auto index = stdutils::index_find(stdutils::Span<int>(&vect[0], 4u), 0, 4, look_for);
        CHECK(index == 4);      // Not found
    }
    {
        // Start search at index 4
        const auto index = stdutils::index_find(stdutils::Span<int>(&vect[4], 4u), 0, 4, look_for);
        CHECK(index == 1);      // Index in the original vector: 4 + 1
    }
}

TEST_CASE("pop_back", "[algorithm]")
{
    auto list = stdutils::testing::make_container_of_copymovestring<std::list>({ "a", "b", "c", "d" });
    std::vector<stdutils::testing::CopyMoveString> vect;
    while (!list.empty())
    {
        auto elt = stdutils::pop_back(list);
        CHECK(elt.constructed() == 1);
        CHECK(elt.copied() == 0);
        CHECK(elt.moved() == 1);
        vect.emplace_back(std::move(elt));
    }
    CHECK(list.empty());
    CHECK(vect.size() == 4);
    CHECK(vect == stdutils::testing::make_container_of_copymovestring<std::vector>({ "d", "c", "b", "a" }));
}

TEST_CASE("pop_front", "[algorithm]")
{
    auto list = stdutils::testing::make_container_of_copymovestring<std::list>({ "a", "b", "c", "d" });
    std::vector<stdutils::testing::CopyMoveString> vect;
    while (!list.empty())
    {
        auto elt = stdutils::pop_front(list);
        CHECK(elt.constructed() == 1);
        CHECK(elt.copied() == 0);
        CHECK(elt.moved() == 1);
        vect.emplace_back(std::move(elt));
    }
    CHECK(list.empty());
    CHECK(vect.size() == 4);
    CHECK(vect == stdutils::testing::make_container_of_copymovestring<std::vector>({ "a", "b", "c", "d" }));
}

TEST_CASE("pop from a stack", "[algorithm]")
{
    auto stack = stdutils::testing::make_deque_container_of_copymovestring<std::stack>({ "a", "b", "c", "d" });
    std::vector<stdutils::testing::CopyMoveString> vect;
    while (!stack.empty())
    {
        auto elt = stdutils::pop(stack);
        CHECK(elt.constructed() == 1);
        CHECK(elt.copied() == 0);
        CHECK(elt.moved() == 1);
        vect.emplace_back(std::move(elt));
    }
    CHECK(stack.empty());
    CHECK(vect.size() == 4);
    CHECK(vect == stdutils::testing::make_container_of_copymovestring<std::vector>({ "d", "c", "b", "a" }));
}

TEST_CASE("pop from a queue", "[algorithm]")
{
    auto queue = stdutils::testing::make_deque_container_of_copymovestring<std::queue>({ "a", "b", "c", "d" });
    std::vector<stdutils::testing::CopyMoveString> vect;
    while (!queue.empty())
    {
        auto elt = stdutils::pop(queue);
        CHECK(elt.constructed() == 1);
        CHECK(elt.copied() == 0);
        CHECK(elt.moved() == 1);
        vect.emplace_back(std::move(elt));
    }
    CHECK(queue.empty());
    CHECK(vect.size() == 4);
    CHECK(vect == stdutils::testing::make_container_of_copymovestring<std::vector>({ "a", "b", "c", "d" }));
}

TEST_CASE("clear a queue using pop", "[algorithm]")
{
    std::queue<std::string> queue({ "a", "b", "c", "d" });
    REQUIRE(!queue.empty());
    CHECK(queue.size() == 4);
    stdutils::clear_using_pop(queue);
    CHECK(queue.empty());
}

TEST_CASE("merge", "[algorithm]")
{
    std::string dst = "aeiou";
    std::string src = "bcdfghjklmnpqrstvwxyz";
    stdutils::merge(dst, src, std::less<char>());
    CHECK(dst == "abcdefghijklmnopqrstuvwxyz");
}
