#pragma once

#include "common.h"

enum iop_exceptions
{
    exception_interrupt = 0x01,
    exception_addrload = 0x04,
    exception_addrstore = 0x05,
    exception_businst = 0x06,
    exception_busdata = 0x07,
    exception_syscall = 0x08,
    exception_break = 0x09,
    exception_invalidop = 0x0a,
    exception_badcop = 0x0b,
    exception_overflow = 0x0c
};

struct iop_cpu
{
    u32 r[32];

    u32 lo, hi;

    u32 cop0_count;

    union
    {
        struct
        {
            u32 current_int_enable : 1;
            u32 current_in_user_mode : 1;
            u32 previous_int_enable : 1;
            u32 previous_in_user_mode : 1;
            u32 old_int_enable : 1;
            u32 old_in_user_mode : 1;
            u32 reserved1 : 2;
            u32 interrupt_mask : 8;
            u32 isolate_cache : 1;
            u32 swap_caches : 1;
            u32 cache_parity_zero : 1;
            u32 data_cache_last_load_hit : 1;
            u32 cache_parity_error : 1;
            u32 tlb_shutdown : 1;
            u32 boot_except_vectors_rom : 1;
            u32 reserved2 : 5;
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
            u32 reserved1 : 2;
            u32 exception_code : 5;
            u32 reserved2 : 1;
            u32 interrupt_pending : 8;
            u32 reserved3 : 12;
            u32 cop_num : 2;
            u32 reserved4 : 1;
            u32 in_branch_delay : 1;
        };
        u32 whole;
    } cop0_cause;

    u32 cop0_epc;

    u32 pc, newpc;
    bool inc_pc;
    int delay_slot;
    bool branch_on;

    int cycle;
    void* device;
    FILE* iop_debug_log;

    std::function<u8(void*,u32)> rb_real;
    std::function<u16(void*,u32)> rh_real;
    std::function<u32(void*,u32)> rw_real;
    std::function<void(void*,u32,u8)> wb_real;
    std::function<void(void*,u32,u16)> wh_real;
    std::function<void(void*,u32,u32)> ww_real;

    void init();
    void exit();
    u32 translate_addr(u32 addr);
    u8 rb(u32 addr);
    u16 rh(u32 addr);
    u32 rw(u32 addr);
    void wb(u32 addr, u8 data);
    void wh(u32 addr, u16 data);
    void ww(u32 addr, u32 data);
    void tick();
    void generate_exception(int exception);
    void irq_modify(int num, bool level);
};