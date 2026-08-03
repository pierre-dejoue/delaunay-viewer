// Copyright (c) 2024 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <stdutils/macros.h>
#include <stdutils/span.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <type_traits>
#include <stdexcept>

namespace stdutils {

/**
 * Auto-free C pointer. Useful when interfacing with code that makes use of C-style malloc.
 */
template <typename T>
struct AutoFreePtr
{
    AutoFreePtr(T* ptr = nullptr) : ptr(ptr) { }
    ~AutoFreePtr() { if (ptr != nullptr) { std::free(ptr); } }
    T* ptr;
};

template <typename T>
struct AutoFreeCPtr
{
    AutoFreeCPtr(const T* ptr = nullptr) : ptr(ptr) { }
    ~AutoFreeCPtr() { if (ptr != nullptr) { std::free(ptr); } }
    const T* ptr;
};


/**
 * Versions of memcpy with sanity checks
 *
 * - Check for null pointers
 * - Check if dest == src, in which case do nothing and return true
 * - Check that nb_bytes <= max_dest_sz * sizeof(T)
 * - In case of error, copy nothing and return false
 */
template <typename T>
bool memcpy(T* dest, const void* src);                                                   // Copy exactly sizeof(T) bytes

template <typename T>
bool memcpy(T* dest, std::size_t max_dest_sz, const void* src, std::size_t nb_bytes);    // Copy at most max_dest_sz * sizeof(T) bytes


/**
 * Versions of memset with sanity checks
 */
template <typename T>
bool memset(T* dest, int ch);                                                            // Set at exactly sizeof(T) bytes

template <typename T>
bool memset(T* dest, std::size_t max_dest_sz, int ch, std::size_t nb_bytes);             // Set at most max_dest_sz * sizeof(T) bytes


/**
 * Versions of memmove with sanity checks
 *
 * - Check for null pointers
 * - Check if dest == src, in which case do nothing and return true
 * - Check that nb_bytes <= max_dest_sz * sizeof(T)
 * - In case of error, move nothing and return false
 */
template <typename T>
bool memmove(T* dest, const void* src);                                                   // Move exactly sizeof(T) bytes

template <typename T>
bool memmove(T* dest, std::size_t max_dest_sz, const void* src, std::size_t nb_bytes);    // Move at most max_dest_sz * sizeof(T) bytes


/**
 * Local pointer to a local
 *
 * The pointer is reset (to nullptr) when this object leaves scope.
 * Useful when a pointer to a scoped variable is used.
 */
template <typename T>
class ScopedPtrToLocal {
public:
    ScopedPtrToLocal(T** local_ptr_ptr, T& local_var)
        : m_ptr_ptr(local_ptr_ptr)
    {
        assert(m_ptr_ptr);
        *m_ptr_ptr = &local_var;
    }

    ~ScopedPtrToLocal()
    {
        *m_ptr_ptr = nullptr;
    }
private:
    T** m_ptr_ptr;
};

/**
 * A buffer which size is fixed and set at runtime
 *
 * When calling FixedBuffer(n):
 *  - If T is a class type the buffer is initialized with the default constructor.
 *  - Otherwise, the buffer is uninitialized.
 *
 * That buffer is mostly intended for POD types, to benefit from the fact that the memory is allocated and not initialized.
 * It will work with class types as well, but with a penalty cost on some of the constructors (see TODO notes)
 */
template <typename T>
class FixedBuffer {
public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;

    // Trait
    struct memory_is_allocated_and_uninitialized
        : std::is_trivially_default_constructible<T>
    { };

    FixedBuffer() noexcept;
    explicit FixedBuffer(std::size_t n);
    FixedBuffer(std::size_t n, const T& v);                 // Force initialization of the memory

    // Non-copyable. Copy of the buffer is meant to be explicit.
    FixedBuffer(const FixedBuffer&) = delete;
    FixedBuffer& operator=(const FixedBuffer& ) = delete;

    // Moveable
    FixedBuffer(FixedBuffer&& other) noexcept;
    FixedBuffer& operator=(FixedBuffer&& other) noexcept;

    std::size_t size() const noexcept;
    bool empty() const noexcept;

    void init(const T& v);

    pointer data() noexcept;
    const_pointer data() const noexcept;

    Span<T> span() noexcept;
    Span<const T> cspan() const noexcept;

    // For compatibility with other containers. Does nothing.
    void clear() noexcept;

    // Truncate to a smaller size. The same memory buffer is kept and no reallocation occurs.
    void truncate(std::size_t n);

