#include "scph10000.h"

void scph10000::init()
{
    //TODO
    bios = (u8*)malloc(0x400000);
    memset(bios, 0, 0x400000);
    ee_ram = (u8*)malloc(0x2000000);
    memset(ee_ram, 0, 0x2000000);
    iop_ram = (u8*)malloc(0x200000);
    memset(iop_ram, 0, 0x200000);

    iop_dma.device = this;
    iop_dma.rw = scph10000_iop_rw;
    iop_dma.ww = scph10000_iop_ww;
    iop_dma.init();

    reg_access_log = fopen("reglog.txt","w+");
}

void scph10000::exit()
{
    free(bios);
    if(serial_buffer_log) fclose(serial_buffer_log);
    if(reg_access_log) fclose(reg_access_log);
}

void scph10000::tick()
{
}

u8 scph10000_ee_rb(void* dev, u32 addr)
{
    scph10000* device = (scph10000*) dev;
    addr &= 0x1fffffff;
    if(addr < 0x10000000)
    {
        return device->ee_ram[addr & 0x1ffffff];
    }
    else if(addr >= 0x10000000 && addr < 0x10010000)
    {
        fprintf(device->reg_access_log, "[EE] Unknown register read %08x pc %08x\n", addr, device->ee->pc);
    }
    else if(addr >= 0x12000000 && addr < 0x12010000)
    {
        fprintf(device->reg_access_log, "[EE] Unknown register read %08x pc %08x\n", addr, device->ee->pc);
    }
    else if(addr >= 0x1c000000 && addr < 0x1c200000)
    {
        return device->iop_ram[addr & 0x1fffff];
    }
    else if(addr >= 0x1fc00000 && addr < 0x20000000)
    {
        return device->bios[addr & 0x3fffff];
    }
    else printf("[EE] Unknown address %08x!\n", addr);
    return 0;
}

u32 scph10000_ee_rw(void* dev, u32 addr)
{
    scph10000* device = (scph10000*) dev;
    addr &= 0x1fffffff;
    if(addr < 0x10000000)
    {
        return device->ee_ram[(addr+0) & 0x1ffffff] | (device->ee_ram[(addr+1) & 0x1ffffff] << 8)
        | (device->ee_ram[(addr+2) & 0x1ffffff] << 16) | (device->ee_ram[(addr+3) & 0x1ffffff] << 24);
    }
    else if(addr >= 0x10000000 && addr < 0x10010000)
    {
        fprintf(device->reg_access_log, "[EE] Unknown register read %08x pc %08x\n", addr, device->ee->pc);
    }
    else if(addr >= 0x12000000 && addr < 0x12010000)
    {
        fprintf(device->reg_access_log, "[EE] Unknown register read %08x pc %08x\n", addr, device->ee->pc);
    }
    else if(addr >= 0x1c000000 && addr < 0x1c200000)
    {
        return device->iop_ram[(addr+0) & 0x1fffff] | (device->iop_ram[(addr+1) & 0x1fffff] << 8)
        | (device->iop_ram[(addr+2) & 0x1fffff] << 16) | (device->iop_ram[(addr+3) & 0x1fffff] << 24);
    }
    else if(addr >= 0x1fc00000 && addr < 0x20000000)
    {
        return device->bios[(addr+0) & 0x3fffff] | (device->bios[(addr+1) & 0x3fffff] << 8)
        | (device->bios[(addr+2) & 0x3fffff] << 16) | (device->bios[(addr+3) & 0x3fffff] << 24);
    }
    else printf("[EE] Unknown address %08x!\n", addr);
    return 0;
}

void scph10000_ee_wb(void* dev, u32 addr, u8 data)
{
    scph10000* device = (scph10000*) dev;
    addr &= 0x1fffffff;
    if(addr < 0x10000000)
    {
        device->ee_ram[addr & 0x1ffffff] = data;
    }
    else if(addr >= 0x10000000 && addr < 0x10010000)
    {
        fprintf(device->reg_access_log, "[EE] Unknown register write %08x data %02x pc %08x\n", addr, data, device->ee->pc);
    }
    else if(addr >= 0x12000000 && addr < 0x12010000)
    {
        fprintf(device->reg_access_log, "[EE] Unknown register write %08x data %02x pc %08x\n", addr, data, device->ee->pc);
    }
    else if(addr >= 0x1c000000 && addr < 0x1c200000)
    {
        device->iop_ram[addr & 0x1fffff] = data;
    }
    else printf("[EE] Unknown address %08x data %02x!\n", addr, data);
}

