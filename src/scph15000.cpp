#include "scph15000.h"

void scph15000::init()
{
    //TODO
    bios = (u8*)calloc(0x400000, 1);
    ee_ram = (u8*)calloc(0x2000000, 1);
    iop_ram = (u8*)calloc(0x200000, 1);
    ee_sp_ram = (u8*)calloc(0x4000, 1);

    mch_drd = 0;
    mch_ricm = 0;
    rdram_sdevid = 0;

    iop_dma.device = this;
    iop_dma.rw = scph15000_iop_rw;
    iop_dma.ww = scph15000_iop_ww;
    iop_dma.init();

    reg_access_log = fopen("reglog.txt","w+");
    ee_debug_log = fopen("ee_debug_console.txt","w+");
}

void scph15000::exit()
{
    free(bios);
    free(ee_ram);
    free(iop_ram);
    free(ee_sp_ram);
    if(ee_debug_log) fclose(ee_debug_log);
    if(reg_access_log) fclose(reg_access_log);
}

void scph15000::tick()
{
}

u8 scph15000_ee_rb(void* dev, u32 addr)
{
    scph15000* device = (scph15000*) dev;
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
    else if(addr >= 0x70000000 && addr < 0x70004000)
    {
        return device->ee_sp_ram[addr & 0x3fff];
    }
    else fprintf(device->reg_access_log, "[EE] Unknown address %08x!\n", addr);
    return 0;
}

u16 scph15000_ee_rh(void* dev, u32 addr)
{
    scph15000* device = (scph15000*) dev;
    if(addr < 0x10000000)
    {
        return *(u16*)(device->ee_ram + (addr & 0x1ffffff));
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
        return *(u16*)(device->iop_ram + (addr & 0x1fffff));
    }
    else if(addr >= 0x1fc00000 && addr < 0x20000000)
    {
        return *(u16*)(device->bios + (addr & 0x3fffff));
    }
    else if(addr >= 0x70000000 && addr < 0x70004000)
    {
        return *(u16*)(device->ee_sp_ram + (addr & 0x3ffe));
    }
    else fprintf(device->reg_access_log, "[EE] Unknown address %08x!\n", addr);
    return 0;
}

u32 scph15000_ee_rw(void* dev, u32 addr)
{
    scph15000* device = (scph15000*) dev;
    if(addr < 0x10000000)
    {
        return *(u32*)(device->ee_ram + (addr & 0x1ffffff));
    }
    else if(addr >= 0x10000000 && addr < 0x10010000)
    {
        if(addr == 0x1000f440)
        {
            fprintf(device->reg_access_log, "[EE] MCH DRD read %08x pc %08x\n", addr, device->ee->pc);
            u16 cmd = (device->mch_ricm >> 16) & 0xfff;
            u8 subcmd = (device->mch_ricm >> 6) & 0xf;
            u8 lsb5 = device->mch_ricm & 0x1f;
            if(subcmd == 0)
            {
                switch(cmd)
                {
                    case 0x021:
                    {
                        if(device->rdram_sdevid < 2)
                        {
                            device->rdram_sdevid++;
                            return 0x1f;
                        }
                        break;
                    }
                    case 0x40:
                    {
                        return lsb5;
                        break;
                    }
                }
            }
        }
        else fprintf(device->reg_access_log, "[EE] Unknown register read %08x pc %08x\n", addr, device->ee->pc);
    }
    else if(addr >= 0x12000000 && addr < 0x12010000)
    {
        fprintf(device->reg_access_log, "[EE] Unknown register read %08x pc %08x\n", addr, device->ee->pc);
    }
    else if(addr >= 0x1c000000 && addr < 0x1c200000)
    {
        return *(u32*)(device->iop_ram + (addr & 0x1fffff));
    }
    else if(addr >= 0x1fc00000 && addr < 0x20000000)
    {
        return *(u32*)(device->bios + (addr & 0x3fffff));
    }
    else if(addr >= 0x70000000 && addr < 0x70004000)
    {
        return *(u32*)(device->ee_sp_ram + (addr & 0x3ffc));
    }
    else fprintf(device->reg_access_log, "[EE] Unknown address %08x!\n", addr);
    return 0;
}

