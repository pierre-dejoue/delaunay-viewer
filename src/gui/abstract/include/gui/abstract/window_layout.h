// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <gui/abstract/canvas.h>

#include <algorithm>
#include <cassert>

class WindowLayout
{
public:
    // A size of 0.f or less on an axis means the window will occupy whatever is left of the workspace on that axis
    constexpr WindowLayout(ScreenUnit pos_x = 0.f, ScreenUnit pos_y = 0.f, ScreenUnit sz_x = 0.f, ScreenUnit sz_y = 0.f, ScreenUnit padding = 0.f) noexcept
        : m_position(pos_x + padding, pos_y + padding)
        , m_size(sz_x, sz_y)
        , m_padding(padding)
    {
        assert(m_position.x >= 0.f);
        assert(m_position.y >= 0.f);
        assert(m_padding >= 0.f);
    }

    constexpr WindowLayout(ScreenPos pos, ScreenSize sz, ScreenUnit padding = 0.f) noexcept
        : WindowLayout(pos.x, pos.y, sz.x, sz.y, padding)
    { }

    // Top-left corner of the window
    const ScreenPos& pos() const noexcept
    {
        return m_position;
    }

    // Bottom-right corner of the window
    ScreenPos pos_br_corner(ScreenSize workspace_sz) const
    {
        return m_position + size(workspace_sz);
    }

    ScreenSize size(ScreenSize workspace_sz) const noexcept
    {
        return ScreenSize(
            std::max(1.f, m_size.x > 0.f ? m_size.x - m_padding : workspace_sz.x - m_position.x - m_padding),
            std::max(1.f, m_size.y > 0.f ? m_size.y - m_padding : workspace_sz.y - m_position.y - m_padding)
        );
    }

private:
    ScreenPos   m_position;
    ScreenSize  m_size;
    ScreenUnit  m_padding;
};
