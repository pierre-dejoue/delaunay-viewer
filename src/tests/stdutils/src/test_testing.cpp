// Copyright (c) 2024 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#include <catch_amalgamated.hpp>

#include <stdutils/testing.h>

#include <queue>
#include <vector>

TEST_CASE("copymovestring", "[testing]")
{
    stdutils::testing::CopyMoveString str("a");
    CHECK(str.constructed() == 1);
    CHECK(str.copied() == 0);
    CHECK(str.moved() == 0);
    CHECK(str.str() == "a");

    auto str_cpy = str;
    CHECK(str_cpy.constructed() == 1);
    CHECK(str_cpy.copied() == 1);
    CHECK(str_cpy.moved() == 0);
    CHECK(str_cpy.str() == "a");

    const stdutils::testing::CopyMoveString other_str("b");
    str_cpy = other_str;
    CHECK(str_cpy.constructed() == 1);
    CHECK(str_cpy.copied() == 2);
    CHECK(str_cpy.moved() == 0);
    CHECK(str_cpy.str() == "b");

    auto str_move = std::move(str);
    CHECK(str_move.constructed() == 1);
    CHECK(str_move.copied() == 0);
    CHECK(str_move.moved() == 1);
    CHECK(str_move.str() == "a");
    CHECK(str.str().empty());
}

TEST_CASE("make_container_of_copymovestring", "[testing]")
{
   auto vect = stdutils::testing::make_container_of_copymovestring<std::vector>({ "a", "b", "c", "d" });
   CHECK(vect.size() == 4);
   CHECK(vect.front().str() == "a");
   CHECK(vect.back().str() == "d");
}

TEST_CASE("make_deque_container_of_copymovestring", "[testing]")
{
   auto queue = stdutils::testing::make_deque_container_of_copymovestring<std::queue>({ "a", "b", "c", "d" });
   CHECK(queue.size() == 4);
   CHECK(queue.front().str() == "a");
}