void scph10000_ee_ww(void* dev, u32 addr, u32 data)
{
    scph10000* device = (scph10000*) dev;
    addr &= 0x1fffffff;
    if(addr < 0x10000000)
    {
        device->ee_ram[(addr+0) & 0x1ffffff] = (data >> 0) & 0xff;
        device->ee_ram[(addr+1) & 0x1ffffff] = (data >> 8) & 0xff;
        device->ee_ram[(addr+2) & 0x1ffffff] = (data >> 16) & 0xff;
        device->ee_ram[(addr+3) & 0x1ffffff] = (data >> 24) & 0xff;
    }
    else if(addr >= 0x10000000 && addr < 0x10010000)
    {
        fprintf(device->reg_access_log, "[EE] Unknown register write %08x data %08x pc %08x\n", addr, data, device->ee->pc);
    }
    else if(addr >= 0x12000000 && addr < 0x12010000)
    {
        fprintf(device->reg_access_log, "[EE] Unknown register write %08x data %08x pc %08x\n", addr, data, device->ee->pc);
    }
    else if(addr >= 0x1c000000 && addr < 0x1c200000)
    {
        device->iop_ram[(addr+0) & 0x1fffff] = (data >> 0) & 0xff;
        device->iop_ram[(addr+1) & 0x1fffff] = (data >> 8) & 0xff;
        device->iop_ram[(addr+2) & 0x1fffff] = (data >> 16) & 0xff;
        device->iop_ram[(addr+3) & 0x1fffff] = (data >> 24) & 0xff;
    }
    else printf("[EE] Unknown address %08x data %08x!\n", addr, data);
}

void scph10000_ee_wd(void* dev, u32 addr, u64 data)
{
    scph10000* device = (scph10000*) dev;
    addr &= 0x1fffffff;
    if(addr < 0x10000000)
    {
        device->ee_ram[(addr+0) & 0x1ffffff] = (data >> 0) & 0xff;
        device->ee_ram[(addr+1) & 0x1ffffff] = (data >> 8) & 0xff;
        device->ee_ram[(addr+2) & 0x1ffffff] = (data >> 16) & 0xff;
        device->ee_ram[(addr+3) & 0x1ffffff] = (data >> 24) & 0xff;
        device->ee_ram[(addr+4) & 0x1ffffff] = (data >> 32) & 0xff;
        device->ee_ram[(addr+5) & 0x1ffffff] = (data >> 40) & 0xff;
        device->ee_ram[(addr+6) & 0x1ffffff] = (data >> 48) & 0xff;
        device->ee_ram[(addr+7) & 0x1ffffff] = (data >> 56) & 0xff;
    }
    else if(addr >= 0x10000000 && addr < 0x10010000)
    {
        if(addr < 0x10002000 || addr >= 0x10003000) scph10000_ee_ww(dev, addr, (u32)data);
        else fprintf(device->reg_access_log, "[EE] Unknown register write %08x data %016x pc %08x\n", addr, data, device->ee->pc);
    }
    else if(addr >= 0x12000000 && addr < 0x12010000)
    {
        fprintf(device->reg_access_log, "[EE] Unknown register write %08x data %016x pc %08x\n", addr, data, device->ee->pc);
    }
    else if(addr >= 0x1c000000 && addr < 0x1c200000)
    {
        device->iop_ram[(addr+0) & 0x1fffff] = (data >> 0) & 0xff;
        device->iop_ram[(addr+1) & 0x1fffff] = (data >> 8) & 0xff;
        device->iop_ram[(addr+2) & 0x1fffff] = (data >> 16) & 0xff;
        device->iop_ram[(addr+3) & 0x1fffff] = (data >> 24) & 0xff;
        device->iop_ram[(addr+4) & 0x1fffff] = (data >> 32) & 0xff;
        device->iop_ram[(addr+5) & 0x1fffff] = (data >> 40) & 0xff;
        device->iop_ram[(addr+6) & 0x1fffff] = (data >> 48) & 0xff;
        device->iop_ram[(addr+7) & 0x1fffff] = (data >> 56) & 0xff;
    }
    else printf("[EE] Unknown address %08x data %016x!\n", addr, data);
}

