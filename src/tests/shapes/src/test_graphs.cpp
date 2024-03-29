// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#include <catch_amalgamated.hpp>

#include <graphs/graph.h>
#include <graphs/graph_algos.h>

#include <cassert>
#include <cstdint>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

namespace graphs {

namespace {
namespace tests {
namespace assets {

template <typename I = std::uint32_t>
Path<I> open_path()
{
    Path<I> out;
    out.closed = false;
    out.vertices.push_back(1);
    out.vertices.push_back(2);
    out.vertices.push_back(4);
    return out;
}

template <typename I = std::uint32_t>
Path<I> closed_path()
{
    Path<I> out;
    out.closed = true;
    out.vertices.push_back(2);
    out.vertices.push_back(3);
    out.vertices.push_back(6);
    return out;
}

template <typename I = std::uint32_t>
EdgeSoup<I> edge_soup_open_path()
{
    // Derived from the test open path above
    EdgeSoup<I> out;
    out.push_back(Edge<I>(1, 2));
    out.push_back(Edge<I>(2, 4));
    return out;
}

template <typename I = std::uint32_t>
EdgeSoup<I> edge_soup_closed_path()
{
    // Derived from the test closed path above
    EdgeSoup<I> out;
    out.push_back(Edge<I>(2, 3));
    out.push_back(Edge<I>(3, 6));
    out.push_back(Edge<I>(6, 2));
    return out;
}

template <typename I = std::uint32_t>
EdgeSoup<I> edge_soup_non_manifold_letter_a()
{
    // Shaped like the capital letter A
    EdgeSoup<I> out;
    out.push_back(Edge<I>(0, 1));
    out.push_back(Edge<I>(0, 3));
    out.push_back(Edge<I>(1, 2));
    out.push_back(Edge<I>(2, 3));
    out.push_back(Edge<I>(1, 4));
    out.push_back(Edge<I>(3, 5));
    return out;
}

template <typename I = std::uint32_t>
EdgeSoup<I> edge_soup_non_manifold_lollipop()
{
    //                                   //
    //                   5               //
    //                 /   \             //
    //      0 -- 1 -- 2     4            //
    //                 \   /             //
    //                   3               //
    //                                   //
    EdgeSoup<I> out;
    out.push_back(Edge<I>(0, 1));
    out.push_back(Edge<I>(1, 2));
    out.push_back(Edge<I>(2, 3));
    out.push_back(Edge<I>(3, 4));
    out.push_back(Edge<I>(4, 5));
    out.push_back(Edge<I>(5, 2));
    return out;
}

template <typename I = std::uint32_t>
EdgeSoup<I> edge_soup_non_manifold_plus_sign()
{
    // One vertex of degree 4
    EdgeSoup<I> out;
    out.push_back(Edge<I>(2, 3));
    out.push_back(Edge<I>(2, 5));
    out.push_back(Edge<I>(2, 7));
    out.push_back(Edge<I>(2, 9));
    return out;
}

template <typename I = std::uint32_t>
EdgeSoup<I> edge_soup_non_manifold_five_star()
{
    // One vertex of degree 5
    EdgeSoup<I> out;
    out.push_back(Edge<I>(0, 5));
    out.push_back(Edge<I>(1, 5));
    out.push_back(Edge<I>(2, 5));
    out.push_back(Edge<I>(3, 5));
    out.push_back(Edge<I>(4, 5));
    return out;
}

} // namespace assets
} // namespace tests
} // namespace

TEST_CASE("Edge: basic tests", "[graphs]")
{
    using I = std::uint16_t;

    // The default ctor for Edge<I> generates an invalid/undefined edge
    Edge<I> dflt_edge;
    CHECK_FALSE(is_defined(dflt_edge));
    CHECK_FALSE(is_valid(dflt_edge));
    CHECK(dflt_edge[0] == IndexTraits<I>::undef());
    CHECK(dflt_edge[1] == IndexTraits<I>::undef());

    // Loop edge
    Edge<I> loop_edge(4, 4);
    CHECK(is_defined(loop_edge));
    CHECK(is_loop(loop_edge));
    CHECK_FALSE(is_valid(loop_edge));
    CHECK(loop_edge[0] == 4);
    CHECK(loop_edge.orig() == 4);
    CHECK(loop_edge[1] == 4);
    CHECK(loop_edge.dest() == 4);

    // Some legit edge
    Edge<I> edge(3, 5);
    CHECK(is_defined(edge));
    CHECK_FALSE(is_loop(edge));
    CHECK(is_valid(edge));
    CHECK(edge[0] == 3);
    CHECK(edge.orig() == 3);
    CHECK(edge[1] == 5);
    CHECK(edge.dest() == 5);

    CHECK_FALSE(loop_edge == edge);
    CHECK(loop_edge != edge);
}

TEST_CASE("Invalid paths: A closed path must have at least 3 vertices", "[graphs]")
{
    Path<> path;
    path.closed = true;
    CHECK(is_valid(path) == false);

    path.vertices.push_back(1);
    CHECK(is_valid(path) == false);

    path.vertices.push_back(2);
    CHECK(is_valid(path) == false);

    path.vertices.push_back(3);
    CHECK(is_valid(path) == true);
}

TEST_CASE("Simple paths", "[graphs]")
{
    Path<> path;
    path.closed = false;
    path.vertices = { 0, 1, 2, 3, 2 };
    CHECK(is_valid(path) == true);
    CHECK(is_simple(path) == false);

    path.vertices.pop_back();
    CHECK(is_valid(path) == true);
    CHECK(is_simple(path) == true);
}

TEST_CASE("Open paths", "[graphs]")
{
    const auto path = tests::assets::open_path<>();
    REQUIRE(is_valid(path));
    CHECK(nb_vertices(path) == 3);
    CHECK(nb_edges(path) == 2);

    const Path<> empty_path;
    CHECK(empty_path.closed == false);
    REQUIRE(is_valid(empty_path));
    CHECK(nb_vertices(empty_path) == 0);
    CHECK(nb_edges(empty_path) == 0);
}

TEST_CASE("Closed paths", "[graphs]")
{
    const auto path = tests::assets::closed_path<>();
    REQUIRE(is_valid(path));
    CHECK(nb_vertices(path) == 3);
    CHECK(nb_edges(path) == 3);
}

TEST_CASE("Path compact indexing", "[graphs]")
{
    using I = std::uint8_t;
    auto path = tests::assets::closed_path<I>();
    REQUIRE(is_valid(path));
    const auto [min_idx, max_idx] = minmax_indices(path);
    CHECK(min_idx == 2);
    CHECK(max_idx == 6);
    compact_indexing(path);
    CHECK(path.vertices == std::vector<I> { 0, 1, 2 });
}

TEST_CASE("Edge soup from open path", "[graphs]")
{
    const auto edge_soup = tests::assets::edge_soup_open_path();
    REQUIRE(is_valid(edge_soup) == true);
    CHECK(nb_vertices(edge_soup) == 3);
    CHECK(nb_edges(edge_soup) == 2);

    const auto path = tests::assets::open_path();
    const auto edge_soup_compare = to_edge_soup(path);
    CHECK(edge_soup == edge_soup_compare);
}

TEST_CASE("Edge soup from closed path", "[graphs]")
{
    const auto edge_soup = tests::assets::edge_soup_closed_path();
    REQUIRE(is_valid(edge_soup) == true);
    CHECK(nb_vertices(edge_soup) == 3);
    CHECK(nb_edges(edge_soup) == 3);

    const auto path = tests::assets::closed_path();
    const auto edge_soup_compare = to_edge_soup(path);
    CHECK(edge_soup == edge_soup_compare);
}

TEST_CASE("Invalid EdgeSoup: Duplicate edges", "[graphs]")
{
    EdgeSoup<> edges;
    edges.emplace_back(0, 1);
    edges.emplace_back(1, 2);
    edges.emplace_back(1, 0);

    CHECK(is_valid(edges) == false);
    CHECK(has_duplicated_edges(edges) == true);

    edges.pop_back();
    CHECK(is_valid(edges) == true);
}

TEST_CASE("Invalid EdgeSoup: Undef index", "[graphs]")
{
    using I = std::uint8_t;
    REQUIRE(IndexTraits<I>::undef() == 255);
    EdgeSoup<I> edges;
    edges.emplace_back(I{0}, I{1});
    edges.emplace_back(I{1}, I{2});
    edges.emplace_back(I{2}, I{255});

    CHECK(is_valid(edges) == false);

    edges.pop_back();
    CHECK(is_valid(edges) == true);
}

TEST_CASE("EdgeSoup: min_degree, max_degree and minmax_degree", "[graphs]")
{
    struct TestCase
    {
        MinMaxDeg  expected_min_max_degree;
        EdgeSoup<> edge_soup;
    };

    std::vector<TestCase> test_cases;
    test_cases.emplace_back(TestCase{ MinMaxDeg{1, 2}, tests::assets::edge_soup_open_path() });
    test_cases.emplace_back(TestCase{ MinMaxDeg{2, 2}, tests::assets::edge_soup_closed_path() });
    test_cases.emplace_back(TestCase{ MinMaxDeg{1, 3}, tests::assets::edge_soup_non_manifold_letter_a() });
    test_cases.emplace_back(TestCase{ MinMaxDeg{1, 3}, tests::assets::edge_soup_non_manifold_lollipop() });
    test_cases.emplace_back(TestCase{ MinMaxDeg{1, 4}, tests::assets::edge_soup_non_manifold_plus_sign() });
    test_cases.emplace_back(TestCase{ MinMaxDeg{1, 5}, tests::assets::edge_soup_non_manifold_five_star() });

    unsigned int test_loop_idx = 0;
    for (const auto& test : test_cases)
    {
        CAPTURE(test_loop_idx);
        REQUIRE(is_valid(test.edge_soup) == true);
        const auto [min, max] = minmax_degree(test.edge_soup);
        CHECK(min <= max);
        CHECK(min_degree(test.edge_soup) == min);
        CHECK(max_degree(test.edge_soup) == max);
        CHECK(min == test.expected_min_max_degree.first);
        CHECK(max == test.expected_min_max_degree.second);
        test_loop_idx++;
    }
}

TEST_CASE("EdgeSoup: minmax_indices", "[graphs]")
{
    using I = std::uint8_t;
    struct TestCase
    {
        std::pair<I, I> expected_min_max_indices;
        EdgeSoup<I> edge_soup;
    };

    std::vector<TestCase> test_cases;
    test_cases.emplace_back(TestCase{ std::make_pair<I, I>(1, 4), tests::assets::edge_soup_open_path<I>() });
    test_cases.emplace_back(TestCase{ std::make_pair<I, I>(2, 6), tests::assets::edge_soup_closed_path<I>() });
    test_cases.emplace_back(TestCase{ std::make_pair<I, I>(0, 5), tests::assets::edge_soup_non_manifold_letter_a<I>() });
    test_cases.emplace_back(TestCase{ std::make_pair<I, I>(0, 5), tests::assets::edge_soup_non_manifold_lollipop<I>() });
    test_cases.emplace_back(TestCase{ std::make_pair<I, I>(2, 9), tests::assets::edge_soup_non_manifold_plus_sign<I>() });
    test_cases.emplace_back(TestCase{ std::make_pair<I, I>(0, 5), tests::assets::edge_soup_non_manifold_five_star<I>() });

    unsigned int test_loop_idx = 0;
    for (const auto& test : test_cases)
    {
        CAPTURE(test_loop_idx);
        REQUIRE(is_valid(test.edge_soup) == true);
        const auto min_max = minmax_indices(test.edge_soup);
        CHECK(min_max.first < min_max.second);
        CHECK(min_max == test.expected_min_max_indices);
        test_loop_idx++;
    }
}

TEST_CASE("EdgeSoup: nb_vertices and compact indexing", "[graphs]")
{
    using I = std::uint16_t;
    struct TestCase
    {
        I expected_nb_vertices;
        EdgeSoup<I> edge_soup;
    };

    std::vector<TestCase> test_cases;
    test_cases.emplace_back(TestCase{ 3, tests::assets::edge_soup_open_path<I>() });
    test_cases.emplace_back(TestCase{ 3, tests::assets::edge_soup_closed_path<I>() });
    test_cases.emplace_back(TestCase{ 6, tests::assets::edge_soup_non_manifold_letter_a<I>() });
    test_cases.emplace_back(TestCase{ 6, tests::assets::edge_soup_non_manifold_lollipop<I>() });
    test_cases.emplace_back(TestCase{ 5, tests::assets::edge_soup_non_manifold_plus_sign<I>() });
    test_cases.emplace_back(TestCase{ 6, tests::assets::edge_soup_non_manifold_five_star<I>() });

    unsigned int test_loop_idx = 0;
    for (auto& test : test_cases)
    {
        CAPTURE(test_loop_idx);
        REQUIRE(is_valid(test.edge_soup) == true);
        CHECK(nb_vertices(test.edge_soup) == test.expected_nb_vertices);
        compact_indexing(test.edge_soup);
        const auto min_max = minmax_indices(test.edge_soup);
        CHECK(min_max.first == 0);
        CHECK(min_max.second + 1 == test.expected_nb_vertices);
        test_loop_idx++;
    }
}

TEST_CASE("Algo: Extract paths of empty edge soup", "[graphs]")
{
    EdgeSoup<> test;
    REQUIRE(is_valid(test) == true);
    const auto paths = extract_paths(test);
    CHECK(paths.empty());
}

TEST_CASE("Algo: Extract paths of a edge soup with a single edge", "[graphs]")
{
    using I = std::uint32_t;
    EdgeSoup<I> test;
    test.emplace_back(Edge<I>(2, 4));
    REQUIRE(is_valid(test) == true);
    const auto paths = extract_paths(test);
    REQUIRE(paths.size() == 1);
    CHECK(paths[0].closed == false);
    CHECK(paths[0].vertices.size() == 2);
}

TEST_CASE("Algo: Extract paths from edge soup", "[graphs]")
{
    struct TestCase
    {
        std::size_t expected_open_paths;
        std::size_t expected_closed_paths;
        EdgeSoup<> edge_soup;
    };

    std::vector<TestCase> test_cases;
    test_cases.emplace_back(TestCase{ 1, 0, tests::assets::edge_soup_open_path() });
    test_cases.emplace_back(TestCase{ 0, 1, tests::assets::edge_soup_closed_path() });
    test_cases.emplace_back(TestCase{ 4, 0, tests::assets::edge_soup_non_manifold_letter_a() });
    test_cases.emplace_back(TestCase{ 1, 1, tests::assets::edge_soup_non_manifold_lollipop() });
    test_cases.emplace_back(TestCase{ 4, 0, tests::assets::edge_soup_non_manifold_plus_sign() });
    test_cases.emplace_back(TestCase{ 5, 0, tests::assets::edge_soup_non_manifold_five_star() });

    unsigned int test_loop_idx = 0;
    for (const auto& test : test_cases)
    {
        CAPTURE(test_loop_idx);
        REQUIRE(is_valid(test.edge_soup) == true);
        const auto paths = extract_paths(test.edge_soup);
        const std::size_t count_closed = static_cast<std::size_t>(std::count_if(paths.cbegin(), paths.cend(), [](const auto& path) { return path.closed; }));
        const std::size_t count_open = paths.size() - count_closed;
        CHECK(count_closed == test.expected_closed_paths);
        CHECK(count_open == test.expected_open_paths);
        test_loop_idx++;
    }
}

} // namespace graphs