u64 scph15000_ee_rd(void* dev, u32 addr)
{
    scph15000* device = (scph15000*) dev;
    if(addr < 0x10000000)
    {
        return *(u64*)(device->ee_ram + (addr & 0x1ffffff));
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
        return *(u64*)(device->iop_ram + (addr & 0x1fffff));
    }
    else if(addr >= 0x1fc00000 && addr < 0x20000000)
    {
        return *(u64*)(device->bios + (addr & 0x3fffff));
    }
    else if(addr >= 0x70000000 && addr < 0x70004000)
    {
        return *(u64*)(device->ee_sp_ram + (addr & 0x3ff8));
    }
    else fprintf(device->reg_access_log, "[EE] Unknown address %08x!\n", addr);
    return 0;
}

u128 scph15000_ee_rq(void* dev, u32 addr)
{
    scph15000* device = (scph15000*) dev;
    u128 result;
    result.lo = 0;
    result.hi = 0;
    if(addr < 0x10000000)
    {
        result.lo = *(u64*)(device->ee_ram + (addr & 0x1ffffff));
        result.hi = *(u64*)(device->ee_ram + ((addr + 8) & 0x1ffffff));
        return result;
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
        result.lo = *(u64*)(device->iop_ram + (addr & 0x1fffff));
        result.hi = *(u64*)(device->iop_ram + ((addr + 8) & 0x1fffff));
        return result;
    }
    else if(addr >= 0x1fc00000 && addr < 0x20000000)
    {
        result.lo = *(u64*)(device->bios + (addr & 0x3fffff));
        result.hi = *(u64*)(device->bios + ((addr + 8) & 0x3fffff));;
        return result;
    }
    else if(addr >= 0x70000000 && addr < 0x70004000)
    {
        result.lo = *(u64*)(device->ee_sp_ram + (addr & 0x3ff0));
        result.hi = *(u64*)(device->ee_sp_ram + (addr & 0x3ff0) + 8);
        return result;
    }
    else fprintf(device->reg_access_log, "[EE] Unknown address %08x!\n", addr);
    return result;
}

void scph15000_ee_wb(void* dev, u32 addr, u8 data)
{
    scph15000* device = (scph15000*) dev;
    if(addr < 0x10000000)
    {
        device->ee_ram[addr & 0x1ffffff] = data;
    }
    else if(addr >= 0x10000000 && addr < 0x10010000)
    {
        if(addr == 0x1000f180)
        {
            fputc(data, device->ee_debug_log);
        }
        else fprintf(device->reg_access_log, "[EE] Unknown register write %08x data %02x pc %08x\n", addr, data, device->ee->pc);
    }
    else if(addr >= 0x12000000 && addr < 0x12010000)
    {
        fprintf(device->reg_access_log, "[EE] Unknown register write %08x data %02x pc %08x\n", addr, data, device->ee->pc);
    }
    else if(addr >= 0x1c000000 && addr < 0x1c200000)
    {
        device->iop_ram[addr & 0x1fffff] = data;
    }
    else if(addr >= 0x70000000 && addr < 0x70004000)
    {
        device->ee_sp_ram[addr & 0x3fff] = data;
    }
    else fprintf(device->reg_access_log, "[EE] Unknown address %08x data %02x!\n", addr, data);
}

void scph15000_ee_wh(void* dev, u32 addr, u16 data)
{
    scph15000* device = (scph15000*) dev;
    if(addr < 0x10000000)
    {
        *(u16*)(device->ee_ram + (addr & 0x1ffffff)) = data;
    }
    else if(addr >= 0x10000000 && addr < 0x10010000)
    {
        fprintf(device->reg_access_log, "[EE] Unknown register write %08x data %04x pc %08x\n", addr, data, device->ee->pc);
    }
    else if(addr >= 0x12000000 && addr < 0x12010000)
    {
        fprintf(device->reg_access_log, "[EE] Unknown register write %08x data %04x pc %08x\n", addr, data, device->ee->pc);
    }
    else if(addr >= 0x1c000000 && addr < 0x1c200000)
    {
        *(u16*)(device->iop_ram + (addr & 0x1fffff)) = data;
    }
    else if(addr >= 0x70000000 && addr < 0x70004000)
    {
        *(u16*)(device->ee_sp_ram + (addr & 0x3ffe)) = data;
    }
    else fprintf(device->reg_access_log, "[EE] Unknown address %08x data %04x!\n", addr, data);
}

