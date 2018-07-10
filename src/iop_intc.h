#pragma once

#include "common.h"
#include "iop.h"

struct iop_intc_t
{
    u32 irq_status, irq_mask;
    bool irq_enable;

    iop_cpu* iop;
    void* device;

    void init();
    void tick();
    u32 rw(u32 addr);
    void ww(u32 addr, u32 data);
};