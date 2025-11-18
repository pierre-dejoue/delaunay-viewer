// Copyright (c) 2024 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#include "embedded_file.h"

#include <cassert>
#include <ostream>

std::ostream& operator<<(std::ostream& out, const EmbeddedFile::Format& format)
{
    using Format = EmbeddedFile::Format;
    switch(format)
    {
        case Format::PNG:
            out << "PNG";
            break;

        case Format::TTF:
            out << "TTF";
            break;

        default:
            assert(0);
            break;
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, const EmbeddedFile::Encoding& encoding)
{
    using Encoding = EmbeddedFile::Encoding;
    switch(encoding)
    {
        case Encoding::Bytes:
            out << "Bytes";
            break;

        case Encoding::Base64:
            out << "Base64";
            break;

        default:
            assert(0);
            break;
    }
    return out;
}