void scph15000_ee_ww(void* dev, u32 addr, u32 data)
{
    scph15000* device = (scph15000*) dev;
    if(addr < 0x10000000)
    {
        *(u32*)(device->ee_ram + (addr & 0x1ffffff)) = data;
    }
    else if(addr >= 0x10000000 && addr < 0x10010000)
    {
        if(addr == 0x1000f430)
        {
            fprintf(device->reg_access_log, "[EE] MCH RICM write %08x data %08x pc %08x\n", addr, data, device->ee->pc);
            u16 cmd = (data >> 16) & 0xfff;
            u8 subcmd = (data >> 6) & 0xf;
            bool tmp = ((device->mch_drd >> 7) & 1) ? false : true;
            switch(cmd)
            {
                case 0x021:
                {
                    if(subcmd == 1 && tmp) device->rdram_sdevid = 0;
                    break;
                }
            }
            device->mch_ricm = data & ~(1 << 31);
        }
        else if(addr == 0x1000f440)
        {
            fprintf(device->reg_access_log, "[EE] MCH DRD write %08x data %08x pc %08x\n", addr, data, device->ee->pc);
            device->mch_drd = data;
        }
        else fprintf(device->reg_access_log, "[EE] Unknown register write %08x data %08x pc %08x\n", addr, data, device->ee->pc);
    }
    else if(addr >= 0x12000000 && addr < 0x12010000)
    {
        fprintf(device->reg_access_log, "[EE] Unknown register write %08x data %08x pc %08x\n", addr, data, device->ee->pc);
    }
    else if(addr >= 0x1c000000 && addr < 0x1c200000)
    {
        fprintf(device->reg_access_log, "[EE] IOP RAM write %08x data %08x pc %08x\n", addr & 0x1fffff, data, device->ee->pc);
        *(u32*)(device->iop_ram + (addr & 0x1fffff)) = data;
    }
    else if(addr >= 0x70000000 && addr < 0x70004000)
    {
        *(u32*)(device->ee_sp_ram + (addr & 0x3ffc)) = data;
    }
    else fprintf(device->reg_access_log, "[EE] Unknown address %08x data %08x!\n", addr, data);
}

void scph15000_ee_wd(void* dev, u32 addr, u64 data)
{
    scph15000* device = (scph15000*) dev;
    if(addr < 0x10000000)
    {
        *(u64*)(device->ee_ram + (addr & 0x1ffffff)) = data;
    }
    else if(addr >= 0x10000000 && addr < 0x10010000)
    {
        if(addr < 0x10002000 || addr >= 0x10003000) scph15000_ee_ww(dev, addr, (u32)data);
        else fprintf(device->reg_access_log, "[EE] Unknown register write %08x data %016" PRIx64 " pc %08x\n", addr, data, device->ee->pc);
    }
    else if(addr >= 0x12000000 && addr < 0x12010000)
    {
        fprintf(device->reg_access_log, "[EE] Unknown register write %08x data %016" PRIx64 " pc %08x\n", addr, data, device->ee->pc);
    }
    else if(addr >= 0x1c000000 && addr < 0x1c200000)
    {
        *(u64*)(device->iop_ram + (addr & 0x1fffff)) = data;
    }
    else if(addr >= 0x70000000 && addr < 0x70004000)
    {
        *(u64*)(device->ee_sp_ram + (addr & 0x3ff8)) = data;
    }
    else fprintf(device->reg_access_log, "[EE] Unknown address %08x data %016" PRIx64 "!\n", addr, data);
}

void scph15000_ee_wq(void* dev, u32 addr, u128 data)
{
    scph15000* device = (scph15000*) dev;
    if(addr < 0x10000000)
    {
        *(u64*)(device->ee_ram + (addr & 0x1ffffff)) = data.lo;
        *(u64*)(device->ee_ram + ((addr + 8) & 0x1ffffff)) = data.hi;
    }
    else if(addr >= 0x10000000 && addr < 0x10010000)
    {
        fprintf(device->reg_access_log, "[EE] Unknown register write %08x data %016" PRIx64 "%016" PRIx64 " pc %08x\n", addr, data.hi, data.lo, device->ee->pc);
    }
    else if(addr >= 0x12000000 && addr < 0x12010000)
    {
        fprintf(device->reg_access_log, "[EE] Unknown register write %08x data %016" PRIx64 "%016" PRIx64 " pc %08x\n", addr, data.hi, data.lo, device->ee->pc);
    }
    else if(addr >= 0x1c000000 && addr < 0x1c200000)
    {
        *(u64*)(device->iop_ram + (addr & 0x1fffff)) = data.lo;
        *(u64*)(device->iop_ram + ((addr + 8) & 0x1fffff)) = data.hi;
    }
    else if(addr >= 0x70000000 && addr < 0x70004000)
    {
        *(u64*)(device->ee_sp_ram + (addr & 0x3ff0)) = data.lo;
        *(u64*)(device->ee_sp_ram + (addr & 0x3ff0) + 8) = data.hi;
    }
    else fprintf(device->reg_access_log, "[EE] Unknown address %08x data %016" PRIx64 "%016" PRIx64 "!\n", addr, data.hi, data.lo);
}

