#pragma once

#include "common.h"

struct iop_cpu
{
    u32 r[32];

    u32 lo, hi;

    u32 pc, newpc;
    bool inc_pc;
    int delay_slot;
    bool branch_on;

    void* device;

    std::function<u8(void*,u32)> rb_real;
    std::function<u16(void*,u32)> rh_real;
    std::function<u32(void*,u32)> rw_real;
    std::function<void(void*,u32,u8)> wb_real;
    std::function<void(void*,u32,u16)> wh_real;
    std::function<void(void*,u32,u32)> ww_real;

    void init();
    u32 translate_addr(u32 addr);
    u8 rb(u32 addr);
    u16 rh(u32 addr);
    u32 rw(u32 addr);
    void wb(u32 addr, u8 data);
    void wh(u32 addr, u16 data);
    void ww(u32 addr, u32 data);
    void tick();
};