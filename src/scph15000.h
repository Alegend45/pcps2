#pragma once

#include "common.h"
#include "ee.h"
#include "iop_dma.h"
#include "iop.h"

struct scph15000
{
    u8* bios; //[0x400000]
    u8* ee_ram; //[0x2000000]
    u8* iop_ram; //[0x200000]

    ee_cpu* ee;
    iop_cpu* iop;

    iop_dma_t iop_dma;

    FILE* reg_access_log;
    FILE* serial_buffer_log;

    void init();
    void exit();

    void tick();
};

u8 scph15000_ee_rb(void* dev, u32 addr);
u32 scph15000_ee_rw(void* dev, u32 addr);
void scph15000_ee_wb(void* dev, u32 addr, u8 data);
void scph15000_ee_ww(void* dev, u32 addr, u32 data);
void scph15000_ee_wd(void* dev, u32 addr, u64 data);

u8 scph15000_iop_rb(void* dev, u32 addr);
u32 scph15000_iop_rw(void* dev, u32 addr);
void scph15000_iop_wb(void* dev, u32 addr, u8 data);
void scph15000_iop_ww(void* dev, u32 addr, u32 data);