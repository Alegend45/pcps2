#pragma once

#include "common.h"

struct ee_cpu
{
    u64 r[32];
    u64 rhi[32];

    u32 pc, newpc;
    bool inc_pc;
    int delay_slot;
    bool branch_on;

    void* device;

    std::function<u32(void*,u32)> rw_real;
    std::function<void(void*,u32,u32)> ww_real;

    void init();
    u32 rw(u32 addr);
    void ww(u32 addr, u32 data);
    void tick();
};