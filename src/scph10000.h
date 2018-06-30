#pragma once

#include "common.h"
#include "ee.h"
#include "iop.h"

struct scph10000
{
    u8* bios; //[0x400000]
    u8* ee_ram; //[0x2000000]
    u8* iop_ram; //[0x200000]

    ee_cpu* ee;
    iop_cpu* iop;

    FILE* reg_access_log;
    FILE* serial_buffer_log;

    void init();
    void exit();

    void tick();
};

u32 scph10000_ee_rw(void* dev, u32 addr);
void scph10000_ee_ww(void* dev, u32 addr, u32 data);

u32 scph10000_iop_rw(void* dev, u32 addr);
void scph10000_iop_ww(void* dev, u32 addr, u32 data);