#include "iop_dma.h"

void iop_dma_t::init()
{
    for(int i = 0; i < 16; i++)
    {
        dpcr.enable[i] = false;
        dpcr.priorities[i] = 7;
    }
}

void iop_dma_t::tick()
{
}

u32 iop_dma_t::dma_rw(u32 addr)
{
    u32 dma_addr = addr & 0xfff;
    if(dma_addr <= 0x0f0 || (dma_addr >= 0x500 && dma_addr < 0x570))
    {
        u32 channel = (dma_addr >> 4) & 0x7;
        if(dma_addr >= 0x500 && dma_addr < 0x570) channel += 8;
        switch(dma_addr & 0xc)
        {
            case 0x0:
            {
                return channels[channel].addr;
            }
            case 0x4:
            {
                return channels[channel].word_count | (channels[channel].block_size << 16);
            }
            case 0x8:
            {
                return channels[channel].control.whole;
            }
            case 0xc:
            {
                return channels[channel].tag_addr;
            }
        }
    }
    switch(dma_addr)
    {
        case 0x0f0:
        {
            u32 result = 0;
            for(u32 i = 0; i < 8; i++)
            {
                result |= dpcr.priorities[i] << (i << 2);
                result |= dpcr.enable[i] << ((i << 2) + 3);
            }
            return result;
        }
        case 0x0f4:
        {
            u32 result = 0;
            result |= dicr.mask[0] << 16;
            result |= dicr.master_int_enable[0] << 23;
            result |= dicr.status[0] << 24;

            bool irq;
            if(dicr.master_int_enable[0] && (dicr.mask[0] & dicr.status[0])) irq = true;
            else irq = false;

            result |= irq << 31;
            return result;
        }
        case 0x570:
        {
            u32 result = 0;
            for(u32 i = 8; i < 16; i++)
            {
                int bit = i - 8;
                result |= dpcr.priorities[i] << (bit << 2);
                result |= dpcr.enable[i] << ((bit << 2) + 3);
            }
            return result;
        }
        case 0x574:
        {
            u32 result = 0;
            result |= dicr.mask[1] << 16;
            result |= dicr.master_int_enable[1] << 23;
            result |= dicr.status[1] << 24;

            bool irq;
            if(dicr.master_int_enable[1] && (dicr.mask[1] & dicr.status[1])) irq = true;
            else irq = false;

            result |= irq << 31;
            return result;
        }
    }
    return 0;
}

void iop_dma_t::dma_ww(u32 addr, u32 data)
{
    u32 dma_addr = addr & 0xfff;
    if(dma_addr <= 0x0f0 || (dma_addr >= 0x500 && dma_addr < 0x570))
    {
        u32 channel = (dma_addr >> 4) & 0x7;
        if(dma_addr >= 0x500 && dma_addr < 0x570) channel += 8;
        switch(dma_addr & 0xc)
        {
            case 0x0:
            {
                channels[channel].addr = data;
                break;
            }
            case 0x4:
            {
                channels[channel].block_size = data & 0xffff;
                channels[channel].word_count = data >> 16;
                break;
            }
            case 0x8:
            {
                channels[channel].control.whole = data;
                break;
            }
            case 0xc:
            {
                channels[channel].tag_addr = data;
                break;
            }
        }
    }
    switch(dma_addr)
    {
        case 0x0f0:
        {
            for(u32 i = 0; i < 8; i++)
            {
                bool old_enable = dpcr.enable[i];
                dpcr.priorities[i] = (data >> (i << 2)) & 7;
                dpcr.enable[i] = (data & (1 << ((i << 2) + 3))) ? true : false;
                if(!old_enable && dpcr.enable[i]) channels[i].tag_end = false;
            }
            break;
        }
        case 0x0f4:
        {
            dicr.mask[0] = (data >> 16) & 0x7f;
            dicr.master_int_enable[0] = (data & (1 << 23)) ? true : false;
            dicr.status[0] &= ~((data >> 24) & 0x7f);
            break;
        }
        case 0x570:
        {
            for(u32 i = 8; i < 16; i++)
            {
                int bit = i - 8;
                bool old_enable = dpcr.enable[i];
                dpcr.priorities[i] = (data >> (bit << 2)) & 7;
                dpcr.enable[i] = (data & (1 << ((bit << 2) + 3))) ? true : false;
                if(!old_enable && dpcr.enable[i]) channels[i].tag_end = false;
            }
            break;
        }
        case 0x574:
        {
            dicr.mask[1] = (data >> 16) & 0x7f;
            dicr.master_int_enable[1] = (data & (1 << 23)) ? true : false;
            dicr.status[1] &= ~((data >> 24) & 0x7f);
            break;
        }
    }
}