    // Comparison
    bool operator==(const FixedBuffer<T>& other) const;
    bool operator!=(const FixedBuffer<T>& other) const;

private:
    std::unique_ptr<T[]>    m_buffer;
    std::size_t             m_size;
};

// Explicit copy of data into a FixedBuffer. The sizes of both buffers MUST match.
template <typename T>
void copy(FixedBuffer<T>& dest, const Span<const T>& src);
template <typename T>
void copy(FixedBuffer<T>& dest, const FixedBuffer<T>& src);


// A raw memory buffer
using FixedByteBuffer = FixedBuffer<std::byte>;


//
//
// Implementation
//
//


template <typename T>
bool memcpy(T* dest, const void* src)
{
    if (dest == nullptr || src == nullptr)
    {
        return false;
    }
    if (static_cast<void*>(dest) == src)
    {
        return true;
    }
    IGNORE_RETURN std::memcpy(static_cast<void*>(dest), src, sizeof(T));
    return true;
}

template <typename T>
bool memcpy(T* dest, std::size_t max_dest_sz, const void* src, std::size_t nb_bytes)
{
    if (dest == nullptr || src == nullptr)
    {
        return false;
    }
    if (nb_bytes > max_dest_sz * sizeof(T))
    {
        return false;
    }
    if (static_cast<void*>(dest) == src)
    {
        return true;
    }
    IGNORE_RETURN std::memcpy(static_cast<void*>(dest), src, nb_bytes);
    return true;
}

template <typename T>
bool memset(T* dest, int ch)
{
    if (dest == nullptr)
    {
        return false;
    }
    IGNORE_RETURN std::memset(static_cast<void*>(dest), ch, sizeof(T));
    return true;
}

template <typename T>
bool memset(T* dest, std::size_t max_dest_sz, int ch, std::size_t nb_bytes)
{
    if (dest == nullptr)
    {
        return false;
    }
    if (nb_bytes > max_dest_sz * sizeof(T))
    {
        return false;
    }
    IGNORE_RETURN std::memset(static_cast<void*>(dest), ch, nb_bytes);
    return true;
}

template <typename T>
bool memmove(T* dest, const void* src)
{
    if (dest == nullptr || src == nullptr)
    {
        return false;
    }
    if (static_cast<void*>(dest) == src)
    {
        return true;
    }
    IGNORE_RETURN std::memmove(static_cast<void*>(dest), src, sizeof(T));
    return true;
}

template <typename T>
bool memmove(T* dest, std::size_t max_dest_sz, const void* src, std::size_t nb_bytes)
{
    if (dest == nullptr || src == nullptr)
    {
        return false;
    }
    if (nb_bytes > max_dest_sz * sizeof(T))
    {
        return false;
    }
    if (static_cast<void*>(dest) == src)
    {
        return true;
    }
    IGNORE_RETURN std::memmove(static_cast<void*>(dest), src, nb_bytes);
    return true;
}

template <typename T>
FixedBuffer<T>::FixedBuffer() noexcept
    : m_buffer(std::unique_ptr<T[]>())
    , m_size(0)
{ }

template <typename T>
FixedBuffer<T>::FixedBuffer(std::size_t n)
    : m_buffer(std::unique_ptr<T[]>(new T[n]))
    , m_size(n)
{ }

// TODO: For non-POD types we get a double initialization cost
template <typename T>
FixedBuffer<T>::FixedBuffer(std::size_t n, const T& v)
    : m_buffer(std::unique_ptr<T[]>(new T[n]))
    , m_size(n)
{
    init(v);
}

template <typename T>
FixedBuffer<T>::FixedBuffer(FixedBuffer&& other) noexcept
    : m_buffer(std::move(other.m_buffer))
    , m_size(other.m_size)
{
    other.m_size = 0;
    assert(!other.m_buffer);
}

template <typename T>
FixedBuffer<T>& FixedBuffer<T>::operator=(FixedBuffer&& other) noexcept
{
    m_buffer = std::move(other.m_buffer);
    m_size = other.m_size;
    other.m_size = 0;
    assert(!other.m_buffer);
    return *this;
}

template <typename T>
std::size_t FixedBuffer<T>::size() const noexcept
{
    return m_size;
}

template <typename T>
bool FixedBuffer<T>::empty() const noexcept
{
    return m_size == 0;
}

template <typename T>
void FixedBuffer<T>::init(const T& v)
{
    std::fill_n(m_buffer.get(), m_size, v);
}

template <typename T>
typename FixedBuffer<T>::pointer FixedBuffer<T>::data() noexcept
{
    return m_buffer.get();
}

template <typename T>
typename FixedBuffer<T>::const_pointer FixedBuffer<T>::data() const noexcept
{
    return m_buffer.get();
}

template <typename T>
Span<T> FixedBuffer<T>::span() noexcept
{
    T* ptr = m_buffer.get();
    return ptr ? Span<T>(ptr, m_size) : Span<T>();
}

template <typename T>
Span<const T> FixedBuffer<T>::cspan() const noexcept
{
    const T* ptr = m_buffer.get();
    return ptr ? Span<const T>(ptr, m_size) : Span<const T>();
}

template <typename T>
void FixedBuffer<T>::clear() noexcept
{
    // For compatibility with other containers. Does nothing.
}

template <typename T>
void FixedBuffer<T>::truncate(std::size_t n)
{
    if (n > m_size)
    {
        throw std::invalid_argument("Truncation size greater than the current size");
    }
    m_size = n;
}

template <typename T>
bool FixedBuffer<T>::operator==(const FixedBuffer<T>& other) const
{
    const Span<const T> this_span = cspan();
    const Span<const T> other_span = other.cspan();
    return std::equal(this_span.begin(), this_span.end(), other_span.begin(), other_span.end());
}

template <typename T>
bool FixedBuffer<T>::operator!=(const FixedBuffer<T>& other) const
{
    return !(*this == other);
}

template <typename T>
void copy(FixedBuffer<T>& dest, const Span<const T>& src)
{
    if (dest.size() != src.size())
    {
        throw std::invalid_argument("Buffers size mismatch");
    }
    if (dest.size() > 0)
    {
        assert(src.data());
        assert(dest.data());
        IGNORE_RETURN memcpy<T>(dest.data(), dest.size(), src.data(), dest.size() * sizeof(T));
    }
}

template <typename T>
void copy(FixedBuffer<T>& dest, const FixedBuffer<T>& src)
{
    copy<T>(dest, src.cspan());
}

} // namespace stdutils
