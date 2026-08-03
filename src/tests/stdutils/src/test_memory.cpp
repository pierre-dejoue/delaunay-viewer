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

// Structure to test memcpy/memset/memmove
struct A
{
    char a0;
    char a1;
    char a2;
    char a3;
};

TEST_CASE("Safer memcpy: Copy a single A struct", "[memory]")
{
    const std::string ref = "0123456789ABCDEF";
    std::string str = ref;

    CHECK_FALSE(stdutils::memcpy<A>(nullptr, ref.data()));
    CHECK(str == ref);

    str = ref;
    CHECK(stdutils::memcpy<A>(reinterpret_cast<A*>(str.data() + 2), ref.data() + 10) == true);
    CHECK(str == "01ABCD6789ABCDEF");
    //              ^^^^
}

TEST_CASE("Safer memcpy: General call memcpy<A>()", "[memory]")
{
    const std::string ref = "0123456789ABCDEF";
    std::string str = ref;

    CHECK_FALSE(stdutils::memcpy<A>(reinterpret_cast<A*>(str.data()), 4, ref.data() + 2, 17));
    CHECK(str == ref);

    str = ref;
    CHECK(stdutils::memcpy<A>(reinterpret_cast<A*>(str.data() + 4), 3, ref.data() + 2, 11) == true);
    CHECK(str == "012323456789ABCF");
    //            0123***********F
}

TEST_CASE("Safer memcpy: General call memcpy<char>()", "[memory]")
{
    const std::string ref = "0123456789ABCDEF";
    std::string str = ref;

    CHECK_FALSE(stdutils::memcpy<char>(str.data() + 1, 15, ref.data(), 16));
    CHECK(str == ref);

    str = ref;
    CHECK(stdutils::memcpy<char>(str.data() + 1, 15, ref.data() + 10, 6) == true);
    CHECK(str == "0ABCDEF789ABCDEF");
    //            0******789ABCDEF
}

TEST_CASE("Safer memset: Fill a single struct A", "[memory]")
{
    const std::string ref = "0123456789ABCDEF";
    std::string str = ref;

    CHECK_FALSE(stdutils::memset<A>(nullptr, 42));
    CHECK(str == ref);

    str = ref;
    CHECK(stdutils::memset<A>(reinterpret_cast<A*>(str.data() + 2), 42) == true);
    CHECK(str == "01****6789ABCDEF");
}

TEST_CASE("Safer memset: General memset<A>()", "[memory]")
{
    const std::string ref = "0123456789ABCDEF";
    std::string str = ref;

    CHECK_FALSE(stdutils::memset<A>(reinterpret_cast<A*>(str.data()), 4, 42, str.length() + 1));
    CHECK(str == ref);

    str = ref;
    CHECK(stdutils::memset<A>(reinterpret_cast<A*>(str.data()), 4, 42, 13) == true);
    CHECK(str == "*************DEF");

}

TEST_CASE("Safer memset: General memset<char>()", "[memory]")
{
    const std::string ref = "0123456789ABCDEF";
    std::string str = ref;

    CHECK_FALSE(stdutils::memset<char>(str.data() + 4, 12, 42, 13));
    CHECK(str == ref);

    str = ref;
    CHECK(stdutils::memset<char>(str.data() + 4, 12, 42, 5) == true);
    CHECK(str == "0123*****9ABCDEF");
}

TEST_CASE("Safer memmove: Move a single struct A", "[memory]")
{
    const std::string ref = "0123456789ABCDEF";
    std::string str = ref;

    CHECK_FALSE(stdutils::memmove<A>(nullptr, str.data()));
    CHECK(str == ref);

    // Overlap, src < dest
    str = ref;
    CHECK(stdutils::memmove<A>(reinterpret_cast<A*>(str.data() + 2), str.data()) == true);
    CHECK(str == "0101236789ABCDEF");
    //            vvvv
    //            01****6789ABCDEF
    //              ^^^^

    // src == dest
    str = ref;
    CHECK(stdutils::memmove<A>(reinterpret_cast<A*>(str.data() + 3), str.data() + 3) == true);
    CHECK(str == ref);

    // Overlap, src > dest
    str = ref;
    CHECK(stdutils::memmove<A>(reinterpret_cast<A*>(str.data() + 4), str.data() + 5) == true);
    CHECK(str == "0123567889ABCDEF");
    //                 vvvv
    //            0123****89ABCDEF
    //                ^^^^

}

TEST_CASE("Safer memmove: General call memmove<A>", "[memory]")
{
    const std::string ref = "0123456789ABCDEF";
    std::string str = ref;

    CHECK_FALSE(stdutils::memmove<A>(reinterpret_cast<A*>(str.data()), 4, str.data() + 2, 17));
    CHECK(str == ref);

    // Overlap, src < dest
    str = ref;
    CHECK(stdutils::memmove<A>(reinterpret_cast<A*>(str.data() + 4), 3, str.data() + 2, 11) == true);
    CHECK(str == "012323456789ABCF");
    //              vvvvvvvvvvv
    //            0123***********F
    //                ^^^^^^^^^^^

    // src == dest
    str = ref;
    CHECK(stdutils::memmove<A>(reinterpret_cast<A*>(str.data() + 2), 3, str.data() + 2, 11) == true);
    CHECK(str == ref);

    // Overlap, src > dest
    str = ref;
    CHECK(stdutils::memmove<A>(reinterpret_cast<A*>(str.data() + 2), 3, str.data() + 3, 9) == true);
    CHECK(str == "013456789ABBCDEF");
    //               vvvvvvvvv
    //            01*********BCDEF
    //              ^^^^^^^^^
}

TEST_CASE("Safer memmove: General call memmove<char>", "[memory]")
{
    const std::string ref = "0123456789ABCDEF";
    std::string str = ref;

    CHECK_FALSE(stdutils::memmove<char>(str.data() + 1, 15, str.data(), 16));
    CHECK(str == ref);

    // Overlap, src < dest
    str = ref;
    CHECK(stdutils::memmove<char>(str.data() + 2, 14, str.data() + 1, 6) == true);
    CHECK(str == "0112345689ABCDEF");
    //             vvvvvv
    //            01******89ABCDEF
    //              ^^^^^^

    // src == dest
    str = ref;
    CHECK(stdutils::memmove<char>(str.data() + 1, 15, str.data() + 1, 6) == true);
    CHECK(str == ref);

    // Overlap, src > dest
    str = ref;
    CHECK(stdutils::memmove<char>(str.data() + 6, 10, str.data() + 10, 6) == true);
    CHECK(str == "012345ABCDEFCDEF");
    //                     vvvvvv
    //            012345******CDEF
    //                  ^^^^^^
}
