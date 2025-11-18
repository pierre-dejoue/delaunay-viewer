// Copyright (c) 2024 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <stdutils/span.h>

#include <cassert>
#include <cstddef>
#include <ostream>
#include <string_view>

/**
 * Embed binary files inside the application executable.
 *
 *  - The contents of the file can be encoded in base64 (which is convenient for small files)
 */
struct EmbeddedFile
{
    enum class Format
    {
        PNG,
        TTF,
    };

    enum class Encoding
    {
        Bytes,
        Base64,
    };

    using Data = stdutils::Span<const std::byte>;

    EmbeddedFile(Format format, const char* source, const char* b64_encoded_file_cstr)
        : format(format)
        , source(source)
        , encoding(Encoding::Base64)
        , data()
    {
        assert(source);
        assert(b64_encoded_file_cstr);
        std::string_view b64_encoded_str(b64_encoded_file_cstr);
        data = Data(reinterpret_cast<const std::byte*>(b64_encoded_str.data()), b64_encoded_str.size());
    }

    EmbeddedFile(Format format, const char* source, const Data& data)
        : format(format)
        , source(source)
        , encoding(Encoding::Bytes)
        , data(data)
    {
        assert(source);
        assert(!data.empty());
    }

    EmbeddedFile(Format format, const char* source, const std::byte* data_ptr, std::size_t data_sz)
        : EmbeddedFile(format, source, Data(data_ptr, data_sz))
    { }

    std::string_view data_as_string_view() const
    {
        return std::string_view(reinterpret_cast<const char*>(data.data()), data.size());
    }

    Format           format;
    const char*      source;
    Encoding         encoding;
    Data             data;
};

std::ostream& operator<<(std::ostream& out, const typename EmbeddedFile::Format& format);
std::ostream& operator<<(std::ostream& out, const typename EmbeddedFile::Encoding& encoding);
