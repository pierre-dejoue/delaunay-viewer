// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#include "ssvg_init.h"

#include <ssvg/ssvg.h>

namespace {

class SSVGInit {
public:
    SSVGInit()
        : initialized{false}
    {
        ssvg::initLib();
        initialized = true;
    }

    ~SSVGInit()
    {
        ssvg::shutdownLib();
    }

    bool is_initialized() const noexcept { return initialized; }

private:
    bool initialized;
};

} // namespace

bool svg::io::initialize_ssvg_lib()
{
    static const SSVGInit ssvg_init;
    return ssvg_init.is_initialized();
}