u8 scph15000_iop_rb(void* dev, u32 addr)
{
    scph15000* device = (scph15000*) dev;
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
    else fprintf(device->reg_access_log, "[IOP] Unknown address %08x!\n", addr);
    return 0;
}

u16 scph15000_iop_rh(void* dev, u32 addr)
{
    scph15000* device = (scph15000*) dev;
    if(addr < 0x00200000)
    {
        return *(u16*)(device->iop_ram + (addr & 0x1fffff));
    }
    else if(addr >= 0x1d000000 && addr < 0x1f810000)
    {
        fprintf(device->reg_access_log, "[IOP] Unknown register read %08x pc %08x\n", addr, device->iop->pc);
    }
    else if(addr >= 0x1fc00000 && addr < 0x20000000)
    {
        return *(u16*)(device->bios + (addr & 0x3fffff));
    }
    else fprintf(device->reg_access_log, "[IOP] Unknown address %08x!\n", addr);
    return 0;
}

u32 scph15000_iop_rw(void* dev, u32 addr)
{
    scph15000* device = (scph15000*) dev;
    if(addr < 0x00200000)
    {
        return *(u32*)(device->iop_ram + (addr & 0x1fffff));
    }
    else if(addr >= 0x1d000000 && addr < 0x1f810000)
    {
        if(addr >= 0x1f801080 && addr < 0x1f801100)
        {
            fprintf(device->reg_access_log, "[IOP] DMA read %08x pc %08x\n", addr, device->iop->pc);
            return device->iop_dma.dma_rw(addr);
        }
        else if(addr >= 0x1f801500 && addr < 0x1f801580)
        {
            fprintf(device->reg_access_log, "[IOP] DMA read %08x pc %08x\n", addr, device->iop->pc);
            return device->iop_dma.dma_rw(addr);
        }
        else fprintf(device->reg_access_log, "[IOP] Unknown register read %08x pc %08x\n", addr, device->iop->pc);
    }
    else if(addr >= 0x1fc00000 && addr < 0x20000000)
    {
        return *(u32*)(device->bios + (addr & 0x3fffff));
    }
    else fprintf(device->reg_access_log, "[IOP] Unknown address %08x!\n", addr);
    return 0;
}

void scph15000_iop_wb(void* dev, u32 addr, u8 data)
{
    scph15000* device = (scph15000*) dev;
    if(addr < 0x00200000)
    {
        device->iop_ram[addr & 0x1fffff] = data;
    }
    else if(addr >= 0x1d000000 && addr < 0x1f810000)
    {
        fprintf(device->reg_access_log, "[IOP] Unknown register write %08x data %02x pc %08x\n", addr, data, device->iop->pc);
    }
    else fprintf(device->reg_access_log, "[IOP] Unknown address %08x data %02x!\n", addr, data);
}

void scph15000_iop_wh(void* dev, u32 addr, u16 data)
{
    scph15000* device = (scph15000*) dev;
    if(addr < 0x00200000)
    {
        *(u16*)(device->iop_ram + (addr & 0x1fffff)) = data;
    }
    else if(addr >= 0x1d000000 && addr < 0x1f810000)
    {
        fprintf(device->reg_access_log, "[IOP] Unknown register write %08x data %08x pc %08x\n", addr, data, device->iop->pc);
    }
    else fprintf(device->reg_access_log, "[IOP] Unknown address %08x data %08x!\n", addr, data);
}

void scph15000_iop_ww(void* dev, u32 addr, u32 data)
{
    scph15000* device = (scph15000*) dev;
    if(addr < 0x00200000)
    {
        fprintf(device->reg_access_log, "[IOP] IOP RAM write %08x data %08x pc %08x\n", addr & 0x1fffff, data, device->iop->pc);
        *(u32*)(device->iop_ram + (addr & 0x1fffff)) = data;
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
    else fprintf(device->reg_access_log, "[IOP] Unknown address %08x data %08x!\n", addr, data);
}