// Copyright (c) 2024 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#include <catch_amalgamated.hpp>

#include <stdutils/memory.h>

#include <cstdlib>
#include <type_traits>

TEST_CASE("Trivial fixed-sized buffer", "[memory]")
{
    stdutils::FixedBuffer<int> int_buffer;

    CHECK(int_buffer.size() == 0);
    CHECK(int_buffer.empty() == true);
    CHECK(int_buffer.span().size() == 0);
    CHECK(int_buffer.span().empty() == true);
    CHECK(int_buffer.data() == nullptr);
    CHECK(int_buffer.span().data() == nullptr);

    stdutils::FixedBuffer<int> zero_int_buffer(0);

    CHECK(zero_int_buffer.size() == 0);
    CHECK(zero_int_buffer.empty() == true);
    // zero_int_buffer.data() is not necessarily the nullptr
}

TEST_CASE("Allocate an uninitialized fixed-sized buffer", "[memory]")
{
    const std::size_t N = 12;

    stdutils::FixedBuffer<int> int_buffer(N);       // No initialization

    CHECK(decltype(int_buffer)::memory_is_allocated_and_uninitialized::value == true);

    CHECK(int_buffer.size() == 12);

    int_buffer.span()[4] = 42;
    CHECK(int_buffer.data()[4] == 42);

    int_buffer.init(1);
    CHECK(int_buffer.data()[4] == 1);
}

// Non-POD class
struct C
{
    static constexpr int DEFAULT = 2;

    C() : c{DEFAULT} {}
    C(int c) : c{c} {}

    int c;
};

TEST_CASE("Allocate an initialized fixed-sized buffer", "[memory]")
{
    static_assert(std::is_default_constructible_v<C> == true);
    static_assert(std::is_trivially_default_constructible_v<C> == false);

    const std::size_t N = 12;

    stdutils::FixedBuffer<C> class_buffer(N);       // C() is called on each element

    CHECK(decltype(class_buffer)::memory_is_allocated_and_uninitialized::value == false);

    CHECK(class_buffer.size() == 12);
    CHECK(class_buffer.data()[4].c == C::DEFAULT);
}

TEST_CASE("Move fixed-sized buffers", "[memory]")
{
    const std::size_t N = 12, M = 10;

    stdutils::FixedBuffer<C> buffer_a(N);           // C() is called on each element
    buffer_a.init(C(42));
    CHECK(buffer_a.size() == N);

    stdutils::FixedBuffer<C> buffer_b(std::move(buffer_a));
    CHECK(buffer_b.size() == N);
    CHECK(buffer_b.data()[4].c == 42);
    CHECK(buffer_a.size() == 0);
    CHECK(buffer_a.empty() == true);
    CHECK(buffer_a.data() == nullptr);

    stdutils::FixedBuffer<C> buffer_c(M);
    buffer_c.init(C(7));
    CHECK(buffer_c.size() == M);
    CHECK_NOTHROW(buffer_a = std::move(buffer_c));
    CHECK(buffer_a.size() == M);
    CHECK(buffer_a.data()[4].c == 7);
    CHECK(buffer_c.size() == 0);
    CHECK(buffer_c.empty() == true);
    CHECK(buffer_c.data() == nullptr);
}

TEST_CASE("Copy fixed-sized buffers", "[memory]")
{
    const std::size_t N = 12, M = 10;

    stdutils::FixedBuffer<C> buffer_a(N);           // C() is called on each element
    buffer_a.init(C(42));
    stdutils::FixedBuffer<C> buffer_b(N);
    stdutils::FixedBuffer<C> buffer_c(M);

    CHECK(buffer_a.size() == buffer_b.size());
    CHECK(buffer_b.data()[4].c == C::DEFAULT);
    CHECK_NOTHROW(stdutils::copy(buffer_b, buffer_a));
    CHECK(buffer_b.data()[4].c == 42);              // Value was copied
    CHECK_THROWS(stdutils::copy(buffer_a, buffer_c));
    CHECK_THROWS(stdutils::copy(buffer_c, buffer_a));
}

// POD structure
struct P
{
    unsigned char p;
};

TEST_CASE("Allocate an uninitialized fixed-sized buffer of POD struct", "[memory]")
{
    static_assert(std::is_default_constructible_v<P> == true);
    static_assert(std::is_trivially_default_constructible_v<P> == true);        // Hence a POD

    const std::size_t N = 12;

    stdutils::FixedBuffer<P> pod_buffer(N);         // No initialization

    CHECK(decltype(pod_buffer)::memory_is_allocated_and_uninitialized::value == true);

    CHECK(pod_buffer.size() == 12);
    pod_buffer.data()[4].p = 42;
    CHECK(pod_buffer.data()[4].p == 42);

    pod_buffer.init(P{3});
    CHECK(pod_buffer.data()[4].p == 3);
}
