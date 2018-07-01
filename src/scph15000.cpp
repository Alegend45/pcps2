#include "scph15000.h"

void scph15000::init()
{
    //TODO
    bios = (u8*)malloc(0x400000);
    memset(bios, 0, 0x400000);
    ee_ram = (u8*)malloc(0x2000000);
    memset(ee_ram, 0, 0x2000000);
    iop_ram = (u8*)malloc(0x200000);
    memset(iop_ram, 0, 0x200000);
    ee_sp_ram = (u8*)malloc(0x4000);
    memset(ee_sp_ram, 0, 0x4000);

    mch_drd = 0;
    mch_ricm = 0;
    rdram_sdevid = 0;

    iop_dma.device = this;
    iop_dma.rw = scph15000_iop_rw;
    iop_dma.ww = scph15000_iop_ww;
    iop_dma.init();

    reg_access_log = fopen("reglog.txt","w+");
    ee_debug_log = fopen("debug_console.txt","w+");
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
    else printf("[EE] Unknown address %08x!\n", addr);
    return 0;
}

u16 scph15000_ee_rh(void* dev, u32 addr)
{
    scph15000* device = (scph15000*) dev;
    if(addr < 0x10000000)
    {
        return device->ee_ram[(addr+0) & 0x1ffffff] | (device->ee_ram[(addr+1) & 0x1ffffff] << 8);
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
        return device->iop_ram[(addr+0) & 0x1fffff] | (device->iop_ram[(addr+1) & 0x1fffff] << 8);
    }
    else if(addr >= 0x1fc00000 && addr < 0x20000000)
    {
        return device->bios[(addr+0) & 0x3fffff] | (device->bios[(addr+1) & 0x3fffff] << 8);
    }
    else if(addr >= 0x70000000 && addr < 0x70004000)
    {
        return device->ee_sp_ram[(addr+0) & 0x3fff] | (device->ee_sp_ram[(addr+1) & 0x3fff] << 8);
    }
    else printf("[EE] Unknown address %08x!\n", addr);
    return 0;
}

