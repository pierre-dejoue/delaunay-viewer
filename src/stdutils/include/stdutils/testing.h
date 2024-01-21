// Copyright (c) 2024 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <cstdlib>
#include <string>

// Utilities for testing
namespace stdutils
{
namespace testing
{

struct CopyMoveString
{
    CopyMoveString() { m_ctor_counter++; }
    CopyMoveString(const std::string& str) : m_str(str) { m_ctor_counter++; }
    CopyMoveString(std::string&& str) : m_str(str) { m_ctor_counter++; }
    CopyMoveString(const CopyMoveString& o) : m_str(o.m_str) { m_copy_ctor_counter++; }
    CopyMoveString& operator=(const CopyMoveString& o) { m_str = o.m_str;  m_copy_assign_counter++; return *this; }
    CopyMoveString(CopyMoveString&& o) : m_str(std::move(o.m_str)) { m_move_ctor_counter++; }
    CopyMoveString& operator=(CopyMoveString&& o) { m_str = std::move(o.m_str); m_move_assign_counter++; return *this; }

    bool operator==(const CopyMoveString& o) const { return m_str == o.m_str; }

    const std::string& str() const { return m_str; }

    std::size_t constructed() const { return m_ctor_counter + m_copy_ctor_counter + m_move_ctor_counter; }
    std::size_t copied() const { return m_copy_ctor_counter + m_copy_assign_counter; }
    std::size_t moved() const { return m_move_ctor_counter + m_move_assign_counter; }

    std::string m_str{};
    std::size_t m_ctor_counter = 0;
    std::size_t m_copy_ctor_counter = 0;
    std::size_t m_copy_assign_counter = 0;
    std::size_t m_move_ctor_counter = 0;
    std::size_t m_move_assign_counter = 0;
};

template <template <typename...> class C>
C<CopyMoveString> make_container_of_copymovestring(std::initializer_list<std::string> il)
{
    C<CopyMoveString> result;
    for (auto& s : il)
        result.emplace_back(std::move(s));
    return result;
}

template <template <typename...> class C>
C<CopyMoveString> make_deque_container_of_copymovestring(std::initializer_list<std::string> il)
{
    C<CopyMoveString> result;
    for (auto& s : il)
        result.emplace(std::move(s));
    return result;
}

} // namespace testing
} // namespace stdutils
