// Copyright (c) 2025 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include "embedded_file.h"

enum class FontFileRef
{
    TITILLIUM_WEB,
};

EmbeddedFile get_font_file(FontFileRef font_file);
