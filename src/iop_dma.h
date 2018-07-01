#pragma once

#include "common.h"

typedef u32(*rw_func)(void*, u32);
typedef void(*ww_func)(void*, u32, u32);

struct iop_dma_t
{
    struct
    {
        u32 addr;
        u16 word_count;
        u16 block_size;
        union
        {
            struct
            {
                u32 direction : 1;
                u32 reserved1 : 7;
                u32 unknown1 : 1;
                u32 sync_mode : 8;
                u32 reserved2 : 8;
                u32 busy : 1;
                u32 reserved3 : 7;
                u32 unknown2 : 1;
                u32 reserved4 : 1;
            };
            u32 whole;
        } control;
        u32 tag_addr;

        bool tag_end;
    } channels[16];

    struct
    {
        u8 priorities[16];
        bool enable[16];
    } dpcr;

    struct
    {
        u8 status[2];
        u8 mask[2];
        bool master_int_enable[2];
    } dicr;

    void* device;

    rw_func rw;
    ww_func ww;

    void init();
    void tick();
    u32 dma_rw(u32 addr);
    void dma_ww(u32 addr, u32 data);
};