u8 scph10000_iop_rb(void* dev, u32 addr)
{
    scph10000* device = (scph10000*) dev;
    if(addr < 0x00200000)
    {
        return device->iop_ram[addr & 0x1fffff];
    }
    else if(addr >= 0x1d000000 && addr < 0x1f810000)
    {
        fprintf(device->reg_access_log, "[IOP] Unknown register read %08x pc %08x\n", addr, device->iop->pc);
    }
    else if(addr >= 0x1fc00000 && addr < 0x20000000)
    {
        return device->bios[addr & 0x3fffff];
    }
    else printf("[IOP] Unknown address %08x!\n", addr);
    return 0;
}

u32 scph10000_iop_rw(void* dev, u32 addr)
{
    scph10000* device = (scph10000*) dev;
    if(addr < 0x00200000)
    {
        return device->iop_ram[(addr+0) & 0x1fffff] | (device->iop_ram[(addr+1) & 0x1fffff] << 8)
        | (device->iop_ram[(addr+2) & 0x1fffff] << 16) | (device->iop_ram[(addr+3) & 0x1fffff] << 24);
    }
    else if(addr >= 0x1d000000 && addr < 0x1f810000)
    {
        if(addr >= 0x1f801080 && addr < 0x1f801100)
        {
            fprintf(device->reg_access_log, "[IOP] DMA write %08x pc %08x\n", addr, device->iop->pc);
            return device->iop_dma.dma_rw(addr);
        }
        else if(addr >= 0x1f801500 && addr < 0x1f801580)
        {
            fprintf(device->reg_access_log, "[IOP] DMA write %08x pc %08x\n", addr, device->iop->pc);
            return device->iop_dma.dma_rw(addr);
        }
        else fprintf(device->reg_access_log, "[IOP] Unknown register read %08x pc %08x\n", addr, device->iop->pc);
    }
    else if(addr >= 0x1fc00000 && addr < 0x20000000)
    {
        return device->bios[(addr+0) & 0x3fffff] | (device->bios[(addr+1) & 0x3fffff] << 8)
        | (device->bios[(addr+2) & 0x3fffff] << 16) | (device->bios[(addr+3) & 0x3fffff] << 24);
    }
    else printf("[IOP] Unknown address %08x!\n", addr);
    return 0;
}

void scph10000_iop_wb(void* dev, u32 addr, u8 data)
{
    scph10000* device = (scph10000*) dev;
    if(addr < 0x00200000)
    {
        device->iop_ram[addr & 0x1fffff] = data;
    }
    else if(addr >= 0x1d000000 && addr < 0x1f810000)
    {
        fprintf(device->reg_access_log, "[IOP] Unknown register write %08x data %02x pc %08x\n", addr, data, device->iop->pc);
    }
    else printf("[IOP] Unknown address %08x data %02x!\n", addr, data);
}

void scph10000_iop_ww(void* dev, u32 addr, u32 data)
{
    scph10000* device = (scph10000*) dev;
    if(addr < 0x00200000)
    {
        device->iop_ram[(addr+0) & 0x1fffff] = (data >> 0) & 0xff;
        device->iop_ram[(addr+1) & 0x1fffff] = (data >> 8) & 0xff;
        device->iop_ram[(addr+2) & 0x1fffff] = (data >> 16) & 0xff;
        device->iop_ram[(addr+3) & 0x1fffff] = (data >> 24) & 0xff;
    }
    else if(addr >= 0x1d000000 && addr < 0x1f810000)
    {
        if(addr >= 0x1f801080 && addr < 0x1f801100)
        {
            fprintf(device->reg_access_log, "[IOP] DMA write %08x data %08x pc %08x\n", addr, data, device->iop->pc);
            device->iop_dma.dma_ww(addr, data);
        }
        else if(addr >= 0x1f801500 && addr < 0x1f801580)
        {
            fprintf(device->reg_access_log, "[IOP] DMA write %08x data %08x pc %08x\n", addr, data, device->iop->pc);
            device->iop_dma.dma_ww(addr, data);
        }
        else fprintf(device->reg_access_log, "[IOP] Unknown register write %08x data %08x pc %08x\n", addr, data, device->iop->pc);
    }
    else printf("[IOP] Unknown address %08x data %08x!\n", addr, data);
}