u32 scph15000_ee_rw(void* dev, u32 addr)
{
    scph15000* device = (scph15000*) dev;
    if(addr < 0x10000000)
    {
        return device->ee_ram[(addr+0) & 0x1ffffff] | (device->ee_ram[(addr+1) & 0x1ffffff] << 8)
        | (device->ee_ram[(addr+2) & 0x1ffffff] << 16) | (device->ee_ram[(addr+3) & 0x1ffffff] << 24);
    }
    else if(addr >= 0x10000000 && addr < 0x10010000)
    {
        if(addr == 0x1000f440)
        {
            fprintf(device->reg_access_log, "[EE] MCH DRD read %08x pc %08x\n", addr, device->ee->pc);
            if(!((device->mch_ricm >> 6) & 0xf))
            {
                switch((device->mch_ricm >> 16) & 0xfff)
                {
                    case 0x021:
                    {
                        if(device->rdram_sdevid < 2)
                        {
                            device->rdram_sdevid++;
                            return 0x1f;
                        }
                        return 0;
                    }
                    case 0x023:
                    {
                        return 0x0d0d;
                    }
                    case 0x024:
                    {
                        return 0x0090;
                    }
                    case 0x040:
                    {
                        return device->mch_ricm & 0x1f;
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
        return device->iop_ram[(addr+0) & 0x1fffff] | (device->iop_ram[(addr+1) & 0x1fffff] << 8)
        | (device->iop_ram[(addr+2) & 0x1fffff] << 16) | (device->iop_ram[(addr+3) & 0x1fffff] << 24);
    }
    else if(addr >= 0x1fc00000 && addr < 0x20000000)
    {
        return device->bios[(addr+0) & 0x3fffff] | (device->bios[(addr+1) & 0x3fffff] << 8)
        | (device->bios[(addr+2) & 0x3fffff] << 16) | (device->bios[(addr+3) & 0x3fffff] << 24);
    }
    else if(addr >= 0x70000000 && addr < 0x70004000)
    {
        return device->ee_sp_ram[(addr+0) & 0x3fff] | (device->ee_sp_ram[(addr+1) & 0x3fff] << 8)
        | (device->ee_sp_ram[(addr+2) & 0x3fff] << 16) | (device->ee_sp_ram[(addr+3) & 0x3fff] << 24);
    }
    else printf("[EE] Unknown address %08x!\n", addr);
    return 0;
}

u64 scph15000_ee_rd(void* dev, u32 addr)
{
    scph15000* device = (scph15000*) dev;
    if(addr < 0x10000000)
    {
        return device->ee_ram[(addr+0) & 0x1ffffff] | (device->ee_ram[(addr+1) & 0x1ffffff] << 8)
        | (device->ee_ram[(addr+2) & 0x1ffffff] << 16) | (device->ee_ram[(addr+3) & 0x1ffffff] << 24)
        | (device->ee_ram[(addr+4) & 0x1ffffff] << 32) | (device->ee_ram[(addr+5) & 0x1ffffff] << 40)
        | (device->ee_ram[(addr+6) & 0x1ffffff] << 48) | (device->ee_ram[(addr+7) & 0x1ffffff] << 56);
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
        | (device->iop_ram[(addr+2) & 0x1fffff] << 16) | (device->iop_ram[(addr+3) & 0x1fffff] << 24)
        | (device->iop_ram[(addr+4) & 0x1fffff] << 32) | (device->iop_ram[(addr+5) & 0x1fffff] << 40)
        | (device->iop_ram[(addr+6) & 0x1fffff] << 48) | (device->iop_ram[(addr+7) & 0x1fffff] << 56);
    }
    else if(addr >= 0x1fc00000 && addr < 0x20000000)
    {
        return device->bios[(addr+0) & 0x3fffff] | (device->bios[(addr+1) & 0x3fffff] << 8)
        | (device->bios[(addr+2) & 0x3fffff] << 16) | (device->bios[(addr+3) & 0x3fffff] << 24)
        | (device->bios[(addr+4) & 0x3fffff] << 32) | (device->bios[(addr+5) & 0x3fffff] << 40)
        | (device->bios[(addr+6) & 0x3fffff] << 48) | (device->bios[(addr+7) & 0x3fffff] << 56);
    }
    else if(addr >= 0x70000000 && addr < 0x70004000)
    {
        return device->ee_sp_ram[(addr+0) & 0x3fff] | (device->ee_sp_ram[(addr+1) & 0x3fff] << 8)
        | (device->ee_sp_ram[(addr+2) & 0x3fff] << 16) | (device->ee_sp_ram[(addr+3) & 0x3fff] << 24)
        | (device->ee_sp_ram[(addr+4) & 0x3fff] << 32) | (device->ee_sp_ram[(addr+5) & 0x3fff] << 40)
        | (device->ee_sp_ram[(addr+6) & 0x3fff] << 48) | (device->ee_sp_ram[(addr+7) & 0x3fff] << 56);
    }
    else printf("[EE] Unknown address %08x!\n", addr);
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
        result.lo = device->ee_ram[(addr+0) & 0x1ffffff] | (device->ee_ram[(addr+1) & 0x1ffffff] << 8)
        | (device->ee_ram[(addr+2) & 0x1ffffff] << 16) | (device->ee_ram[(addr+3) & 0x1ffffff] << 24)
        | (device->ee_ram[(addr+4) & 0x1ffffff] << 32) | (device->ee_ram[(addr+5) & 0x1ffffff] << 40)
        | (device->ee_ram[(addr+6) & 0x1ffffff] << 48) | (device->ee_ram[(addr+7) & 0x1ffffff] << 56);

        result.hi = device->ee_ram[(addr+8) & 0x1ffffff] | (device->ee_ram[(addr+9) & 0x1ffffff] << 8)
        | (device->ee_ram[(addr+10) & 0x1ffffff] << 16) | (device->ee_ram[(addr+11) & 0x1ffffff] << 24)
        | (device->ee_ram[(addr+12) & 0x1ffffff] << 32) | (device->ee_ram[(addr+13) & 0x1ffffff] << 40)
        | (device->ee_ram[(addr+14) & 0x1ffffff] << 48) | (device->ee_ram[(addr+15) & 0x1ffffff] << 56);

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
        result.lo = device->iop_ram[(addr+0) & 0x1fffff] | (device->iop_ram[(addr+1) & 0x1fffff] << 8)
        | (device->iop_ram[(addr+2) & 0x1fffff] << 16) | (device->iop_ram[(addr+3) & 0x1fffff] << 24)
        | (device->iop_ram[(addr+4) & 0x1fffff] << 32) | (device->iop_ram[(addr+5) & 0x1fffff] << 40)
        | (device->iop_ram[(addr+6) & 0x1fffff] << 48) | (device->iop_ram[(addr+7) & 0x1fffff] << 56);

        result.hi = device->iop_ram[(addr+8) & 0x1fffff] | (device->iop_ram[(addr+9) & 0x1fffff] << 8)
        | (device->iop_ram[(addr+10) & 0x1fffff] << 16) | (device->iop_ram[(addr+11) & 0x1fffff] << 24)
        | (device->iop_ram[(addr+12) & 0x1fffff] << 32) | (device->iop_ram[(addr+13) & 0x1fffff] << 40)
        | (device->iop_ram[(addr+14) & 0x1fffff] << 48) | (device->iop_ram[(addr+15) & 0x1fffff] << 56);

        return result;
    }
    else if(addr >= 0x1fc00000 && addr < 0x20000000)
    {
        result.lo = device->bios[(addr+0) & 0x3fffff] | (device->bios[(addr+1) & 0x3fffff] << 8)
        | (device->bios[(addr+2) & 0x3fffff] << 16) | (device->bios[(addr+3) & 0x3fffff] << 24)
        | (device->bios[(addr+4) & 0x3fffff] << 32) | (device->bios[(addr+5) & 0x3fffff] << 40)
        | (device->bios[(addr+6) & 0x3fffff] << 48) | (device->bios[(addr+7) & 0x3fffff] << 56);

        result.hi = device->bios[(addr+8) & 0x3fffff] | (device->bios[(addr+9) & 0x3fffff] << 8)
        | (device->bios[(addr+10) & 0x3fffff] << 16) | (device->bios[(addr+11) & 0x3fffff] << 24)
        | (device->bios[(addr+12) & 0x3fffff] << 32) | (device->bios[(addr+13) & 0x3fffff] << 40)
        | (device->bios[(addr+14) & 0x3fffff] << 48) | (device->bios[(addr+15) & 0x3fffff] << 56);

        return result;
    }
    else if(addr >= 0x70000000 && addr < 0x70004000)
    {
        result.lo = device->ee_sp_ram[(addr+0) & 0x3fff] | (device->ee_sp_ram[(addr+1) & 0x3fff] << 8)
        | (device->ee_sp_ram[(addr+2) & 0x3fff] << 16) | (device->ee_sp_ram[(addr+3) & 0x3fff] << 24)
        | (device->ee_sp_ram[(addr+4) & 0x3fff] << 32) | (device->ee_sp_ram[(addr+5) & 0x3fff] << 40)
        | (device->ee_sp_ram[(addr+6) & 0x3fff] << 48) | (device->ee_sp_ram[(addr+7) & 0x3fff] << 56);

        result.hi = device->ee_sp_ram[(addr+8) & 0x3fff] | (device->ee_sp_ram[(addr+9) & 0x3fff] << 8)
        | (device->ee_sp_ram[(addr+10) & 0x3fff] << 16) | (device->ee_sp_ram[(addr+11) & 0x3fff] << 24)
        | (device->ee_sp_ram[(addr+12) & 0x3fff] << 32) | (device->ee_sp_ram[(addr+13) & 0x3fff] << 40)
        | (device->ee_sp_ram[(addr+14) & 0x3fff] << 48) | (device->ee_sp_ram[(addr+15) & 0x3fff] << 56);

        return result;
    }
    else printf("[EE] Unknown address %08x!\n", addr);
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
    else printf("[EE] Unknown address %08x data %02x!\n", addr, data);
}

void scph15000_ee_wh(void* dev, u32 addr, u16 data)
{
    scph15000* device = (scph15000*) dev;
    if(addr < 0x10000000)
    {
        device->ee_ram[(addr+0) & 0x1ffffff] = (data >> 0) & 0xff;
        device->ee_ram[(addr+1) & 0x1ffffff] = (data >> 8) & 0xff;
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
        device->iop_ram[(addr+0) & 0x1fffff] = (data >> 0) & 0xff;
        device->iop_ram[(addr+1) & 0x1fffff] = (data >> 8) & 0xff;
    }
    else if(addr >= 0x70000000 && addr < 0x70004000)
    {
        device->ee_sp_ram[(addr+0) & 0x3fff] = (data >> 0) & 0xff;
        device->ee_sp_ram[(addr+1) & 0x3fff] = (data >> 8) & 0xff;
    }
    else printf("[EE] Unknown address %08x data %04x!\n", addr, data);
}

void scph15000_ee_ww(void* dev, u32 addr, u32 data)
{
    scph15000* device = (scph15000*) dev;
    if(addr < 0x10000000)
    {
        device->ee_ram[(addr+0) & 0x1ffffff] = (data >> 0) & 0xff;
        device->ee_ram[(addr+1) & 0x1ffffff] = (data >> 8) & 0xff;
        device->ee_ram[(addr+2) & 0x1ffffff] = (data >> 16) & 0xff;
        device->ee_ram[(addr+3) & 0x1ffffff] = (data >> 24) & 0xff;
    }
    else if(addr >= 0x10000000 && addr < 0x10010000)
    {
        if(addr == 0x1000f430)
        {
            fprintf(device->reg_access_log, "[EE] MCH RICM write %08x data %08x pc %08x\n", addr, data, device->ee->pc);
            if((((data >> 16) & 0xfff) == 0x021) && (((data >> 6) & 0xf) == 1) && (((device->mch_drd >> 7) & 1) == 0))
            {
                device->rdram_sdevid = 0;
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
        device->iop_ram[(addr+0) & 0x1fffff] = (data >> 0) & 0xff;
        device->iop_ram[(addr+1) & 0x1fffff] = (data >> 8) & 0xff;
        device->iop_ram[(addr+2) & 0x1fffff] = (data >> 16) & 0xff;
        device->iop_ram[(addr+3) & 0x1fffff] = (data >> 24) & 0xff;
    }
    else if(addr >= 0x70000000 && addr < 0x70004000)
    {
        device->ee_sp_ram[(addr+0) & 0x3fff] = (data >> 0) & 0xff;
        device->ee_sp_ram[(addr+1) & 0x3fff] = (data >> 8) & 0xff;
        device->ee_sp_ram[(addr+2) & 0x3fff] = (data >> 16) & 0xff;
        device->ee_sp_ram[(addr+3) & 0x3fff] = (data >> 24) & 0xff;
    }
    else printf("[EE] Unknown address %08x data %08x!\n", addr, data);
}

void scph15000_ee_wd(void* dev, u32 addr, u64 data)
{
    scph15000* device = (scph15000*) dev;
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
        if(addr < 0x10002000 || addr >= 0x10003000) scph15000_ee_ww(dev, addr, (u32)data);
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
    else if(addr >= 0x70000000 && addr < 0x70004000)
    {
        device->ee_sp_ram[(addr+0) & 0x3fff] = (data >> 0) & 0xff;
        device->ee_sp_ram[(addr+1) & 0x3fff] = (data >> 8) & 0xff;
        device->ee_sp_ram[(addr+2) & 0x3fff] = (data >> 16) & 0xff;
        device->ee_sp_ram[(addr+3) & 0x3fff] = (data >> 24) & 0xff;
        device->ee_sp_ram[(addr+4) & 0x3fff] = (data >> 32) & 0xff;
        device->ee_sp_ram[(addr+5) & 0x3fff] = (data >> 40) & 0xff;
        device->ee_sp_ram[(addr+6) & 0x3fff] = (data >> 48) & 0xff;
        device->ee_sp_ram[(addr+7) & 0x3fff] = (data >> 56) & 0xff;
    }
    else printf("[EE] Unknown address %08x data %016x!\n", addr, data);
}

void scph15000_ee_wq(void* dev, u32 addr, u128 data)
{
    scph15000* device = (scph15000*) dev;
    if(addr < 0x10000000)
    {
        device->ee_ram[(addr+0) & 0x1ffffff] = (data.lo >> 0) & 0xff;
        device->ee_ram[(addr+1) & 0x1ffffff] = (data.lo >> 8) & 0xff;
        device->ee_ram[(addr+2) & 0x1ffffff] = (data.lo >> 16) & 0xff;
        device->ee_ram[(addr+3) & 0x1ffffff] = (data.lo >> 24) & 0xff;
        device->ee_ram[(addr+4) & 0x1ffffff] = (data.lo >> 32) & 0xff;
        device->ee_ram[(addr+5) & 0x1ffffff] = (data.lo >> 40) & 0xff;
        device->ee_ram[(addr+6) & 0x1ffffff] = (data.lo >> 48) & 0xff;
        device->ee_ram[(addr+7) & 0x1ffffff] = (data.lo >> 56) & 0xff;

        device->ee_ram[(addr+8) & 0x1ffffff] = (data.hi >> 0) & 0xff;
        device->ee_ram[(addr+9) & 0x1ffffff] = (data.hi >> 8) & 0xff;
        device->ee_ram[(addr+10) & 0x1ffffff] = (data.hi >> 16) & 0xff;
        device->ee_ram[(addr+11) & 0x1ffffff] = (data.hi >> 24) & 0xff;
        device->ee_ram[(addr+12) & 0x1ffffff] = (data.hi >> 32) & 0xff;
        device->ee_ram[(addr+13) & 0x1ffffff] = (data.hi >> 40) & 0xff;
        device->ee_ram[(addr+14) & 0x1ffffff] = (data.hi >> 48) & 0xff;
        device->ee_ram[(addr+15) & 0x1ffffff] = (data.hi >> 56) & 0xff;
    }
    else if(addr >= 0x10000000 && addr < 0x10010000)
    {
        fprintf(device->reg_access_log, "[EE] Unknown register write %08x data %016x%016x pc %08x\n", addr, data.hi, data.lo, device->ee->pc);
    }
    else if(addr >= 0x12000000 && addr < 0x12010000)
    {
        fprintf(device->reg_access_log, "[EE] Unknown register write %08x data %016x%016x pc %08x\n", addr, data.hi, data.lo, device->ee->pc);
    }
    else if(addr >= 0x1c000000 && addr < 0x1c200000)
    {
        device->iop_ram[(addr+0) & 0x1fffff] = (data.lo >> 0) & 0xff;
        device->iop_ram[(addr+1) & 0x1fffff] = (data.lo >> 8) & 0xff;
        device->iop_ram[(addr+2) & 0x1fffff] = (data.lo >> 16) & 0xff;
        device->iop_ram[(addr+3) & 0x1fffff] = (data.lo >> 24) & 0xff;
        device->iop_ram[(addr+4) & 0x1fffff] = (data.lo >> 32) & 0xff;
        device->iop_ram[(addr+5) & 0x1fffff] = (data.lo >> 40) & 0xff;
        device->iop_ram[(addr+6) & 0x1fffff] = (data.lo >> 48) & 0xff;
        device->iop_ram[(addr+7) & 0x1fffff] = (data.lo >> 56) & 0xff;

        device->iop_ram[(addr+8) & 0x1fffff] = (data.hi >> 0) & 0xff;
        device->iop_ram[(addr+9) & 0x1fffff] = (data.hi >> 8) & 0xff;
        device->iop_ram[(addr+10) & 0x1fffff] = (data.hi >> 16) & 0xff;
        device->iop_ram[(addr+11) & 0x1fffff] = (data.hi >> 24) & 0xff;
        device->iop_ram[(addr+12) & 0x1fffff] = (data.hi >> 32) & 0xff;
        device->iop_ram[(addr+13) & 0x1fffff] = (data.hi >> 40) & 0xff;
        device->iop_ram[(addr+14) & 0x1fffff] = (data.hi >> 48) & 0xff;
        device->iop_ram[(addr+15) & 0x1fffff] = (data.hi >> 56) & 0xff;
    }
    else if(addr >= 0x70000000 && addr < 0x70004000)
    {
        device->ee_sp_ram[(addr+0) & 0x3fff] = (data.lo >> 0) & 0xff;
        device->ee_sp_ram[(addr+1) & 0x3fff] = (data.lo >> 8) & 0xff;
        device->ee_sp_ram[(addr+2) & 0x3fff] = (data.lo >> 16) & 0xff;
        device->ee_sp_ram[(addr+3) & 0x3fff] = (data.lo >> 24) & 0xff;
        device->ee_sp_ram[(addr+4) & 0x3fff] = (data.lo >> 32) & 0xff;
        device->ee_sp_ram[(addr+5) & 0x3fff] = (data.lo >> 40) & 0xff;
        device->ee_sp_ram[(addr+6) & 0x3fff] = (data.lo >> 48) & 0xff;
        device->ee_sp_ram[(addr+7) & 0x3fff] = (data.lo >> 56) & 0xff;

        device->ee_sp_ram[(addr+8) & 0x3fff] = (data.hi >> 0) & 0xff;
        device->ee_sp_ram[(addr+9) & 0x3fff] = (data.hi >> 8) & 0xff;
        device->ee_sp_ram[(addr+10) & 0x3fff] = (data.hi >> 16) & 0xff;
        device->ee_sp_ram[(addr+11) & 0x3fff] = (data.hi >> 24) & 0xff;
        device->ee_sp_ram[(addr+12) & 0x3fff] = (data.hi >> 32) & 0xff;
        device->ee_sp_ram[(addr+13) & 0x3fff] = (data.hi >> 40) & 0xff;
        device->ee_sp_ram[(addr+14) & 0x3fff] = (data.hi >> 48) & 0xff;
        device->ee_sp_ram[(addr+15) & 0x3fff] = (data.hi >> 56) & 0xff;
    }
    else printf("[EE] Unknown address %08x data %016x%016x!\n", addr, data.hi, data.lo);
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
    else printf("[IOP] Unknown address %08x!\n", addr);
    return 0;
}

u16 scph15000_iop_rh(void* dev, u32 addr)
{
    scph15000* device = (scph15000*) dev;
    if(addr < 0x00200000)
    {
        return device->iop_ram[(addr+0) & 0x1fffff] | (device->iop_ram[(addr+1) & 0x1fffff] << 8);
    }
    else if(addr >= 0x1d000000 && addr < 0x1f810000)
    {
        fprintf(device->reg_access_log, "[IOP] Unknown register read %08x pc %08x\n", addr, device->iop->pc);
    }
    else if(addr >= 0x1fc00000 && addr < 0x20000000)
    {
        return device->bios[(addr+0) & 0x3fffff] | (device->bios[(addr+1) & 0x3fffff] << 8);
    }
    else printf("[IOP] Unknown address %08x!\n", addr);
    return 0;
}

u32 scph15000_iop_rw(void* dev, u32 addr)
{
    scph15000* device = (scph15000*) dev;
    if(addr < 0x00200000)
    {
        return device->iop_ram[(addr+0) & 0x1fffff] | (device->iop_ram[(addr+1) & 0x1fffff] << 8)
        | (device->iop_ram[(addr+2) & 0x1fffff] << 16) | (device->iop_ram[(addr+3) & 0x1fffff] << 24);
    }
    else if(addr >= 0x1d000000 && addr < 0x1f810000)
    {
        fprintf(device->reg_access_log, "[IOP] Unknown register read %08x pc %08x\n", addr, device->iop->pc);
    }
    else if(addr >= 0x1fc00000 && addr < 0x20000000)
    {
        return device->bios[(addr+0) & 0x3fffff] | (device->bios[(addr+1) & 0x3fffff] << 8)
        | (device->bios[(addr+2) & 0x3fffff] << 16) | (device->bios[(addr+3) & 0x3fffff] << 24);
    }
    else printf("[IOP] Unknown address %08x!\n", addr);
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
    else printf("[IOP] Unknown address %08x data %02x!\n", addr, data);
}

void scph15000_iop_wh(void* dev, u32 addr, u16 data)
{
    scph15000* device = (scph15000*) dev;
    if(addr < 0x00200000)
    {
        device->iop_ram[(addr+0) & 0x1fffff] = (data >> 0) & 0xff;
        device->iop_ram[(addr+1) & 0x1fffff] = (data >> 8) & 0xff;
    }
    else if(addr >= 0x1d000000 && addr < 0x1f810000)
    {
        fprintf(device->reg_access_log, "[IOP] Unknown register write %08x data %08x pc %08x\n", addr, data, device->iop->pc);
    }
    else printf("[IOP] Unknown address %08x data %08x!\n", addr, data);
}

void scph15000_iop_ww(void* dev, u32 addr, u32 data)
{
    scph15000* device = (scph15000*) dev;
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