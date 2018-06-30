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

u32 scph10000_ee_rw(void* dev, u32 addr)
{
    scph10000* device = (scph10000*) dev;
    if(addr < 0x10000000)
    {
        return device->ee_ram[(addr+0) & 0x1ffffff] | (device->ee_ram[(addr+1) & 0x1ffffff] << 8)
        | (device->ee_ram[(addr+2) & 0x1ffffff] << 16) | (device->ee_ram[(addr+3) & 0x1ffffff] << 24);
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
    else printf("Unknown address %08x!\n", addr);
    return 0;
}

void scph10000_ee_ww(void* dev, u32 addr, u32 data)
{
    scph10000* device = (scph10000*) dev;
    if(addr < 0x10000000)
    {
        device->ee_ram[(addr+0) & 0x1ffffff] = (data >> 0) & 0xff;
        device->ee_ram[(addr+1) & 0x1ffffff] = (data >> 8) & 0xff;
        device->ee_ram[(addr+2) & 0x1ffffff] = (data >> 16) & 0xff;
        device->ee_ram[(addr+3) & 0x1ffffff] = (data >> 24) & 0xff;
    }
    else if(addr >= 0x1c000000 && addr < 0x1c200000)
    {
        device->iop_ram[(addr+0) & 0x1fffff] = (data >> 0) & 0xff;
        device->iop_ram[(addr+1) & 0x1fffff] = (data >> 8) & 0xff;
        device->iop_ram[(addr+2) & 0x1fffff] = (data >> 16) & 0xff;
        device->iop_ram[(addr+3) & 0x1fffff] = (data >> 24) & 0xff;
    }
    else printf("Unknown address %08x data %08x!\n", addr, data);
}

u32 scph10000_iop_rw(void* dev, u32 addr)
{
    scph10000* device = (scph10000*) dev;
    if(addr < 0x00200000)
    {
        return device->iop_ram[(addr+0) & 0x1fffff] | (device->iop_ram[(addr+1) & 0x1fffff] << 8)
        | (device->iop_ram[(addr+2) & 0x1fffff] << 16) | (device->iop_ram[(addr+3) & 0x1fffff] << 24);
    }
    else if(addr >= 0x1fc00000 && addr < 0x20000000)
    {
        return device->bios[(addr+0) & 0x3fffff] | (device->bios[(addr+1) & 0x3fffff] << 8)
        | (device->bios[(addr+2) & 0x3fffff] << 16) | (device->bios[(addr+3) & 0x3fffff] << 24);
    }
    else printf("Unknown address %08x!\n", addr);
    return 0;
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
    else printf("Unknown address %08x data %08x!\n", addr, data);
}