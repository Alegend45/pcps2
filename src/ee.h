#pragma once

#include "common.h"

struct ee_cpu
{
    u64 r[32];
    u64 rhi[32];

    u64 lo, hi;
    u64 lo1, hi1;
    u64 ee_sa;

    u8 interrupt_mask;

    u32 cop0_count;

    union
    {
        struct
        {
            u32 interrupt_enable : 1;
            u32 exception_level : 1;
            u32 error_level : 1;
            u32 cpu_mode : 2;
            u32 reserved1 : 5;
            u32 interrupt_mask1 : 2;
            u32 bus_error_mask : 1;
            u32 reserved2 : 2;
            u32 interrupt_mask2 : 1;
            u32 enable_interrupt_enable : 1;
            u32 enable_ei_di : 1;
            u32 cache_hit_status : 1;
            u32 reserved3 : 3;
            u32 general_exception_vect_select : 1;
            u32 debug_exception_vect_select : 1;
            u32 reserved4 : 4;
            u32 cop0_usable : 1;
            u32 cop1_usable : 1;
            u32 cop2_usable : 1;
            u32 cop3_enable : 1;
        };
        u32 whole;
    } cop0_status;

    union
    {
        struct
        {
            u32 kseg0_cache_mode : 3;
            u32 reserved1 : 3;
            u32 data_cache_size : 3;
            u32 insn_cache_size : 3;
            u32 branch_prediction_enable : 1;
            u32 non_blocking_load_enable : 1;
            u32 reserved2 : 2;
            u32 data_cache_enable : 1;
            u32 insn_cache_enable : 1;
            u32 double_issue_enable : 1;
            u32 reserved3 : 9;
            u32 bus_clock_ratio : 3;
            u32 reserved4 : 1;
        };
        u32 whole;
    } cop0_config;

    union
    {
        float f;
        u32 uw;
        s32 sw;
    } fpr[32], acc;

    u32 pc, newpc;
    bool inc_pc;
    int delay_slot;
    bool branch_on;

    void* device;

    std::function<u8(void*,u32)> rb_real;
    std::function<u16(void*,u32)> rh_real;
    std::function<u32(void*,u32)> rw_real;
    std::function<u64(void*,u32)> rd_real;
    std::function<u128(void*,u32)> rq_real;
    std::function<void(void*,u32,u8)> wb_real;
    std::function<void(void*,u32,u16)> wh_real;
    std::function<void(void*,u32,u32)> ww_real;
    std::function<void(void*,u32,u64)> wd_real;
    std::function<void(void*,u32,u128)> wq_real;

    void init();
    u32 translate_addr(u32 addr);
    u8 rb(u32 addr);
    u16 rh(u32 addr);
    u32 rw(u32 addr);
    u64 rd(u32 addr);
    u128 rq(u32 addr);
    void wb(u32 addr, u8 data);
    void wh(u32 addr, u16 data);
    void ww(u32 addr, u32 data);
    void wd(u32 addr, u64 data);
    void wq(u32 addr, u128 data);
    void tick();
};