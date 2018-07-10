#include "iop_intc.h"

void iop_intc_t::init()
{
    irq_status = irq_mask = 0;
    irq_enable = false;
}

void iop_intc_t::tick()
{
}

u32 iop_intc_t::rw(u32 addr)
{
    u32 intc_addr = addr & 0xf;
    switch(intc_addr)
    {
        case 0x0: return irq_status;
        case 0x4: return irq_mask;
        case 0x8:
        {
            u32 ret = irq_enable;
            irq_enable = false;
            return ret;
        }
    }
    return 0;
}

void iop_intc_t::ww(u32 addr, u32 data)
{
    u32 intc_addr = addr & 0xf;
    switch(intc_addr)
    {
        case 0x0:
        {
            irq_status &= data;
            break;
        }
        case 0x4:
        {
            irq_mask = data;
            break;
        }
        case 0x8:
        {
            irq_enable = data & 1;
            break;
        }
    }
    bool irq = irq_enable && (irq_status & irq_mask);
    if(irq) iop->irq_modify(false, irq);
}