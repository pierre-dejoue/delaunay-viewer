// Copyright (c) 2024 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#include <catch_amalgamated.hpp>

#include <stdutils/locked_buffer.h>

#include <stdutils/algorithm.h>

#include <string_view>

namespace stdutils {

namespace details {

    template <typename T>
    void append_n_elements(std::size_t n, T elt, LockedBuffer<T>& buffer, std::vector<std::size_t>& offsets)
    {
        if (buffer.is_unlocked())
        {
            // If the buffer is unlocked we write data to it
            repeat_n_times(n, [&]() { buffer.buffer().emplace_back(elt); });
        }
        // In any case, we advance the index by the same amount of data
        buffer.consume(n);
        // Store the current offset
        offsets.emplace_back(buffer.consumed());
    }

    // Emulate a piece of equipement that stores a large buffer of data and some offsets
    class HW {
    public:
        HW() = default;

        // READ
        std::size_t read_nb_segments() const;
        std::string_view read_segment(std::size_t idx) const;
        const std::vector<std::size_t>& read_segment_offsets() const;

        // WRITE (The previous data & offsets are overwritten)
        void write_data(const LockedBuffer<char>& buffer, const std::vector<std::size_t>& segment_offsets);

    private:
        std::string m_data;
        std::vector<std::size_t> m_segment_offsets;
    };

    std::size_t HW::read_nb_segments() const
    {
        return !m_segment_offsets.empty() ? m_segment_offsets.size() - 1 : 0;
    }

    std::string_view HW::read_segment(std::size_t idx) const
    {
        if (idx + 1 >= m_segment_offsets.size())
            return std::string_view();
        const auto begin = m_segment_offsets[idx];
        const auto end = m_segment_offsets[idx + 1];
        if (begin >= end || end > m_data.size())
            return std::string_view();
        return std::string_view(&m_data[begin], end - begin);
    }

    const std::vector<std::size_t>& HW::read_segment_offsets() const
    {
        return m_segment_offsets;
    }

