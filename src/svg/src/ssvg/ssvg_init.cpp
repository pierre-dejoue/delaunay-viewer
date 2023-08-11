#include "ssvg_init.h"

#include <bx/allocator.h>
#include <ssvg/ssvg.h>

namespace
{
    bx::DefaultAllocator s_bx_default_allocator;

    bx::AllocatorI* s_initialize_ssvg_lib()
    {
        ssvg::initLib(&s_bx_default_allocator);
        return &s_bx_default_allocator;
    }
}

bool svg::io::initialize_ssvg_lib()
{
    static bx::AllocatorI* bx_allocator = s_initialize_ssvg_lib();
    return bx_allocator != nullptr;
}
