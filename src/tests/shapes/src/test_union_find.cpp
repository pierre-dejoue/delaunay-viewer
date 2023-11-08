// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#include <catch_amalgamated.hpp>

#include <graphs/union_find.h>

namespace graphs
{

TEST_CASE("Union Find: simple test", "[graphs]")
{
    using I = UnionFind<>::index;

    UnionFind<I> set(10);

    CHECK(set.size() == 10);

    for (I i = 0; i < 10; i++)
    {
        I j{0};
        I sz{0};
        CHECK_NOTHROW(j = set.find(i));
        CHECK_NOTHROW(sz = set.subset_size(i));
        CHECK(i == j);
        CHECK(sz == 1);
    }

    set.subset_union(2, 3);
    CHECK(set.subset_size(2) == 2);
    CHECK(set.subset_size(3) == 2);
    CHECK(set.find(2) == set.find(3));

    set.subset_union(4, 5);
    set.subset_union(3, 4);
    CHECK(set.subset_size(2) == 4);
    CHECK(set.subset_size(3) == 4);
    CHECK(set.subset_size(4) == 4);
    CHECK(set.subset_size(5) == 4);
    CHECK(set.find(2) == set.find(3));
    CHECK(set.find(3) == set.find(4));
    CHECK(set.find(4) == set.find(5));
}

} // namespace graphs