    void HW::write_data(const LockedBuffer<char>& buffer, const std::vector<std::size_t>& segment_offsets)
    {
        CHECK(buffer.is_locked());
        m_data = std::string(buffer.data(), buffer.size());
        m_segment_offsets = segment_offsets;
    }
}

TEST_CASE("A typical use case", "[locked_buffer]")
{
    details::HW hardware;
    std::vector<std::size_t> segment_offsets;

    // Right after construction the buffer is unlocked and its index is zero
    LockedBuffer<char> local_char_buffer;
    CHECK(local_char_buffer.is_unlocked());
    CHECK(local_char_buffer.consumed() == 0);

    // Frame 1: Send data to the HW

    // 1.a Prepare the data locally
    CHECK(local_char_buffer.size() == 0);
    {
        // Frame sequence
        segment_offsets.clear();
        segment_offsets.emplace_back(0);

        details::append_n_elements(3, 'a', local_char_buffer, segment_offsets);
        details::append_n_elements(4, 'b', local_char_buffer, segment_offsets);
        details::append_n_elements(3, 'c', local_char_buffer, segment_offsets);
    }
    CHECK(local_char_buffer.size() == 10);
    CHECK(local_char_buffer.consumed() == 10);

    // 1.b Lock the buffer before sending it to the hardware
    REQUIRE(local_char_buffer.index_is_aligned());
    local_char_buffer.lock();
    CHECK(local_char_buffer.is_locked());

    // 1.c. Send the data to the HW
    hardware.write_data(local_char_buffer, segment_offsets);
    CHECK(hardware.read_nb_segments() == 3);
    CHECK(hardware.read_segment(0) == "aaa");
    CHECK(hardware.read_segment(1) == "bbbb");
    CHECK(hardware.read_segment(2) == "ccc");

    // Frame 2: No need to update the data on the HW

    // 2.a Keep the buffer locked, reset the index
    local_char_buffer.index_reset();
    CHECK(local_char_buffer.is_locked());         // Can't modify the buffer data
    CHECK(local_char_buffer.consumed() == 0);

    // 2.b Run through the same code sequence for this frame.
    //     The data in the buffer remains identical and is not overwritten.
    //     We manage to rebuild the same offset vector.
    CHECK(local_char_buffer.size() == 10);
    {
        // Frame sequence
        segment_offsets.clear();
        segment_offsets.emplace_back(0);

        details::append_n_elements(3, 'a', local_char_buffer, segment_offsets);
        details::append_n_elements(4, 'b', local_char_buffer, segment_offsets);
        details::append_n_elements(3, 'c', local_char_buffer, segment_offsets);
    }
    CHECK(local_char_buffer.size() == 10);
    CHECK(segment_offsets == hardware.read_segment_offsets());

    // Frame 3: Now the data on the HW must be updated

    // 3.a Clear the buffer
    local_char_buffer.clear();
    CHECK(local_char_buffer.is_unlocked());
    CHECK(local_char_buffer.consumed() == 0);

    // 3.b Prepare the data to send to the HW
    CHECK(local_char_buffer.size() == 0);
    {
        // Frame sequence
        segment_offsets.clear();
        segment_offsets.emplace_back(0);

        details::append_n_elements(4, 'd', local_char_buffer, segment_offsets);
        details::append_n_elements(8, 'e', local_char_buffer, segment_offsets);
    }
    CHECK(local_char_buffer.size() == 12);
    CHECK(local_char_buffer.consumed() == 12);

    // 3.c Lock the buffer before sending it to the hardware
    REQUIRE(local_char_buffer.index_is_aligned());
    local_char_buffer.lock();
    CHECK(local_char_buffer.is_locked());

    // 3.d Send the data to the HW
    hardware.write_data(local_char_buffer, segment_offsets);
    CHECK(hardware.read_nb_segments() == 2);
    CHECK(hardware.read_segment(0) == "dddd");
    CHECK(hardware.read_segment(1) == "eeeeeeee");
}

TEST_CASE("Error detection", "[locked_buffer]")
{
    std::vector<std::size_t> segment_offsets;
    segment_offsets.emplace_back(0);

    LockedBuffer<char> char_buffer;
    REQUIRE(char_buffer.is_unlocked());
    REQUIRE(char_buffer.consumed() == 0);

    details::append_n_elements(3, 'a', char_buffer, segment_offsets);
    CHECK(char_buffer.index_is_aligned());
    CHECK_NOTHROW(char_buffer.lock());
    CHECK(char_buffer.is_locked());
    CHECK_THROWS(char_buffer.lock());                                           // Excpt: Buffer already locked
    CHECK_THROWS([&](){ char_buffer.buffer().emplace_back('b'); }());           // Excpt: Cannot write to a locked buffer

    CHECK_NOTHROW(char_buffer.unlock());
    CHECK_THROWS(char_buffer.unlock());                                         // Excpt: Buffer already unlocked
    CHECK_THROWS(char_buffer.consume(1));                                       // Excpt: Consume data past the buffer size
    CHECK(char_buffer.index_is_aligned());                                      // The buffer is still valid after the exception catch
    details::append_n_elements(4, 'b', char_buffer, segment_offsets);
    CHECK(char_buffer.size() == 7);
    CHECK(char_buffer.consumed() == 7);
    repeat_n_times(3, [&](){ char_buffer.buffer().emplace_back('c'); });
    CHECK_THROWS(char_buffer.lock());                                           // Excpt: Cannot lock a misaligned buffer
    char_buffer.consume(3);
    CHECK(char_buffer.index_is_aligned());
    CHECK_NOTHROW(char_buffer.lock());                                          // Now OK

    std::string_view stored_data(char_buffer.data(), char_buffer.size());
    CHECK(stored_data == "aaabbbbccc");
}

} // namespace stdutils
