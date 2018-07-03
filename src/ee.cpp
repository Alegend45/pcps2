#include "ee.h"

void ee_cpu::init()
{
    for(int i = 0; i < 32; i++)
    {
        r[i] = 0;
        rhi[i] = 0;
    }

    pc = 0xbfc00000;
    inc_pc = true;
    delay_slot = 0;
    branch_on = false;

    cop0_count = 0;
}

//TODO: This MMU emulation is COMPLETELY inaccurate, but it's good enough for now :/
u32 ee_cpu::translate_addr(u32 addr)
{
    if(addr >= 0x70000000 && addr < 0x70004000) return addr;
    if(addr >= 0x30100000 && addr < 0x32000000) return addr - 0x10000000;
    else return addr & 0x1fffffff;
}

u8 ee_cpu::rb(u32 addr)
{
    u32 phys_addr = translate_addr(addr);
    return rb_real(device, phys_addr);
}

u16 ee_cpu::rh(u32 addr)
{
    u32 phys_addr = translate_addr(addr);
    return rh_real(device, phys_addr);
}

u32 ee_cpu::rw(u32 addr)
{
    u32 phys_addr = translate_addr(addr);
    return rw_real(device, phys_addr);
}

u64 ee_cpu::rd(u32 addr)
{
    u32 phys_addr = translate_addr(addr);
    return rd_real(device, phys_addr);
}

u128 ee_cpu::rq(u32 addr)
{
    u32 phys_addr = translate_addr(addr);
    return rq_real(device, phys_addr);
}

void ee_cpu::wb(u32 addr, u8 data)
{
    u32 phys_addr = translate_addr(addr);
    wb_real(device, phys_addr, data);
}

void ee_cpu::wh(u32 addr, u16 data)
{
    u32 phys_addr = translate_addr(addr);
    wh_real(device, phys_addr, data);
}

void ee_cpu::ww(u32 addr, u32 data)
{
    u32 phys_addr = translate_addr(addr);
    ww_real(device, phys_addr, data);
}

void ee_cpu::wd(u32 addr, u64 data)
{
    u32 phys_addr = translate_addr(addr);
    wd_real(device, phys_addr, data);
}

void ee_cpu::wq(u32 addr, u128 data)
{
    u32 phys_addr = translate_addr(addr);
    wq_real(device, phys_addr, data);
}

void ee_cpu::tick()
{
    u32 opcode = rw(pc);
    printf("[EE] Opcode: %08x\n[EE] PC: %08x\n", opcode, pc);
    for(int i = 0; i < 32; i++)
    {
        printf("[EE] R%d: %016" PRIx64 "%016" PRIx64 "\n", i, rhi[i], r[i]);
    }

#define printf(fmt, ...)

    switch(opcode >> 26)
    {
        case 0x00:
        {
            switch(opcode & 0x3f)
            {
                case 0x00:
                {
                    printf("[EE] SLL\n");
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    int sa = (opcode >> 6) & 0x1f;
                    s32 temp = (u32)r[rt] << sa;
                    if(rd) r[rd] = (u64)(s64)temp;
                    break;
                }
                case 0x02:
                {
                    printf("[EE] SRL\n");
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    int sa = (opcode >> 6) & 0x1f;
                    s32 temp = (u32)r[rt] >> sa;
                    if(rd) r[rd] = (u64)(s64)temp;
                    break;
                }
                case 0x03:
                {
                    printf("[EE] SRA\n");
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    int sa = (opcode >> 6) & 0x1f;
                    s32 temp = (s32)r[rt] >> sa;
                    if(rd) r[rd] = (u64)(s64)temp;
                    break;
                }
                case 0x04:
                {
                    printf("[EE] SLLV\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    s32 temp = (u32)r[rt] << (r[rs] & 0x1f);
                    if(rd) r[rd] = (u64)(s64)temp;
                    break;
                }
                case 0x06:
                {
                    printf("[EE] SRLV\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    s32 temp = (u32)r[rt] >> (r[rs] & 0x1f);
                    if(rd) r[rd] = (u64)(s64)temp;
                    break;
                }
                case 0x07:
                {
                    printf("[EE] SRAV\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    s32 temp = (s32)r[rt] >> (r[rs] & 0x1f);
                    if(rd) r[rd] = (u64)(s64)temp;
                    break;
                }
                case 0x08:
                {
                    printf("[EE] JR\n");
                    int rs = (opcode >> 21) & 0x1f;
                    branch_on = true;
                    newpc = (u32)r[rs];
                    delay_slot = 1;
                    break;
                }
                case 0x09:
                {
                    printf("[EE] JALR\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    u32 return_addr = pc + 8;
                    branch_on = true;
                    newpc = (u32)r[rs];
                    delay_slot = 1;
                    if(rd) r[rd] = return_addr;
                    break;
                }
                case 0x0a:
                {
                    printf("[EE] MOVZ\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    if(!r[rt])
                    {
                        if(rd) r[rd] = r[rs];
                    }
                    break;
                }
                case 0x0b:
                {
                    printf("[EE] MOVN\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    if(r[rt])
                    {
                        if(rd) r[rd] = r[rs];
                    }
                    break;
                }
                case 0x0f:
                {
                    printf("[EE] SYNC\n");
                    int sync_type = (opcode >> 6) & 0x1f;
                    if(sync_type & 0x10)
                    {
                        //SYNC.P
                    }
                    else
                    {
                        //SYNC.L
                    }
                    break;
                }
                case 0x10:
                {
                    printf("[EE] MFHI\n");
                    int rd = (opcode >> 11) & 0x1f;
                    if(rd) r[rd] = hi;
                    break;
                }
                case 0x11:
                {
                    printf("[EE] MTHI\n");
                    int rs = (opcode >> 21) & 0x1f;
                    hi = r[rs];
                    break;
                }
                case 0x12:
                {
                    printf("[EE] MFLO\n");
                    int rd = (opcode >> 11) & 0x1f;
                    if(rd) r[rd] = lo;
                    break;
                }
                case 0x13:
                {
                    printf("[EE] MTLO\n");
                    int rs = (opcode >> 21) & 0x1f;
                    lo = r[rs];
                    break;
                }
                case 0x14:
                {
                    printf("[EE] DSLLV\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    if(rd) r[rd] = r[rt] << (r[rs] & 0x3f);
                    break;
                }
                case 0x16:
                {
                    printf("[EE] DSRLV\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    if(rd) r[rd] = r[rt] >> (r[rs] & 0x3f);
                    break;
                }
                case 0x17:
                {
                    printf("[EE] DSRAV\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    if(rd) r[rd] = (s64)r[rt] >> (r[rs] & 0x3f);
                    break;
                }
                case 0x18:
                {
                    printf("[EE] MULT\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    s64 result = (s64)(s32)r[rs] * (s64)(s32)r[rt];
                    s64 result_lo = (s32)result;
                    s64 result_hi = (s32)(result >> 32);
                    lo = result_lo;
                    hi = result_hi;
                    if(rd) r[rd] = lo;
                    break;
                }
                case 0x19:
                {
                    printf("[EE] MULTU\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    u64 result = (u64)(u32)r[rs] * (u64)(u32)r[rt];
                    u32 result_lo = (u32)result;
                    u32 result_hi = result >> 32;
                    lo = result_lo;
                    hi = result_hi;
                    if(rd) r[rd] = lo;
                    break;
                }
                case 0x1a:
                {
                    printf("[EE] DIV\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    if(!(s32)r[rt])
                    {
                        hi = r[rs];
                        if((s32)r[rs] > 0x80000000) lo = 1;
                        else lo = 0xffffffff;
                    }
                    else if((u32)r[rs] == 0x80000000 && (u32)r[rt] == 0xffffffff)
                    {
                        lo = 0x80000000;
                        hi = 0;
                    }
                    else
                    {
                        lo = (s32)r[rs] / (s32)r[rt];
                        hi = (s32)r[rs] % (s32)r[rt];
                    }
                    break;
                }
                case 0x1b:
                {
                    printf("[EE] DIVU\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    if(!(u32)r[rt])
                    {
                        lo = 0xffffffff;
                        hi = (u32)r[rs];
                    }
                    else
                    {
                        lo = (u32)r[rs] / (u32)r[rt];
                        hi = (u32)r[rs] % (u32)r[rt];
                    }
                    break;
                }
                case 0x20:
                {
                    printf("[EE] ADD\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    s32 result = (s64)r[rs] + (s64)r[rt];
                    if(rd) r[rd] = (s64)result;
                    break;
                }
                case 0x21:
                {
                    printf("[EE] ADDU\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    s32 result = (s64)r[rs] + (s64)r[rt];
                    if(rd) r[rd] = (s64)result;
                    break;
                }
                case 0x22:
                {
                    printf("[EE] SUB\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    s32 result = (s64)r[rs] - (s64)r[rt];
                    if(rd) r[rd] = (s64)result;
                    break;
                }
                case 0x23:
                {
                    printf("[EE] SUBU\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    s32 result = (s64)r[rs] - (s64)r[rt];
                    if(rd) r[rd] = (s64)result;
                    break;
                }
                case 0x24:
                {
                    printf("[EE] AND\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    if(rd) r[rd] = r[rs] & r[rt];
                    break;
                }
                case 0x25:
                {
                    printf("[EE] OR\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    if(rd) r[rd] = r[rs] | r[rt];
                    break;
                }
                case 0x26:
                {
                    printf("[EE] XOR\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    if(rd) r[rd] = r[rs] ^ r[rt];
                    break;
                }
                case 0x27:
                {
                    printf("[EE] NOR\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    if(rd) r[rd] = ~(r[rs] | r[rt]);
                    break;
                }
                case 0x28:
                {
                    printf("[EE] MFSA\n");
                    int rd = (opcode >> 11) & 0x1f;
                    if(rd) r[rd] = ee_sa;
                    break;
                }
                case 0x29:
                {
                    printf("[EE] MTSA\n");
                    int rs = (opcode >> 21) & 0x1f;
                    ee_sa = r[rs];
                    break;
                }
                case 0x2a:
                {
                    printf("[EE] SLT\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    if(rd)
                    {
                        if((s64)r[rs] < (s64)r[rt]) r[rd] = 1;
                        else r[rd] = 0;
                    }
                    break;
                }
                case 0x2b:
                {
                    printf("[EE] SLTU\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    if(rd)
                    {
                        if(r[rs] < r[rt]) r[rd] = 1;
                        else r[rd] = 0;
                    }
                    break;
                }
                case 0x2c:
                {
                    printf("[EE] DADD\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    if(rd) r[rd] = (s64)r[rs] + (s64)r[rt];
                    break;
                }
                case 0x2d:
                {
                    printf("[EE] DADDU\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    if(rd) r[rd] = (s64)r[rs] + (s64)r[rt];
                    break;
                }
                case 0x2e:
                {
                    printf("[EE] DSUB\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    if(rd) r[rd] = (s64)r[rs] - (s64)r[rt];
                    break;
                }
                case 0x2f:
                {
                    printf("[EE] DSUBU\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    if(rd) r[rd] = (s64)r[rs] - (s64)r[rt];
                    break;
                }
                case 0x38:
                {
                    printf("[EE] DSLL\n");
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    int sa = (opcode >> 6) & 0x1f;
                    if(rd) r[rd] = r[rt] << sa;
                    break;
                }
                case 0x3a:
                {
                    printf("[EE] DSRL\n");
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    int sa = (opcode >> 6) & 0x1f;
                    if(rd) r[rd] = r[rt] >> sa;
                    break;
                }
                case 0x3b:
                {
                    printf("[EE] DSRA\n");
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    int sa = (opcode >> 6) & 0x1f;
                    if(rd) r[rd] = (s64)r[rt] >> sa;
                    break;
                }
                case 0x3c:
                {
                    printf("[EE] DSLL32\n");
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    int sa = (opcode >> 6) & 0x1f;
                    sa += 32;
                    if(rd) r[rd] = r[rt] << sa;
                    break;
                }
                case 0x3e:
                {
                    printf("[EE] DSRL32\n");
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    int sa = (opcode >> 6) & 0x1f;
                    sa += 32;
                    if(rd) r[rd] = r[rt] >> sa;
                    break;
                }
                case 0x3f:
                {
                    printf("[EE] DSRA32\n");
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    int sa = (opcode >> 6) & 0x1f;
                    sa += 32;
                    if(rd) r[rd] = (s64)r[rt] >> sa;
                    break;
                }
            }
            break;
        }
        case 0x01:
        {
            switch((opcode >> 16) & 0x1f)
            {
                case 0x00:
                {
                    printf("[EE] BLTZ\n");
                    int rs = (opcode >> 21) & 0x1f;
                    s32 offset = (s16)(opcode & 0xffff);
                    offset <<= 2;
                    if((s64)r[rs] < 0)
                    {
                        branch_on = true;
                        newpc = pc + offset + 4;
                        delay_slot = 1;
                    }
                    break;
                }
                case 0x01:
                {
                    printf("[EE] BGEZ\n");
                    int rs = (opcode >> 21) & 0x1f;
                    s32 offset = (s16)(opcode & 0xffff);
                    offset <<= 2;
                    if((s64)r[rs] >= 0)
                    {
                        branch_on = true;
                        newpc = pc + offset + 4;
                        delay_slot = 1;
                    }
                    break;
                }
                case 0x02:
                {
                    printf("[EE] BLTZL\n");
                    int rs = (opcode >> 21) & 0x1f;
                    s32 offset = (s16)(opcode & 0xffff);
                    offset <<= 2;
                    if((s64)r[rs] < 0)
                    {
                        branch_on = true;
                        newpc = pc + offset + 4;
                        delay_slot = 1;
                    }
                    else pc += 4;
                    break;
                }
                case 0x03:
                {
                    printf("[EE] BGEZL\n");
                    int rs = (opcode >> 21) & 0x1f;
                    s32 offset = (s16)(opcode & 0xffff);
                    offset <<= 2;
                    if((s64)r[rs] >= 0)
                    {
                        branch_on = true;
                        newpc = pc + offset + 4;
                        delay_slot = 1;
                    }
                    else pc += 4;
                    break;
                }
                case 0x10:
                {
                    printf("[EE] BLTZAL\n");
                    int rs = (opcode >> 21) & 0x1f;
                    s32 offset = (s16)(opcode & 0xffff);
                    offset <<= 2;
                    if((s64)r[rs] < 0)
                    {
                        branch_on = true;
                        newpc = pc + offset + 4;
                        delay_slot = 1;
                        r[31] = pc + 8;
                    }
                    break;
                }
                case 0x11:
                {
                    printf("[EE] BGEZAL\n");
                    int rs = (opcode >> 21) & 0x1f;
                    s32 offset = (s16)(opcode & 0xffff);
                    offset <<= 2;
                    if((s64)r[rs] >= 0)
                    {
                        branch_on = true;
                        newpc = pc + offset + 4;
                        delay_slot = 1;
                        r[31] = pc + 8;
                    }
                    break;
                }
                case 0x12:
                {
                    printf("[EE] BLTZALL\n");
                    int rs = (opcode >> 21) & 0x1f;
                    s32 offset = (s16)(opcode & 0xffff);
                    offset <<= 2;
                    if((s64)r[rs] < 0)
                    {
                        branch_on = true;
                        newpc = pc + offset + 4;
                        delay_slot = 1;
                        r[31] = pc + 8;
                    }
                    else pc += 4;
                    break;
                }
                case 0x13:
                {
                    printf("[EE] BGEZALL\n");
                    int rs = (opcode >> 21) & 0x1f;
                    s32 offset = (s16)(opcode & 0xffff);
                    offset <<= 2;
                    if((s64)r[rs] >= 0)
                    {
                        branch_on = true;
                        newpc = pc + offset + 4;
                        delay_slot = 1;
                        r[31] = pc + 8;
                    }
                    else pc += 4;
                    break;
                }
                case 0x18:
                {
                    printf("[EE] MTSAB\n");
                    int rs = (opcode >> 21) & 0x1f;
                    u16 imm = opcode & 0xffff;
                    ee_sa = ((r[rs] & 0xf) | (imm & 0xf)) << 3;
                    break;
                }
                case 0x19:
                {
                    printf("[EE] MTSAH\n");
                    int rs = (opcode >> 21) & 0x1f;
                    u16 imm = opcode & 0xffff;
                    ee_sa = ((r[rs] & 0x7) | (imm & 0x7)) << 4;
                    break;
                }
            }
            break;
        }
        case 0x02:
        {
            printf("[EE] J\n");
            u32 addr = (opcode & 0x3ffffff) << 2;
            addr += (pc + 4) & 0xf0000000;
            branch_on = true;
            newpc = addr;
            delay_slot = 1;
            break;
        }
        case 0x03:
        {
            printf("[EE] JAL\n");
            u32 return_addr = pc;
            u32 addr = (opcode & 0x3ffffff) << 2;
            addr += (pc + 4) & 0xf0000000;
            branch_on = true;
            newpc = addr;
            delay_slot = 1;
            r[31] = return_addr + 8;
            break;
        }
        case 0x04:
        {
            printf("[EE] BEQ\n");
            int rs = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            s32 offset = (s16)(opcode & 0xffff);
            offset <<= 2;
            if(r[rt] == r[rs])
            {
                branch_on = true;
                newpc = pc + offset + 4;
                delay_slot = 1;
            }
            break;
        }
        case 0x05:
        {
            printf("[EE] BNE\n");
            int rs = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            s32 offset = (s16)(opcode & 0xffff);
            offset <<= 2;
            if(r[rt] != r[rs])
            {
                branch_on = true;
                newpc = pc + offset + 4;
                delay_slot = 1;
            }
            break;
        }
        case 0x06:
        {
            printf("[EE] BLEZ\n");
            int rs = (opcode >> 21) & 0x1f;
            s32 offset = (s16)(opcode & 0xffff);
            offset <<= 2;
            if((s64)r[rs] <= 0)
            {
                branch_on = true;
                newpc = pc + offset + 4;
                delay_slot = 1;
            }
            break;
        }
        case 0x07:
        {
            printf("[EE] BGTZ\n");
            int rs = (opcode >> 21) & 0x1f;
            s32 offset = (s16)(opcode & 0xffff);
            offset <<= 2;
            if((s64)r[rs] > 0)
            {
                branch_on = true;
                newpc = pc + offset + 4;
                delay_slot = 1;
            }
            break;
        }
        case 0x08:
        {
            printf("[EE] ADDI\n");
            int rs = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            s32 imm = (s16)(opcode & 0xffff);
            if(rt)
            {
                s64 temp = (s32)((u32)r[rs] + imm);
                r[rt] = (s64)temp;
            }
            break;
        }
        case 0x09:
        {
            printf("[EE] ADDIU\n");
            int rs = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            s32 imm = (s16)(opcode & 0xffff);
            if(rt)
            {
                s64 temp = (s32)((u32)r[rs] + imm);
                r[rt] = (s64)temp;
            }
            break;
        }
        case 0x0a:
        {
            printf("[EE] SLTI\n");
            int rs = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            s64 imm = (s16)(opcode & 0xffff);
            if(rt)
            {
                if((s64)r[rs] < imm) r[rt] = 1;
                else r[rt] = 0;
            }
            break;
        }
        case 0x0b:
        {
            printf("[EE] SLTIU\n");
            int rs = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            s64 imm = (s16)(opcode & 0xffff);
            if(rt)
            {
                if(r[rs] < (u64)imm) r[rt] = 1;
                else r[rt] = 0;
            }
            break;
        }
        case 0x0c:
        {
            printf("[EE] ANDI\n");
            int rs = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            u16 imm = opcode & 0xffff;
            if(rt) r[rt] = r[rs] & imm;
            break;
        }
        case 0x0d:
        {
            printf("[EE] ORI\n");
            int rs = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            u16 imm = opcode & 0xffff;
            if(rt) r[rt] = r[rs] | imm;
            break;
        }
        case 0x0e:
        {
            printf("[EE] XORI\n");
            int rs = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            u16 imm = opcode & 0xffff;
            if(rt) r[rt] = r[rs] ^ imm;
            break;
        }
        case 0x0f:
        {
            printf("[EE] LUI\n");
            int rt = (opcode >> 16) & 0x1f;
            s64 imm = (s32)((opcode & 0xffff) << 16);
            if(rt) r[rt] = imm;
            break;
        }
        case 0x10:
        {
            switch((opcode >> 21) & 0x1f)
            {
                case 0x00:
                {
                    printf("[EE] MFC0\n");
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    switch(rd)
                    {
                        case 0x09:
                        {
                            if(rt) r[rt] = cop0_count;
                            break;
                        }
                        case 0x0c:
                        {
                            if(rt) r[rt] = cop0_status.whole;
                            break;
                        }
                        case 0x0f:
                        {
                            if(rt) r[rt] = 0x00002e20; //TODO: PCSX2 value. VERIFY!
                            break;
                        }
                        case 0x10:
                        {
                            if(rt) r[rt] = cop0_config.whole;
                            break;
                        }
                    }
                    break;
                }
                case 0x04:
                {
                    printf("[EE] MTC0\n");
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    switch(rd)
                    {
                        case 0x09:
                        {
                            cop0_count = r[rt];
                            break;
                        }
                        case 0x0c:
                        {
                            cop0_status.whole = r[rt];
                            interrupt_mask &= 0x73;
                            interrupt_mask |= cop0_status.interrupt_mask1 << 2;
                            interrupt_mask |= cop0_status.interrupt_mask2 << 7;
                            break;
                        }
                        case 0x10:
                        {
                            cop0_config.whole = r[rt];
                            break;
                        }
                    }
                    break;
                }
                case 0x08:
                {
                    switch((opcode >> 16) & 0x1f)
                    {
                        case 0x00:
                        {
                            printf("[EE] BC0F\n");
                            break;
                        }
                        case 0x01:
                        {
                            printf("[EE] BC0T\n");
                            break;
                        }
                        case 0x02:
                        {
                            printf("[EE] BC0FL\n");
                            break;
                        }
                        case 0x03:
                        {
                            printf("[EE] BC0TL\n");
                            break;
                        }
                    }
                    break;
                }
                case 0x10:
                {
                    switch(opcode & 0x3f)
                    {
                        case 0x01:
                        {
                            printf("[EE] TLBR\n");
                            break;
                        }
                        case 0x02:
                        {
                            printf("[EE] TLBWI\n");
                            break;
                        }
                        case 0x06:
                        {
                            printf("[EE] TLBWR\n");
                            break;
                        }
                        case 0x18:
                        {
                            printf("[EE] ERET\n");
                            break;
                        }
                        case 0x38:
                        {
                            printf("[EE] EI\n");
                            break;
                        }
                        case 0x39:
                        {
                            printf("[EE] DI\n");
                            break;
                        }
                    }
                    break;
                }
            }
            break;
        }
        case 0x14:
        {
            printf("[EE] BEQL\n");
            int rs = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            s32 offset = (s16)(opcode & 0xffff);
            offset <<= 2;
            if(r[rt] == r[rs])
            {
                branch_on = true;
                newpc = pc + offset + 4;
                delay_slot = 1;
            }
            else pc += 4;
            break;
        }
        case 0x15:
        {
            printf("[EE] BNEL\n");
            int rs = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            s32 offset = (s16)(opcode & 0xffff);
            offset <<= 2;
            if(r[rt] != r[rs])
            {
                branch_on = true;
                newpc = pc + offset + 4;
                delay_slot = 1;
            }
            else pc += 4;
            break;
        }
        case 0x16:
        {
            printf("[EE] BLEZL\n");
            int rs = (opcode >> 21) & 0x1f;
            s32 offset = (s16)(opcode & 0xffff);
            offset <<= 2;
            if((s64)r[rs] <= 0)
            {
                branch_on = true;
                newpc = pc + offset + 4;
                delay_slot = 1;
            }
            else pc += 4;
            break;
        }
        case 0x17:
        {
            printf("[EE] BGTZL\n");
            int rs = (opcode >> 21) & 0x1f;
            s32 offset = (s16)(opcode & 0xffff);
            offset <<= 2;
            if((s64)r[rs] > 0)
            {
                branch_on = true;
                newpc = pc + offset + 4;
                delay_slot = 1;
            }
            else pc += 4;
            break;
        }
        case 0x18:
        {
            printf("[EE] DADDI\n");
            int rs = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            s32 imm = (s16)(opcode & 0xffff);
            if(rt)
            {
                r[rt] = (s64)r[rs] + imm;
            }
            break;
        }
        case 0x19:
        {
            printf("[EE] DADDIU\n");
            int rs = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            s32 imm = (s16)(opcode & 0xffff);
            if(rt)
            {
                r[rt] = (s64)r[rs] + imm;
            }
            break;
        }
        case 0x1a:
        {
            printf("[EE] LDL\n");
            const u64 ldl_mask[8] = {0x00ffffffffffffffULL, 0x0000ffffffffffffULL, 0x000000ffffffffff, 0xffffffffULL,
            0x00ffffffULL, 0x0000ffffULL, 0xff, 0};
            const u8 ldl_shift[8] = {56, 48, 40, 32, 24, 16, 8, 0};
            int base = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            s32 offset = (s16)(opcode & 0xffff);
            u32 addr = r[base] + offset;
            int shift = addr & 7;
            if(rt)
            {
                u64 data = rd(addr & ~7);
                r[rt] = (r[rt] & ldl_mask[shift]) | (data << ldl_shift[shift]);
            }
            break;
        }
        case 0x1b:
        {
            printf("[EE] LDR\n");
            const u64 ldr_mask[8] = {0, 0xff00000000000000ULL, 0xffff000000000000ULL, 0xffffff0000000000ULL,
            0xffffffff00000000ULL, 0xffffffffff000000ULL, 0xffffffffffff0000ULL, 0xffffffffffffff00ULL};
            const u8 ldr_shift[8] = {0, 8, 16, 24, 32, 40, 48, 56};
            int base = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            s32 offset = (s16)(opcode & 0xffff);
            u32 addr = r[base] + offset;
            int shift = addr & 7;
            if(rt)
            {
                u64 data = rd(addr & ~7);
                r[rt] = (r[rt] & ldr_mask[shift]) | (data >> ldr_shift[shift]);;
            }
            break;
        }
        case 0x1c:
        {
            switch(opcode & 0x3f)
            {
                case 0x00:
                {
                    printf("[EE] MADD\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    s64 result = (s64)(s32)r[rs] * (s64)(s32)r[rt];
                    u64 lo_hi = (u32)lo | ((u64)(u32)hi << 32);
                    result += lo_hi;
                    s64 result_lo = (s32)result;
                    s64 result_hi = (s32)(result >> 32);
                    lo = result_lo;
                    hi = result_hi;
                    if(rd) r[rd] = lo;
                    break;
                }
                case 0x01:
                {
                    printf("[EE] MADDU\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    s64 result = (u64)(u32)r[rs] * (u64)(u32)r[rt];
                    u64 lo_hi = (u32)lo | ((u64)(u32)hi << 32);
                    result = (u64)result + lo_hi;
                    s64 result_lo = (s32)result;
                    s64 result_hi = (s32)(result >> 32);
                    lo = result_lo;
                    hi = result_hi;
                    if(rd) r[rd] = lo;
                    break;
                }
                case 0x04:
                {
                    printf("[EE] PLZCW\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    u32 words[2];
                    words[0] = (u32)r[rs];
                    words[1] = r[rs] >> 32;

                    for(int i = 0; i < 2; i++)
                    {
                        bool high_bit = words[i] >> 31;
                        u8 bits = 0;
                        for(int j = 30; j >= 0; j--)
                        {
                            if((words[i] & (1 << j)) == (high_bit << j)) bits++;
                            else break;
                        }
                        words[i] = bits;
                    }

                    if(rd) r[rd] = words[0] | ((u64)words[1] << 32);
                    break;
                }
                case 0x08:
                {
                    switch((opcode >> 6) & 0x1f)
                    {
                        case 0x00:
                        {
                            printf("[EE] PADDW\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            u32 words_rs[4];
                            u32 words_rt[4];
                            u32 words_rd[4];
                            *(u64*)words_rs = r[rs];
                            *(u64*)(words_rs + 2) = rhi[rs];
                            *(u64*)words_rt = r[rt];
                            *(u64*)(words_rt + 2) = rhi[rt];

                            for(int i = 0; i < 4; i++)
                            {
                                words_rd[i] = words_rs[i] + words_rt[i];
                            }

                            if(rd)
                            {
                                r[rd] = *(u64*)words_rd;
                                rhi[rd] = *(u64*)(words_rd + 2);
                            }
                            break;
                        }
                        case 0x01:
                        {
                            printf("[EE] PSUBW\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            s32 words_rs[4];
                            s32 words_rt[4];
                            s32 words_rd[4];
                            *(u64*)words_rs = r[rs];
                            *(u64*)(words_rs + 2) = rhi[rs];
                            *(u64*)words_rt = r[rt];
                            *(u64*)(words_rt + 2) = rhi[rt];

                            for(int i = 0; i < 4; i++)
                            {
                                words_rd[i] = words_rs[i] - words_rt[i];
                            }

                            if(rd)
                            {
                                r[rd] = *(u64*)words_rd;
                                rhi[rd] = *(u64*)(words_rd + 2);
                            }
                            break;
                        }
                        case 0x02:
                        {
                            printf("[EE] PCGTW\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            s32 words_rs[4];
                            s32 words_rt[4];
                            s32 words_rd[4];
                            *(u64*)words_rs = r[rs];
                            *(u64*)(words_rs + 2) = rhi[rs];
                            *(u64*)words_rt = r[rt];
                            *(u64*)(words_rt + 2) = rhi[rt];

                            for(int i = 0; i < 4; i++)
                            {
                                words_rd[i] = (words_rs[i] > words_rt[i]) ? ~0 : 0;
                            }

                            if(rd)
                            {
                                r[rd] = *(u64*)words_rd;
                                rhi[rd] = *(u64*)(words_rd + 2);
                            }
                            break;
                        }
                        case 0x03:
                        {
                            printf("[EE] PMAXW\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            s32 words_rs[4];
                            s32 words_rt[4];
                            s32 words_rd[4];
                            *(u64*)words_rs = r[rs];
                            *(u64*)(words_rs + 2) = rhi[rs];
                            *(u64*)words_rt = r[rt];
                            *(u64*)(words_rt + 2) = rhi[rt];

                            for(int i = 0; i < 4; i++)
                            {
                                words_rd[i] = (words_rs[i] > words_rt[i]) ? r[rs] : r[rt];
                            }

                            if(rd)
                            {
                                r[rd] = *(u64*)words_rd;
                                rhi[rd] = *(u64*)(words_rd + 2);
                            }
                            break;
                        }
                        case 0x04:
                        {
                            printf("[EE] PADDH\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            u16 halfwords_rs[8];
                            u16 halfwords_rt[8];
                            u16 halfwords_rd[8];
                            *(u64*)halfwords_rs = r[rs];
                            *(u64*)(halfwords_rs + 4) = rhi[rs];
                            *(u64*)halfwords_rt = r[rt];
                            *(u64*)(halfwords_rt + 4) = rhi[rt];

                            for(int i = 0; i < 8; i++)
                            {
                                halfwords_rd[i] = halfwords_rs[i] + halfwords_rt[i];
                            }

                            if(rd)
                            {
                                r[rd] = *(u64*)halfwords_rd;
                                rhi[rd] = *(u64*)(halfwords_rd + 4);
                            }
                            break;
                        }
                        case 0x05:
                        {
                            printf("[EE] PSUBH\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            s16 halfwords_rs[8];
                            s16 halfwords_rt[8];
                            s16 halfwords_rd[8];
                            *(u64*)halfwords_rs = r[rs];
                            *(u64*)(halfwords_rs + 4) = rhi[rs];
                            *(u64*)halfwords_rt = r[rt];
                            *(u64*)(halfwords_rt + 4) = rhi[rt];

                            for(int i = 0; i < 8; i++)
                            {
                                halfwords_rd[i] = halfwords_rs[i] - halfwords_rt[i];
                            }

                            if(rd)
                            {
                                r[rd] = *(u64*)halfwords_rd;
                                rhi[rd] = *(u64*)(halfwords_rd + 4);
                            }
                            break;
                        }
                        case 0x06:
                        {
                            printf("[EE] PCGTH\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            s16 halfwords_rs[8];
                            s16 halfwords_rt[8];
                            s16 halfwords_rd[8];
                            *(u64*)halfwords_rs = r[rs];
                            *(u64*)(halfwords_rs + 4) = rhi[rs];
                            *(u64*)halfwords_rt = r[rt];
                            *(u64*)(halfwords_rt + 4) = rhi[rt];

                            for(int i = 0; i < 8; i++)
                            {
                                halfwords_rd[i] = (halfwords_rs[i] > halfwords_rt[i]) ? ~0 : 0;
                            }

                            if(rd)
                            {
                                r[rd] = *(u64*)halfwords_rd;
                                rhi[rd] = *(u64*)(halfwords_rd + 4);
                            }
                            break;
                        }
                        case 0x07:
                        {
                            printf("[EE] PMAXH\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            s16 halfwords_rs[8];
                            s16 halfwords_rt[8];
                            s16 halfwords_rd[8];
                            *(u64*)halfwords_rs = r[rs];
                            *(u64*)(halfwords_rs + 4) = rhi[rs];
                            *(u64*)halfwords_rt = r[rt];
                            *(u64*)(halfwords_rt + 4) = rhi[rt];

                            for(int i = 0; i < 8; i++)
                            {
                                halfwords_rd[i] = (halfwords_rs[i] > halfwords_rt[i]) ? r[rs] : r[rt];
                            }

                            if(rd)
                            {
                                r[rd] = *(u64*)halfwords_rd;
                                rhi[rd] = *(u64*)(halfwords_rd + 4);
                            }
                            break;
                        }
                        case 0x08:
                        {
                            printf("[EE] PADDB\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            u8 bytes_rs[16];
                            u8 bytes_rt[16];
                            u8 bytes_rd[16];
                            *(u64*)bytes_rs = r[rs];
                            *(u64*)(bytes_rs + 8) = rhi[rs];
                            *(u64*)bytes_rt = r[rt];
                            *(u64*)(bytes_rt + 8) = rhi[rt];

                            for(int i = 0; i < 16; i++)
                            {
                                bytes_rd[i] = bytes_rs[i] + bytes_rt[i];
                            }

                            if(rd)
                            {
                                r[rd] = *(u64*)bytes_rd;
                                rhi[rd] = *(u64*)(bytes_rd + 8);
                            }
                            break;
                        }
                        case 0x09:
                        {
                            printf("[EE] PSUBB\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            s8 bytes_rs[16];
                            s8 bytes_rt[16];
                            s8 bytes_rd[16];
                            *(u64*)bytes_rs = r[rs];
                            *(u64*)(bytes_rs + 8) = rhi[rs];
                            *(u64*)bytes_rt = r[rt];
                            *(u64*)(bytes_rt + 8) = rhi[rt];

                            for(int i = 0; i < 16; i++)
                            {
                                bytes_rd[i] = bytes_rs[i] - bytes_rt[i];
                            }

                            if(rd)
                            {
                                r[rd] = *(u64*)bytes_rd;
                                rhi[rd] = *(u64*)(bytes_rd + 8);
                            }
                            break;
                        }
                        case 0x0a:
                        {
                            printf("[EE] PCGTB\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            s8 bytes_rs[16];
                            s8 bytes_rt[16];
                            s8 bytes_rd[16];
                            *(u64*)bytes_rs = r[rs];
                            *(u64*)(bytes_rs + 8) = rhi[rs];
                            *(u64*)bytes_rt = r[rt];
                            *(u64*)(bytes_rt + 8) = rhi[rt];

                            for(int i = 0; i < 16; i++)
                            {
                                bytes_rd[i] = (bytes_rs[i] > bytes_rt[i]) ? ~0 : 0;
                            }

                            if(rd)
                            {
                                r[rd] = *(u64*)bytes_rd;
                                rhi[rd] = *(u64*)(bytes_rd + 8);
                            }
                            break;
                        }
                        case 0x10:
                        {
                            printf("[EE] PADDSW\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            s32 words_rs[4];
                            s32 words_rt[4];
                            s32 words_rd[4];
                            *(u64*)words_rs = r[rs];
                            *(u64*)(words_rs + 2) = rhi[rs];
                            *(u64*)words_rt = r[rt];
                            *(u64*)(words_rt + 2) = rhi[rt];

                            for(int i = 0; i < 4; i++)
                            {
                                words_rd[i] = words_rs[i] + words_rt[i];
                                if(words_rd[i] >= 0x80000000) words_rd[i] = 0x7fffffff;
                                else if(words_rd[i] < 0x80000000) words_rd[i] = 0x80000000;
                            }

                            if(rd)
                            {
                                r[rd] = *(u64*)words_rd;
                                rhi[rd] = *(u64*)(words_rd + 2);
                            }
                            break;
                        }
                        case 0x11:
                        {
                            printf("[EE] PSUBSW\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            s32 words_rs[4];
                            s32 words_rt[4];
                            s32 words_rd[4];
                            *(u64*)words_rs = r[rs];
                            *(u64*)(words_rs + 2) = rhi[rs];
                            *(u64*)words_rt = r[rt];
                            *(u64*)(words_rt + 2) = rhi[rt];

                            for(int i = 0; i < 4; i++)
                            {
                                words_rd[i] = words_rs[i] - words_rt[i];
                                if(words_rd[i] >= 0x80000000) words_rd[i] = 0x7fffffff;
                                else if(words_rd[i] < 0x80000000) words_rd[i] = 0x80000000;
                            }

                            if(rd)
                            {
                                r[rd] = *(u64*)words_rd;
                                rhi[rd] = *(u64*)(words_rd + 2);
                            }
                            break;
                        }
                        case 0x12:
                        {
                            printf("[EE] PEXTLW\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;

                            if(rd)
                            {
                                r[rd] = (u32)r[rt] | ((u64)(u32)r[rs] << 32);
                                rhi[rd] = (u32)(r[rt] >> 32) | ((r[rs] >> 32) << 32);
                            }
                            break;
                        }
                        case 0x13:
                        {
                            printf("[EE] PPACW\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;

                            if(rd)
                            {
                                r[rd] = (u32)r[rt] | ((u64)(u32)rhi[rt] << 32);
                                rhi[rd] = (u32)r[rs] | ((u64)(u32)rhi[rs] << 32);
                            }
                            break;
                        }
                        case 0x14:
                        {
                            printf("[EE] PADDSH\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            s16 halfwords_rs[8];
                            s16 halfwords_rt[8];
                            s16 halfwords_rd[8];
                            *(u64*)halfwords_rs = r[rs];
                            *(u64*)(halfwords_rs + 4) = rhi[rs];
                            *(u64*)halfwords_rt = r[rt];
                            *(u64*)(halfwords_rt + 4) = rhi[rt];

                            for(int i = 0; i < 8; i++)
                            {
                                halfwords_rd[i] = halfwords_rs[i] + halfwords_rt[i];
                                if(halfwords_rd[i] >= 0x8000) halfwords_rd[i] = 0x7fff;
                                else if(halfwords_rd[i] < 0x8000) halfwords_rd[i] = 0x8000;
                            }

                            if(rd)
                            {
                                r[rd] = *(u64*)halfwords_rd;
                                rhi[rd] = *(u64*)(halfwords_rd + 4);
                            }
                            break;
                        }
                        case 0x15:
                        {
                            printf("[EE] PSUBSH\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            s16 halfwords_rs[8];
                            s16 halfwords_rt[8];
                            s16 halfwords_rd[8];
                            *(u64*)halfwords_rs = r[rs];
                            *(u64*)(halfwords_rs + 4) = rhi[rs];
                            *(u64*)halfwords_rt = r[rt];
                            *(u64*)(halfwords_rt + 4) = rhi[rt];

                            for(int i = 0; i < 8; i++)
                            {
                                halfwords_rd[i] = halfwords_rs[i] - halfwords_rt[i];
                                if(halfwords_rd[i] >= 0x8000) halfwords_rd[i] = 0x7fff;
                                else if(halfwords_rd[i] < 0x8000) halfwords_rd[i] = 0x8000;
                            }

                            if(rd)
                            {
                                r[rd] = *(u64*)halfwords_rd;
                                rhi[rd] = *(u64*)(halfwords_rd + 4);
                            }
                            break;
                        }
                        case 0x16:
                        {
                            printf("[EE] PEXTLH\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            u16 halfwords_rs[4];
                            u16 halfwords_rt[4];
                            u32 words_rd[4];
                            *(u64*)halfwords_rs = r[rs];
                            *(u64*)halfwords_rt = r[rt];

                            for(int i = 0; i < 4; i++)
                            {
                                words_rd[i] = halfwords_rt[i] | ((u32)halfwords_rs[i] << 16);
                            }

                            if(rd)
                            {
                                r[rd] = *(u64*)words_rd;
                                rhi[rd] = *(u64*)(words_rd + 2);
                            }
                            break;
                        }
                        case 0x17:
                        {
                            printf("[EE] PPACH\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            u16 halfwords_rs[8];
                            u16 halfwords_rt[8];
                            u16 halfwords_rd[8];
                            *(u64*)halfwords_rs = r[rs];
                            *(u64*)(halfwords_rs + 4) = rhi[rs];
                            *(u64*)halfwords_rt = r[rt];
                            *(u64*)(halfwords_rt + 4) = rhi[rt];

                            for(int i = 0; i < 4; i++)
                            {
                                halfwords_rd[i] = halfwords_rt[i << 1];
                                halfwords_rd[i + 4] = halfwords_rs[i << 1];
                            }

                            if(rd)
                            {
                                r[rd] = *(u64*)halfwords_rd;
                                rhi[rd] = *(u64*)(halfwords_rd + 4);
                            }
                            break;
                        }
                        case 0x18:
                        {
                            printf("[EE] PADDSB\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            s8 bytes_rs[16];
                            s8 bytes_rt[16];
                            s8 bytes_rd[16];
                            *(u64*)bytes_rs = r[rs];
                            *(u64*)(bytes_rs + 8) = rhi[rs];
                            *(u64*)bytes_rt = r[rt];
                            *(u64*)(bytes_rt + 8) = rhi[rt];

                            for(int i = 0; i < 16; i++)
                            {
                                bytes_rd[i] = bytes_rs[i] + bytes_rt[i];
                                if(bytes_rd[i] >= 0x80) bytes_rd[i] = 0x7f;
                                else if(bytes_rd[i] < 0x80) bytes_rd[i] = 0x80;
                            }

                            if(rd)
                            {
                                r[rd] = *(u64*)bytes_rd;
                                rhi[rd] = *(u64*)(bytes_rd + 8);
                            }
                            break;
                        }
                        case 0x19:
                        {
                            printf("[EE] PSUBSB\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            s8 bytes_rs[16];
                            s8 bytes_rt[16];
                            s8 bytes_rd[16];
                            *(u64*)bytes_rs = r[rs];
                            *(u64*)(bytes_rs + 8) = rhi[rs];
                            *(u64*)bytes_rt = r[rt];
                            *(u64*)(bytes_rt + 8) = rhi[rt];

                            for(int i = 0; i < 16; i++)
                            {
                                bytes_rd[i] = bytes_rs[i] - bytes_rt[i];
                                if(bytes_rd[i] >= 0x80) bytes_rd[i] = 0x7f;
                                else if(bytes_rd[i] < 0x80) bytes_rd[i] = 0x80;
                            }

                            if(rd)
                            {
                                r[rd] = *(u64*)bytes_rd;
                                rhi[rd] = *(u64*)(bytes_rd + 8);
                            }
                            break;
                        }
                        case 0x1a:
                        {
                            printf("[EE] PEXTLB\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            u8 bytes_rs[8];
                            u8 bytes_rt[8];
                            u16 halfwords_rd[8];
                            *(u64*)bytes_rs = r[rs];
                            *(u64*)bytes_rt = r[rt];

                            for(int i = 0; i < 8; i++)
                            {
                                halfwords_rd[i] = bytes_rt[i] | ((u16)bytes_rs[i] << 8);
                            }

                            if(rd)
                            {
                                r[rd] = *(u64*)halfwords_rd;
                                rhi[rd] = *(u64*)(halfwords_rd + 4);
                            }
                            break;
                        }
                        case 0x1b:
                        {
                            printf("[EE] PPACB\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            u8 bytes_rs[16];
                            u8 bytes_rt[16];
                            u8 bytes_rd[16];
                            *(u64*)bytes_rs = r[rs];
                            *(u64*)(bytes_rs + 8) = rhi[rs];
                            *(u64*)bytes_rt = r[rt];
                            *(u64*)(bytes_rt + 8) = rhi[rt];

                            for(int i = 0; i < 8; i++)
                            {
                                bytes_rd[i] = bytes_rt[i << 1];
                                bytes_rd[i + 8] = bytes_rs[i << 1];
                            }

                            if(rd)
                            {
                                r[rd] = *(u64*)bytes_rd;
                                rhi[rd] = *(u64*)(bytes_rd + 8);
                            }
                            break;
                        }
                        case 0x1e:
                        {
                            printf("[EE] PEXT5\n");
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            u16 halfwords_rt[8];
                            u32 words_rd[4];
                            *(u64*)halfwords_rt = r[rt];
                            *(u64*)(halfwords_rt + 4) = rhi[rt];

                            for(int i = 0; i < 4; i++)
                            {
                                u32 extended = (u32)(halfwords_rt[i << 1] & 0x001f) << 3;
                                extended |= (u32)(halfwords_rt[i << 1] & 0x03e0) << 6;
                                extended |= (u32)(halfwords_rt[i << 1] & 0x7c00) << 9;
                                extended |= (u32)(halfwords_rt[i << 1] & 0x8000) << 16;
                                words_rd[i] = extended;
                            }

                            if(rd)
                            {
                                r[rd] = *(u64*)words_rd;
                                rhi[rd] = *(u64*)(words_rd + 2);
                            }
                            break;
                        }
                        case 0x1f:
                        {
                            printf("[EE] PPAC5\n");
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            u32 words_rt[4];
                            u16 halfwords_rd[8];
                            *(u64*)words_rt = r[rt];
                            *(u64*)(words_rt + 2) = rhi[rt];

                            for(int i = 0; i < 4; i++)
                            {
                                u16 packed = (words_rt[i] >> 3) & 0x1f;
                                packed |= (words_rt[i] >> 11) & 0x1f;
                                packed |= (words_rt[i] >> 19) & 0x1f;
                                packed |= (words_rt[i] >> 31) & 0x01;
                                halfwords_rd[i << 1] = packed;
                                halfwords_rd[(i << 1) + 1] = 0;
                            }

                            if(rd)
                            {
                                r[rd] = *(u64*)halfwords_rd;
                                rhi[rd] = *(u64*)(halfwords_rd + 4);
                            }
                            break;
                        }
                    }
                    break;
                }
                case 0x09:
                {
                    switch((opcode >> 6) & 0x1f)
                    {
                        case 0x00:
                        {
                            printf("[EE] PMADDW\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;

                            s64 result = (s64)(s32)r[rs] * (s64)(s32)r[rt];
                            u64 lo_hi0 = (u32)lo | ((u64)(u32)hi << 32);
                            result += lo_hi0;
                            s64 result_lo = (s32)result;
                            s64 result_hi = (s32)(result >> 32);
                            lo = result_lo;
                            hi = result_hi;
                            if(rd) r[rd] = result;

                            s64 result1 = (s64)(s32)rhi[rs] * (s64)(s32)rhi[rt];
                            u64 lo_hi1 = (u32)lo1 | ((u64)(u32)hi1 << 32);
                            result1 += lo_hi1;
                            s64 result_lo1 = (s32)result1;
                            s64 result_hi1 = (s32)(result1 >> 32);
                            lo1 = result_lo1;
                            hi1 = result_hi1;
                            if(rd) rhi[rd] = result1;
                            break;
                        }
                        case 0x02:
                        {
                            printf("[EE] PSLLVW\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            int shift_low = r[rs] & 0x1f;
                            int shift_high = rhi[rs] & 0x1f;

                            if(rd)
                            {
                                r[rd] = (s64)((s64)(s32)r[rt] << shift_low);
                                rhi[rd] = (s64)((s64)(s32)rhi[rt] << shift_high);
                            }
                            break;
                        }
                        case 0x03:
                        {
                            printf("[EE] PSRLVW\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            int shift_low = r[rs] & 0x1f;
                            int shift_high = rhi[rs] & 0x1f;

                            if(rd)
                            {
                                r[rd] = r[rt] >> shift_low;
                                rhi[rd] = rhi[rt] >> shift_high;
                            }
                            break;
                        }
                        case 0x04:
                        {
                            printf("[EE] PMSUBW\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;

                            s64 result = (s64)(s32)r[rs] * (s64)(s32)r[rt];
                            u64 lo_hi0 = (u32)lo | ((u64)(u32)hi << 32);
                            result = lo_hi0 - result;
                            s64 result_lo = (s32)result;
                            s64 result_hi = (s32)(result >> 32);
                            lo = result_lo;
                            hi = result_hi;
                            if(rd) r[rd] = result;

                            s64 result1 = (s64)(s32)rhi[rs] * (s64)(s32)rhi[rt];
                            u64 lo_hi1 = (u32)lo1 | ((u64)(u32)hi1 << 32);
                            result1 = lo_hi1 - result1;
                            s64 result_lo1 = (s32)result1;
                            s64 result_hi1 = (s32)(result1 >> 32);
                            lo1 = result_lo1;
                            hi1 = result_hi1;
                            if(rd) rhi[rd] = result1;
                            break;
                        }
                        case 0x08:
                        {
                            printf("[EE] PMFHI\n");
                            int rd = (opcode >> 11) & 0x1f;
                            if(rd)
                            {
                                r[rd] = hi;
                                rhi[rd] = hi1;
                            }
                            break;
                        }
                        case 0x09:
                        {
                            printf("[EE] PMFLO\n");
                            int rd = (opcode >> 11) & 0x1f;
                            if(rd)
                            {
                                r[rd] = lo;
                                rhi[rd] = lo1;
                            }
                            break;
                        }
                        case 0x0a:
                        {
                            printf("[EE] PINTH\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            u16 halfwords_rs[4];
                            u16 halfwords_rt[4];
                            u32 words_rd[4];
                            *(u64*)halfwords_rs = rhi[rs];
                            *(u64*)halfwords_rt = r[rt];

                            for(int i = 0; i < 4; i++)
                            {
                                words_rd[i] = halfwords_rt[i] | ((u32)halfwords_rs[i] << 16);
                            }

                            if(rd)
                            {
                                r[rd] = *(u64*)words_rd;
                                rhi[rd] = *(u64*)(words_rd + 2);
                            }
                            break;
                        }
                        case 0x0c:
                        {
                            printf("[EE] PMULTW\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;

                            s64 result = (s64)(s32)r[rs] * (s64)(s32)r[rt];
                            s64 result_lo = (s32)result;
                            s64 result_hi = (s32)(result >> 32);
                            lo = result_lo;
                            hi = result_hi;
                            if(rd) r[rd] = result;

                            s64 result1 = (s64)(s32)rhi[rs] * (s64)(s32)rhi[rt];
                            s64 result_lo1 = (s32)result1;
                            s64 result_hi1 = (s32)(result1 >> 32);
                            lo1 = result_lo1;
                            hi1 = result_hi1;
                            if(rd) rhi[rd] = result1;
                            break;
                        }
                        case 0x0d:
                        {
                            printf("[EE] PDIVW\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;

                            if((u32)r[rs] == 0x80000000 && (u32)r[rt] == 0xffffffff)
                            {
                                lo = 0x80000000;
                                hi = 0;
                            }
                            else if(r[rt])
                            {
                                lo = (s64)(s32)((s32)r[rs] / (s32)r[rt]);
                                hi = (s64)(s32)((s32)r[rs] % (s32)r[rt]);
                            }
                            else
                            {
                                if((s32)r[rs] >= 0) lo = ~0;
                                else lo = 1;
                                hi = (s64)(s32)r[rs];
                            }

                            if((u32)rhi[rs] == 0x80000000 && (u32)rhi[rt] == 0xffffffff)
                            {
                                lo1 = 0x80000000;
                                hi1 = 0;
                            }
                            else if(rhi[rt])
                            {
                                lo1 = (s64)(s32)((s32)rhi[rs] / (s32)rhi[rt]);
                                hi1 = (s64)(s32)((s32)rhi[rs] % (s32)rhi[rt]);
                            }
                            else
                            {
                                if((s32)rhi[rs] >= 0) lo1 = ~0;
                                else lo1 = 1;
                                hi1 = (s64)(s32)rhi[rs];
                            }
                            break;
                        }
                        case 0x0e:
                        {
                            printf("[EE] PCPYLD\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;

                            if(rd)
                            {
                                r[rd] = r[rt];
                                rhi[rd] = r[rs];
                            }
                            break;
                        }
                        case 0x10:
                        {
                            printf("[EE] PMADDH\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            s16 halfwords_rs[8];
                            s16 halfwords_rt[8];
                            *(u64*)halfwords_rs = r[rs];
                            *(u64*)(halfwords_rs + 4) = rhi[rs];
                            *(u64*)halfwords_rt = r[rt];
                            *(u64*)(halfwords_rt + 4) = rhi[rt];

                            u32 result = (u32)lo + (halfwords_rs[0] * halfwords_rt[0]);
                            lo = result;
                            if(rd) r[rd] = result;

                            result = (lo >> 32) + (halfwords_rs[1] * halfwords_rt[1]);
                            lo |= (u64)result << 32;

                            result = (u32)hi + (halfwords_rs[2] * halfwords_rt[2]);
                            hi = result;
                            if(rd) r[rd] |= (u64)result << 32;

                            result = (hi >> 32) + (halfwords_rs[3] * halfwords_rt[3]);
                            hi |= (u64)result << 32;

                            result = (u32)lo1 + (halfwords_rs[4] * halfwords_rt[4]);
                            lo1 = result;
                            if(rd) rhi[rd] = result;

                            result = (lo1 >> 32) + (halfwords_rs[5] * halfwords_rt[5]);
                            lo1 |= (u64)result << 32;

                            result = (u32)hi1 + (halfwords_rs[6] * halfwords_rt[6]);
                            hi1 = result;
                            if(rd) rhi[rd] |= (u64)result << 32;

                            result = (hi1 >> 32) + (halfwords_rs[7] * halfwords_rt[7]);
                            hi1 |= (u64)result << 32;
                            break;
                        }
                        case 0x11:
                        {
                            printf("[EE] PHMADH\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            s16 halfwords_rs[8];
                            s16 halfwords_rt[8];
                            *(u64*)halfwords_rs = r[rs];
                            *(u64*)(halfwords_rs + 4) = rhi[rs];
                            *(u64*)halfwords_rt = r[rt];
                            *(u64*)(halfwords_rt + 4) = rhi[rt];

                            u32 result = (halfwords_rs[1] * halfwords_rt[1]) + (halfwords_rs[0] * halfwords_rt[0]);
                            lo = result;
                            if(rd) r[rd] = result;

                            result = (halfwords_rs[3] * halfwords_rt[3]) + (halfwords_rs[2] * halfwords_rt[2]);
                            hi = result;
                            if(rd) r[rd] |= (u64)result << 32;

                            result = (halfwords_rs[5] * halfwords_rt[5]) + (halfwords_rs[4] * halfwords_rt[4]);
                            lo1 = result;
                            if(rd) rhi[rd] = result;

                            result = (halfwords_rs[7] * halfwords_rt[7]) + (halfwords_rs[6] * halfwords_rt[6]);
                            hi1 = result;
                            if(rd) rhi[rd] |= (u64)result << 32;
                            break;
                        }
                        case 0x12:
                        {
                            printf("[EE] PAND\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;

                            if(rd)
                            {
                                r[rd] = r[rs] & r[rt];
                                rhi[rd] = rhi[rs] & rhi[rt];
                            }
                            break;
                        }
                        case 0x13:
                        {
                            printf("[EE] PXOR\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;

                            if(rd)
                            {
                                r[rd] = r[rs] ^ r[rt];
                                rhi[rd] = rhi[rs] ^ rhi[rt];
                            }
                            break;
                        }
                        case 0x14:
                        {
                            printf("[EE] PMSUBH\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            s16 halfwords_rs[8];
                            s16 halfwords_rt[8];
                            *(u64*)halfwords_rs = r[rs];
                            *(u64*)(halfwords_rs + 4) = rhi[rs];
                            *(u64*)halfwords_rt = r[rt];
                            *(u64*)(halfwords_rt + 4) = rhi[rt];

                            u32 result = (u32)lo - (halfwords_rs[0] * halfwords_rt[0]);
                            lo = result;
                            if(rd) r[rd] = result;

                            result = (lo >> 32) - (halfwords_rs[1] * halfwords_rt[1]);
                            lo |= (u64)result << 32;

                            result = (u32)hi - (halfwords_rs[2] * halfwords_rt[2]);
                            hi = result;
                            if(rd) r[rd] |= (u64)result << 32;

                            result = (hi >> 32) - (halfwords_rs[3] * halfwords_rt[3]);
                            hi |= (u64)result << 32;

                            result = (u32)lo1 - (halfwords_rs[4] * halfwords_rt[4]);
                            lo1 = result;
                            if(rd) rhi[rd] = result;

                            result = (lo1 >> 32) - (halfwords_rs[5] * halfwords_rt[5]);
                            lo1 |= (u64)result << 32;

                            result = (u32)hi1 - (halfwords_rs[6] * halfwords_rt[6]);
                            hi1 = result;
                            if(rd) rhi[rd] |= (u64)result << 32;

                            result = (hi1 >> 32) - (halfwords_rs[7] * halfwords_rt[7]);
                            hi1 |= (u64)result << 32;
                            break;
                        }
                        case 0x15:
                        {
                            printf("[EE] PHMSBH\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            s16 halfwords_rs[8];
                            s16 halfwords_rt[8];
                            *(u64*)halfwords_rs = r[rs];
                            *(u64*)(halfwords_rs + 4) = rhi[rs];
                            *(u64*)halfwords_rt = r[rt];
                            *(u64*)(halfwords_rt + 4) = rhi[rt];

                            u32 result = (halfwords_rs[1] * halfwords_rt[1]) - (halfwords_rs[0] * halfwords_rt[0]);
                            lo = result;
                            if(rd) r[rd] = result;

                            result = (halfwords_rs[3] * halfwords_rt[3]) - (halfwords_rs[2] * halfwords_rt[2]);
                            hi = result;
                            if(rd) r[rd] |= (u64)result << 32;

                            result = (halfwords_rs[5] * halfwords_rt[5]) - (halfwords_rs[4] * halfwords_rt[4]);
                            lo1 = result;
                            if(rd) rhi[rd] = result;

                            result = (halfwords_rs[7] * halfwords_rt[7]) - (halfwords_rs[6] * halfwords_rt[6]);
                            hi1 = result;
                            if(rd) rhi[rd] |= (u64)result << 32;
                            break;
                        }
                        case 0x1a:
                        {
                            printf("[EE] PEXEH\n");
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            u16 halfwords_rt[8];
                            u16 halfwords_rd[8];
                            *(u64*)halfwords_rt = r[rt];
                            *(u64*)(halfwords_rt + 4) = rhi[rt];

                            halfwords_rd[1] = halfwords_rt[1];
                            halfwords_rd[3] = halfwords_rt[3];
                            halfwords_rd[5] = halfwords_rt[5];
                            halfwords_rd[7] = halfwords_rt[7];

                            halfwords_rd[0] = halfwords_rt[2];
                            halfwords_rd[2] = halfwords_rt[0];
                            halfwords_rd[4] = halfwords_rt[6];
                            halfwords_rd[6] = halfwords_rt[4];

                            if(rd)
                            {
                                r[rd] = *(u64*)halfwords_rd;
                                rhi[rd] = *(u64*)(halfwords_rd + 4);
                            }
                            break;
                        }
                        case 0x1b:
                        {
                            printf("[EE] PREVH\n");
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            u16 halfwords_rt[8];
                            u16 halfwords_rd[8];
                            *(u64*)halfwords_rt = r[rt];
                            *(u64*)(halfwords_rt + 4) = rhi[rt];

                            halfwords_rd[0] = halfwords_rt[3];
                            halfwords_rd[1] = halfwords_rt[2];
                            halfwords_rd[2] = halfwords_rt[1];
                            halfwords_rd[3] = halfwords_rt[0];

                            halfwords_rd[4] = halfwords_rt[7];
                            halfwords_rd[5] = halfwords_rt[6];
                            halfwords_rd[6] = halfwords_rt[5];
                            halfwords_rd[7] = halfwords_rt[4];

                            if(rd)
                            {
                                r[rd] = *(u64*)halfwords_rd;
                                rhi[rd] = *(u64*)(halfwords_rd + 4);
                            }
                            break;
                        }
                        case 0x1c:
                        {
                            printf("[EE] PMULTH\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            s16 halfwords_rs[8];
                            s16 halfwords_rt[8];
                            *(u64*)halfwords_rs = r[rs];
                            *(u64*)(halfwords_rs + 4) = rhi[rs];
                            *(u64*)halfwords_rt = r[rt];
                            *(u64*)(halfwords_rt + 4) = rhi[rt];

                            u32 result = (s32)halfwords_rs[0] * (s32)halfwords_rt[0];
                            lo = result;
                            if(rd) r[rd] = result;

                            result = (s32)halfwords_rs[1] * (s32)halfwords_rt[1];
                            lo |= (u64)result << 32;

                            result = (s32)halfwords_rs[2] * (s32)halfwords_rt[2];
                            hi = result;
                            if(rd) r[rd] |= (u64)result << 32;

                            result = (s32)halfwords_rs[3] * (s32)halfwords_rt[3];
                            hi |= (u64)result << 32;

                            result = (s32)halfwords_rs[4] * (s32)halfwords_rt[4];
                            lo1 = result;
                            if(rd) rhi[rd] = result;

                            result = (s32)halfwords_rs[5] * (s32)halfwords_rt[5];
                            lo1 |= (u64)result << 32;

                            result = (s32)halfwords_rs[6] * (s32)halfwords_rt[6];
                            hi1 = result;
                            if(rd) rhi[rd] |= (u64)result << 32;

                            result = (s32)halfwords_rs[7] * (s32)halfwords_rt[7];
                            hi1 |= (u64)result << 32;
                            break;
                        }
                        case 0x1d:
                        {
                            printf("[EE] PDIVBW\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            s32 words_rs[4];
                            *(u64*)words_rs = r[rs];
                            *(u64*)(words_rs + 2) = rhi[rs];

                            if(words_rs[0] == 0x80000000 && (u16)r[rt] == 0xffff)
                            {
                                lo = 0x80000000;
                                hi = 0;
                            }
                            else if(r[rt])
                            {
                                lo = words_rs[0] / (s16)r[rt];
                                hi = (s32)(s16)(words_rs[0] % (u16)r[rt]);
                            }
                            else
                            {
                                if(words_rs[0] >= 0) lo = ~0;
                                else lo = 1;
                                hi = (s32)words_rs[0];
                            }

                            if(words_rs[1] == 0x80000000 && (u16)r[rt] == 0xffff)
                            {
                                lo |= 0x80000000ULL << 32;
                            }
                            else if(r[rt])
                            {
                                lo = (u64)(words_rs[1] / (s16)r[rt]) << 32;
                                hi = (u64)(s32)(s16)(words_rs[1] % (u16)r[rt]) << 32;
                            }
                            else
                            {
                                if(words_rs[1] >= 0) lo = (~0ULL) << 32;
                                else lo = 1ULL << 32;
                                hi = (u64)(s32)words_rs[1] << 32;
                            }

                            if(words_rs[2] == 0x80000000 && (u16)r[rt] == 0xffff)
                            {
                                lo1 = 0x80000000;
                                hi1 = 0;
                            }
                            else if(r[rt])
                            {
                                lo1 = words_rs[2] / (s16)r[rt];
                                hi1 = (s32)(s16)(words_rs[2] % (u16)r[rt]);
                            }
                            else
                            {
                                if(words_rs[2] >= 0) lo1 = ~0;
                                else lo1 = 1;
                                hi1 = (s32)words_rs[2];
                            }

                            if(words_rs[3] == 0x80000000 && (u16)r[rt] == 0xffff)
                            {
                                lo1 |= 0x80000000ULL << 32;
                            }
                            else if(r[rt])
                            {
                                lo1 = (u64)(words_rs[3] / (s16)r[rt]) << 32;
                                hi1 = (u64)(s32)(s16)(words_rs[3] % (u16)r[rt]) << 32;
                            }
                            else
                            {
                                if(words_rs[3] >= 0) lo1 = (~0ULL) << 32;
                                else lo1 = 1ULL << 32;
                                hi1 = (u64)(s32)words_rs[3] << 32;
                            }
                            break;
                        }
                        case 0x1e:
                        {
                            printf("[EE] PEXEW\n");
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            u32 words_rt[4];
                            u32 words_rd[4];
                            *(u64*)words_rt = r[rt];
                            *(u64*)(words_rt + 2) = rhi[rt];

                            words_rd[1] = words_rt[1];
                            words_rd[3] = words_rt[3];

                            words_rd[0] = words_rt[2];
                            words_rd[2] = words_rt[0];

                            if(rd)
                            {
                                r[rd] = *(u64*)words_rd;
                                rhi[rd] = *(u64*)(words_rd + 2);
                            }
                            break;
                        }
                        case 0x1f:
                        {
                            printf("[EE] PROT3W\n");
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            u32 words_rt[4];
                            u32 words_rd[4];
                            *(u64*)words_rt = r[rt];
                            *(u64*)(words_rt + 2) = rhi[rt];

                            words_rd[3] = words_rt[3];

                            words_rd[0] = words_rt[1];
                            words_rd[1] = words_rt[2];
                            words_rd[2] = words_rt[0];

                            if(rd)
                            {
                                r[rd] = *(u64*)words_rd;
                                rhi[rd] = *(u64*)(words_rd + 2);
                            }
                            break;
                        }
                    }
                    break;
                }
                case 0x10:
                {
                    printf("[EE] MFHI1\n");
                    int rd = (opcode >> 11) & 0x1f;
                    if(rd) r[rd] = hi1;
                    break;
                }
                case 0x11:
                {
                    printf("[EE] MTHI1\n");
                    int rs = (opcode >> 21) & 0x1f;
                    hi1 = r[rs];
                    break;
                }
                case 0x12:
                {
                    printf("[EE] MFLO1\n");
                    int rd = (opcode >> 11) & 0x1f;
                    if(rd) r[rd] = lo1;
                    break;
                }
                case 0x13:
                {
                    printf("[EE] MTLO1\n");
                    int rs = (opcode >> 21) & 0x1f;
                    lo1 = r[rs];
                    break;
                }
                case 0x18:
                {
                    printf("[EE] MULT1\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    s64 result = (s64)(s32)r[rs] * (s64)(s32)r[rt];
                    s64 result_lo = (s32)result;
                    s64 result_hi = (s32)(result >> 32);
                    lo1 = result_lo;
                    hi1 = result_hi;
                    if(rd) r[rd] = lo1;
                    break;
                }
                case 0x19:
                {
                    printf("[EE] MULTU1\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    u64 result = (u64)(u32)r[rs] * (u64)(u32)r[rt];
                    u32 result_lo = (u32)result;
                    u32 result_hi = result >> 32;
                    lo1 = result_lo;
                    hi1 = result_hi;
                    if(rd) r[rd] = lo1;
                    break;
                }
                case 0x1a:
                {
                    printf("[EE] DIV1\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    if(!(s32)r[rt])
                    {
                        hi1 = r[rs];
                        if((s32)r[rs] > 0x80000000) lo1 = 1;
                        else lo1 = 0xffffffff;
                    }
                    else if((u32)r[rs] == 0x80000000 && (u32)r[rt] == 0xffffffff)
                    {
                        lo1 = 0x80000000;
                        hi1 = 0;
                    }
                    else
                    {
                        lo1 = (s32)r[rs] / (s32)r[rt];
                        hi1 = (s32)r[rs] % (s32)r[rt];
                    }
                    break;
                }
                case 0x1b:
                {
                    printf("[EE] DIVU1\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    if(!r[rt])
                    {
                        lo1 = 0xffffffff;
                        hi1 = r[rs];
                    }
                    else
                    {
                        lo1 = (u32)r[rs] / (u32)r[rt];
                        hi1 = (u32)r[rs] % (u32)r[rt];
                    }
                    break;
                }
                case 0x20:
                {
                    printf("[EE] MADD1\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    s64 result = (s64)(s32)r[rs] * (s64)(s32)r[rt];
                    u64 lo_hi = (u32)lo1 | ((u64)(u32)hi1 << 32);
                    result += lo_hi;
                    s64 result_lo = (s32)result;
                    s64 result_hi = (s32)(result >> 32);
                    lo1 = result_lo;
                    hi1 = result_hi;
                    if(rd) r[rd] = lo1;
                    break;
                }
                case 0x21:
                {
                    printf("[EE] MADDU1\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    s64 result = (u64)(u32)r[rs] * (u64)(u32)r[rt];
                    u64 lo_hi = (u32)lo1 | ((u64)(u32)hi << 32);
                    result = (u64)result + lo_hi;
                    s64 result_lo = (s32)result;
                    s64 result_hi = (s32)(result >> 32);
                    lo1 = result_lo;
                    hi1 = result_hi;
                    if(rd) r[rd] = lo1;
                    break;
                }
                case 0x28:
                {
                    switch((opcode >> 6) & 0x1f)
                    {
                        case 0x01:
                        {
                            printf("[EE] PABSW\n");
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            s32 words_rt[4];
                            u32 words_rd[4];
                            *(u64*)words_rt = r[rt];
                            *(u64*)(words_rt + 2) = rhi[rt];

                            for(int i = 0; i < 4; i++)
                            {
                                words_rd[i] = abs(words_rt[i]);
                            }

                            if(rd)
                            {
                                r[rd] = *(u64*)words_rd;
                                rhi[rd] = *(u64*)(words_rd + 2);
                            }
                            break;
                        }
                        case 0x02:
                        {
                            printf("[EE] PCEQW\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            s32 words_rs[4];
                            s32 words_rt[4];
                            s32 words_rd[4];
                            *(u64*)words_rs = r[rs];
                            *(u64*)(words_rs + 2) = rhi[rs];
                            *(u64*)words_rt = r[rt];
                            *(u64*)(words_rt + 2) = rhi[rt];

                            for(int i = 0; i < 4; i++)
                            {
                                words_rd[i] = (words_rs[i] == words_rt[i]) ? ~0 : 0;
                            }

                            if(rd)
                            {
                                r[rd] = *(u64*)words_rd;
                                rhi[rd] = *(u64*)(words_rd + 2);
                            }
                            break;
                        }
                        case 0x03:
                        {
                            printf("[EE] PMINW\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            s32 words_rs[4];
                            s32 words_rt[4];
                            s32 words_rd[4];
                            *(u64*)words_rs = r[rs];
                            *(u64*)(words_rs + 2) = rhi[rs];
                            *(u64*)words_rt = r[rt];
                            *(u64*)(words_rt + 2) = rhi[rt];

                            for(int i = 0; i < 4; i++)
                            {
                                words_rd[i] = (words_rs[i] <= words_rt[i]) ? r[rs] : r[rt];
                            }

                            if(rd)
                            {
                                r[rd] = *(u64*)words_rd;
                                rhi[rd] = *(u64*)(words_rd + 2);
                            }
                            break;
                        }
                        case 0x04:
                        {
                            printf("[EE] PADSBH\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            u16 halfwords_rs[8];
                            u16 halfwords_rt[8];
                            u16 halfwords_rd[8];
                            *(u64*)halfwords_rs = r[rs];
                            *(u64*)(halfwords_rs + 4) = rhi[rs];
                            *(u64*)halfwords_rt = r[rt];
                            *(u64*)(halfwords_rt + 4) = rhi[rt];

                            for(int i = 0; i < 8; i++)
                            {
                                if(i & 4) halfwords_rd[i] = halfwords_rs[i] + halfwords_rt[i];
                                else halfwords_rd[i] = halfwords_rs[i] - halfwords_rt[i];
                            }

                            if(rd)
                            {
                                r[rd] = *(u64*)halfwords_rd;
                                rhi[rd] = *(u64*)(halfwords_rd + 4);
                            }
                            break;
                        }
                        case 0x05:
                        {
                            printf("[EE] PABSH\n");
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            s16 halfwords_rt[8];
                            u16 halfwords_rd[8];
                            *(u64*)halfwords_rt = r[rt];
                            *(u64*)(halfwords_rt + 4) = rhi[rt];

                            for(int i = 0; i < 4; i++)
                            {
                                halfwords_rd[i] = abs((s32)halfwords_rt[i]);
                            }

                            if(rd)
                            {
                                r[rd] = *(u64*)halfwords_rd;
                                rhi[rd] = *(u64*)(halfwords_rd + 4);
                            }
                            break;
                        }
                        case 0x06:
                        {
                            printf("[EE] PCEQH\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            s16 halfwords_rs[8];
                            s16 halfwords_rt[8];
                            s16 halfwords_rd[8];
                            *(u64*)halfwords_rs = r[rs];
                            *(u64*)(halfwords_rs + 4) = rhi[rs];
                            *(u64*)halfwords_rt = r[rt];
                            *(u64*)(halfwords_rt + 4) = rhi[rt];

                            for(int i = 0; i < 8; i++)
                            {
                                halfwords_rd[i] = (halfwords_rs[i] == halfwords_rt[i]) ? ~0 : 0;
                            }

                            if(rd)
                            {
                                r[rd] = *(u64*)halfwords_rd;
                                rhi[rd] = *(u64*)(halfwords_rd + 4);
                            }
                            break;
                        }
                        case 0x07:
                        {
                            printf("[EE] PMINH\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            s16 halfwords_rs[8];
                            s16 halfwords_rt[8];
                            s16 halfwords_rd[8];
                            *(u64*)halfwords_rs = r[rs];
                            *(u64*)(halfwords_rs + 4) = rhi[rs];
                            *(u64*)halfwords_rt = r[rt];
                            *(u64*)(halfwords_rt + 4) = rhi[rt];

                            for(int i = 0; i < 8; i++)
                            {
                                halfwords_rd[i] = (halfwords_rs[i] <= halfwords_rt[i]) ? r[rs] : r[rt];
                            }

                            if(rd)
                            {
                                r[rd] = *(u64*)halfwords_rd;
                                rhi[rd] = *(u64*)(halfwords_rd + 4);
                            }
                            break;
                        }
                        case 0x0a:
                        {
                            printf("[EE] PCEQB\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            s8 bytes_rs[16];
                            s8 bytes_rt[16];
                            s8 bytes_rd[16];
                            *(u64*)bytes_rs = r[rs];
                            *(u64*)(bytes_rs + 8) = rhi[rs];
                            *(u64*)bytes_rt = r[rt];
                            *(u64*)(bytes_rt + 8) = rhi[rt];

                            for(int i = 0; i < 16; i++)
                            {
                                bytes_rd[i] = (bytes_rs[i] == bytes_rt[i]) ? ~0 : 0;
                            }

                            if(rd)
                            {
                                r[rd] = *(u64*)bytes_rd;
                                rhi[rd] = *(u64*)(bytes_rd + 8);
                            }
                            break;
                        }
                        case 0x10:
                        {
                            printf("[EE] PADDUW\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            u32 words_rs[4];
                            u32 words_rt[4];
                            u32 words_rd[4];
                            *(u64*)words_rs = r[rs];
                            *(u64*)(words_rs + 2) = rhi[rs];
                            *(u64*)words_rt = r[rt];
                            *(u64*)(words_rt + 2) = rhi[rt];

                            for(int i = 0; i < 4; i++)
                            {
                                u64 result = words_rs[i] + words_rt[i];
                                if(result >= 0x100000000) words_rd[i] = 0xffffffff;
                                else words_rd[i] = (u32)result;
                            }

                            if(rd)
                            {
                                r[rd] = *(u64*)words_rd;
                                rhi[rd] = *(u64*)(words_rd + 2);
                            }
                            break;
                        }
                        case 0x11:
                        {
                            printf("[EE] PSUBUW\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            u32 words_rs[4];
                            u32 words_rt[4];
                            u32 words_rd[4];
                            *(u64*)words_rs = r[rs];
                            *(u64*)(words_rs + 2) = rhi[rs];
                            *(u64*)words_rt = r[rt];
                            *(u64*)(words_rt + 2) = rhi[rt];

                            for(int i = 0; i < 4; i++)
                            {
                                s32 result = words_rs[i] - words_rt[i];
                                if(result < 0) words_rd[i] = 0;
                                else words_rd[i] = (u32)result;
                            }

                            if(rd)
                            {
                                r[rd] = *(u64*)words_rd;
                                rhi[rd] = *(u64*)(words_rd + 2);
                            }
                            break;
                        }
                        case 0x12:
                        {
                            printf("[EE] PEXTUW\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;

                            if(rd)
                            {
                                r[rd] = (u32)rhi[rt] | ((u64)(u32)rhi[rs] << 32);
                                rhi[rd] = (u32)(rhi[rt] >> 32) | ((rhi[rs] >> 32) << 32);
                            }
                            break;
                        }
                        case 0x14:
                        {
                            printf("[EE] PADDUH\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            u16 halfwords_rs[8];
                            u16 halfwords_rt[8];
                            u16 halfwords_rd[8];
                            *(u64*)halfwords_rs = r[rs];
                            *(u64*)(halfwords_rs + 4) = rhi[rs];
                            *(u64*)halfwords_rt = r[rt];
                            *(u64*)(halfwords_rt + 4) = rhi[rt];

                            for(int i = 0; i < 8; i++)
                            {
                                u32 result = halfwords_rs[i] + halfwords_rt[i];
                                if(result >= 0x10000) halfwords_rd[i] = 0xffff;
                                else halfwords_rd[i] = (u16)result;
                            }

                            if(rd)
                            {
                                r[rd] = *(u64*)halfwords_rd;
                                rhi[rd] = *(u64*)(halfwords_rd + 4);
                            }
                            break;
                        }
                        case 0x15:
                        {
                            printf("[EE] PSUBUH\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            u16 halfwords_rs[8];
                            u16 halfwords_rt[8];
                            u16 halfwords_rd[8];
                            *(u64*)halfwords_rs = r[rs];
                            *(u64*)(halfwords_rs + 4) = rhi[rs];
                            *(u64*)halfwords_rt = r[rt];
                            *(u64*)(halfwords_rt + 4) = rhi[rt];

                            for(int i = 0; i < 8; i++)
                            {
                                s16 result = halfwords_rs[i] - halfwords_rt[i];
                                if(result < 0) halfwords_rd[i] = 0;
                                else halfwords_rd[i] = (u16)result;
                            }

                            if(rd)
                            {
                                r[rd] = *(u64*)halfwords_rd;
                                rhi[rd] = *(u64*)(halfwords_rd + 4);
                            }
                            break;
                        }
                        case 0x16:
                        {
                            printf("[EE] PEXTUH\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            u16 halfwords_rs[4];
                            u16 halfwords_rt[4];
                            u32 words_rd[4];
                            *(u64*)halfwords_rs = rhi[rs];
                            *(u64*)halfwords_rt = rhi[rt];

                            for(int i = 0; i < 4; i++)
                            {
                                words_rd[i] = halfwords_rt[i] | ((u32)halfwords_rs[i] << 16);
                            }

                            if(rd)
                            {
                                r[rd] = *(u64*)words_rd;
                                rhi[rd] = *(u64*)(words_rd + 2);
                            }
                            break;
                        }
                        case 0x18:
                        {
                            printf("[EE] PADDUB\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            u8 bytes_rs[16];
                            u8 bytes_rt[16];
                            u8 bytes_rd[16];
                            *(u64*)bytes_rs = r[rs];
                            *(u64*)(bytes_rs + 8) = rhi[rs];
                            *(u64*)bytes_rt = r[rt];
                            *(u64*)(bytes_rt + 8) = rhi[rt];

                            for(int i = 0; i < 16; i++)
                            {
                                u16 result = bytes_rs[i] + bytes_rt[i];
                                if(result >= 0x100) bytes_rd[i] = 0xff;
                                else bytes_rd[i] = (u8)result;
                            }

                            if(rd)
                            {
                                r[rd] = *(u64*)bytes_rd;
                                rhi[rd] = *(u64*)(bytes_rd + 8);
                            }
                            break;
                        }
                        case 0x19:
                        {
                            printf("[EE] PSUBUB\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            u8 bytes_rs[16];
                            u8 bytes_rt[16];
                            u8 bytes_rd[16];
                            *(u64*)bytes_rs = r[rs];
                            *(u64*)(bytes_rs + 8) = rhi[rs];
                            *(u64*)bytes_rt = r[rt];
                            *(u64*)(bytes_rt + 8) = rhi[rt];

                            for(int i = 0; i < 16; i++)
                            {
                                s8 result = bytes_rs[i] - bytes_rt[i];
                                if(result >= 0x100) bytes_rd[i] = 0xff;
                                else bytes_rd[i] = (u8)result;
                            }

                            if(rd)
                            {
                                r[rd] = *(u64*)bytes_rd;
                                rhi[rd] = *(u64*)(bytes_rd + 8);
                            }
                            break;
                        }
                        case 0x1a:
                        {
                            printf("[EE] PEXTUB\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            u8 bytes_rs[8];
                            u8 bytes_rt[8];
                            u16 halfwords_rd[8];
                            *(u64*)bytes_rs = rhi[rs];
                            *(u64*)bytes_rt = rhi[rt];

                            for(int i = 0; i < 8; i++)
                            {
                                halfwords_rd[i] = bytes_rt[i] | ((u16)bytes_rs[i] << 8);
                            }

                            if(rd)
                            {
                                r[rd] = *(u64*)halfwords_rd;
                                rhi[rd] = *(u64*)(halfwords_rd + 4);
                            }
                            break;
                        }
                        case 0x1b:
                        {
                            printf("[EE] QFSRV\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;

                            //Implementation adapted from PCSX2.
                            if(rd)
                            {
                                if(ee_sa == 0)
                                {
                                    r[rd] = r[rt];
                                    rhi[rd] = rhi[rt];
                                }
                                else if(ee_sa < 64)
                                {
                                    r[rd] = r[rt] >> ee_sa;
                                    rhi[rd] = rhi[rt] >> ee_sa;

                                    r[rd] |= rhi[rt] << (64 - ee_sa);
                                    rhi[rd] |= r[rs] << (64 - ee_sa);
                                }
                                else
                                {
                                    r[rd] = rhi[rt] >> (ee_sa - 64);
                                    rhi[rd] = r[rs] >> (ee_sa - 64);
                                    if(ee_sa != 64)
                                    {
                                        r[rd] |= r[rs] << (128u - ee_sa);
                                        rhi[rd] |= rhi[rs] << (128u - ee_sa);
                                    }
                                }
                            }
                            break;
                        }
                    }
                    break;
                }
                case 0x29:
                {
                    switch((opcode >> 6) & 0x1f)
                    {
                        case 0x00:
                        {
                            printf("[EE] PMADDUW\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;

                            u64 result = (u64)(u32)r[rs] * (u64)(u32)r[rt];
                            u64 lo_hi0 = (u32)lo | ((u64)(u32)hi << 32);
                            result += lo_hi0;
                            s64 result_lo = (s32)result;
                            s64 result_hi = (s32)(result >> 32);
                            lo = result_lo;
                            hi = result_hi;
                            if(rd) r[rd] = result;

                            u64 result1 = (u64)(u32)rhi[rs] * (u64)(u32)rhi[rt];
                            u64 lo_hi1 = (u32)lo1 | ((u64)(u32)hi1 << 32);
                            result1 += lo_hi1;
                            s64 result_lo1 = (s32)result1;
                            s64 result_hi1 = (s32)(result1 >> 32);
                            lo1 = result_lo1;
                            hi1 = result_hi1;
                            if(rd) rhi[rd] = result1;
                            break;
                        }
                        case 0x03:
                        {
                            printf("[EE] PSRAVW\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            int shift_low = r[rs] & 0x1f;
                            int shift_high = rhi[rs] & 0x1f;

                            if(rd)
                            {
                                r[rd] = (s64)((s32)r[rt] >> shift_low);
                                rhi[rd] = (s64)((s32)rhi[rt] >> shift_high);
                            }
                            break;
                        }
                        case 0x08:
                        {
                            printf("[EE] PMTHI\n");
                            int rs = (opcode >> 21) & 0x1f;
                            hi = r[rs];
                            hi1 = rhi[rs];
                            break;
                        }
                        case 0x09:
                        {
                            printf("[EE] PMTLO\n");
                            int rs = (opcode >> 21) & 0x1f;
                            lo = r[rs];
                            lo1 = rhi[rs];
                            break;
                        }
                        case 0x0a:
                        {
                            printf("[EE] PINTEH\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            u16 halfwords_rs[8];
                            u16 halfwords_rt[8];
                            u32 words_rd[4];
                            *(u64*)halfwords_rs = r[rs];
                            *(u64*)(halfwords_rs + 4) = rhi[rs];
                            *(u64*)halfwords_rt = r[rt];
                            *(u64*)(halfwords_rt + 4) = rhi[rt];

                            for(int i = 0; i < 4; i++)
                            {
                                words_rd[i] = halfwords_rt[i << 1] | ((u32)halfwords_rs[i << 1] << 16);
                            }

                            if(rd)
                            {
                                r[rd] = *(u64*)words_rd;
                                rhi[rd] = *(u64*)(words_rd + 2);
                            }
                            break;
                        }
                        case 0x0c:
                        {
                            printf("[EE] PMULTUW\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;

                            s64 result = (u64)(u32)r[rs] * (u64)(u32)r[rt];
                            s64 result_lo = (s32)result;
                            s64 result_hi = (s32)(result >> 32);
                            lo = result_lo;
                            hi = result_hi;
                            if(rd) r[rd] = result;

                            s64 result1 = (u64)(u32)rhi[rs] * (u64)(u32)rhi[rt];
                            s64 result_lo1 = (s32)result1;
                            s64 result_hi1 = (s32)(result1 >> 32);
                            lo1 = result_lo1;
                            hi1 = result_hi1;
                            if(rd) rhi[rd] = result1;
                            break;
                        }
                        case 0x0d:
                        {
                            printf("[EE] PDIVUW\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;

                            if(!(u32)r[rt])
                            {
                                lo = 0xffffffff;
                                hi = (u32)r[rs];
                            }
                            else
                            {
                                lo = (s64)(s32)((u32)r[rs] / (u32)r[rt]);
                                hi = (s64)(s32)((u32)r[rs] % (u32)r[rt]);
                            }

                            if(!(u32)rhi[rt])
                            {
                                lo1 = 0xffffffff;
                                hi1 = (u32)rhi[rs];
                            }
                            else
                            {
                                lo1 = (s64)(s32)((u32)rhi[rs] / (u32)rhi[rt]);
                                hi1 = (s64)(s32)((u32)rhi[rs] % (u32)rhi[rt]);
                            }
                            break;
                        }
                        case 0x0e:
                        {
                            printf("[EE] PCPYUD\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;

                            if(rd)
                            {
                                r[rd] = rhi[rt];
                                rhi[rd] = rhi[rs];
                            }
                            break;
                        }
                        case 0x12:
                        {
                            printf("[EE] POR\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;

                            if(rd)
                            {
                                r[rd] = r[rs] | r[rt];
                                rhi[rd] = rhi[rs] | rhi[rt];
                            }
                            break;
                        }
                        case 0x13:
                        {
                            printf("[EE] PNOR\n");
                            int rs = (opcode >> 21) & 0x1f;
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;

                            if(rd)
                            {
                                r[rd] = ~(r[rs] | r[rt]);
                                rhi[rd] = ~(rhi[rs] | rhi[rt]);
                            }
                            break;
                        }
                        case 0x1a:
                        {
                            printf("[EE] PEXCH\n");
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            u16 halfwords_rt[8];
                            u16 halfwords_rd[8];
                            *(u64*)halfwords_rt = r[rt];
                            *(u64*)(halfwords_rt + 4) = rhi[rt];

                            halfwords_rd[0] = halfwords_rt[0];
                            halfwords_rd[3] = halfwords_rt[3];
                            halfwords_rd[4] = halfwords_rt[4];
                            halfwords_rd[7] = halfwords_rt[7];

                            halfwords_rd[1] = halfwords_rt[2];
                            halfwords_rd[2] = halfwords_rt[1];
                            halfwords_rd[5] = halfwords_rt[6];
                            halfwords_rd[6] = halfwords_rt[5];

                            if(rd)
                            {
                                r[rd] = *(u64*)halfwords_rd;
                                rhi[rd] = *(u64*)(halfwords_rd + 4);
                            }
                            break;
                        }
                        case 0x1b:
                        {
                            printf("[EE] PCPYH\n");
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;

                            if(rd)
                            {
                                u64 rt_16 = (u16)r[rt];
                                u64 rthi_16 = (u16)rhi[rt];
                                r[rd] = rt_16 | (rt_16 << 16) | (rt_16 << 32) | (rt_16 << 48);
                                rhi[rd] = rthi_16 | (rthi_16 << 16) | (rthi_16 << 32) | (rthi_16 << 48);
                            }
                            break;
                        }
                        case 0x1e:
                        {
                            printf("[EE] PEXCW\n");
                            int rt = (opcode >> 16) & 0x1f;
                            int rd = (opcode >> 11) & 0x1f;
                            u32 words_rt[4];
                            u32 words_rd[4];
                            *(u64*)words_rt = r[rt];
                            *(u64*)(words_rt + 2) = rhi[rt];

                            words_rd[0] = words_rt[0];
                            words_rd[3] = words_rt[3];

                            words_rd[1] = words_rt[2];
                            words_rd[2] = words_rt[1];

                            if(rd)
                            {
                                r[rd] = *(u64*)words_rd;
                                rhi[rd] = *(u64*)(words_rd + 2);
                            }
                            break;
                        }
                    }
                    break;
                }
                case 0x30:
                {
                    printf("[EE] PMFHL\n");
                    int rd = (opcode >> 11) & 0x1f;
                    int fmt = (opcode >> 6) & 0x1f;
                    if(rd)
                    {
                        switch(fmt)
                        {
                            case 0x00:
                            {
                                r[rd] = (u32)lo | ((u64)(u32)hi << 32);
                                rhi[rd] = (u32)lo1 | ((u64)(u32)hi1 << 32);
                                break;
                            }
                            case 0x01:
                            {
                                r[rd] = (lo >> 32) | (hi & 0xffffffff00000000ULL);
                                rhi[rd] = (lo1 >> 32) | (hi1 & 0xffffffff00000000ULL);
                                break;
                            }
                            case 0x02:
                            {
                                u64 lo_hi[2];
                                lo_hi[0] = (u32)lo | ((u64)(u32)hi << 32);
                                lo_hi[1] = (u32)lo1 | ((u64)(u32)hi1 << 32);

                                if(lo_hi[0] >= 0x80000000ULL) r[rd] = 0x7fffffffULL;
                                else if(lo_hi[0] < 0xffffffff80000000ULL) r[rd] = 0xffffffff80000000ULL;
                                else r[rd] = lo_hi[0];

                                if(lo_hi[1] >= 0x80000000ULL) rhi[rd] = 0x7fffffffULL;
                                else if(lo_hi[1] < 0xffffffff80000000ULL) rhi[rd] = 0xffffffff80000000ULL;
                                else rhi[rd] = lo_hi[1];
                                break;
                            }
                            case 0x03:
                            {
                                r[rd] = (u16)lo | (u64)(u32)(lo >> 16) | ((u64)(u16)hi << 32) | ((hi & 0xffff00000000ULL) << 16);
                                rhi[rd] = (u16)lo1 | (u64)(u32)(lo1 >> 16) | ((u64)(u16)hi1 << 32) | ((hi1 & 0xffff00000000ULL) << 16);
                                break;
                            }
                            case 0x04:
                            {
                                auto clamp = [](u32 data) -> s16
                                {
                                    return std::clamp<u32>(data, INT16_MIN, INT16_MAX);
                                };
                                r[rd] = clamp((u32)lo) | ((u32)clamp(lo >> 32) << 16) | ((u64)clamp((u32)hi) << 32) | ((u64)clamp(hi >> 32) << 48);
                                rhi[rd] = clamp((u32)lo1) | ((u32)clamp(lo1 >> 32) << 16) | ((u64)clamp((u32)hi1) << 32) | ((u64)clamp(hi1 >> 32) << 48);
                                break;
                            }
                        }
                    }
                    break;
                }
                case 0x31:
                {
                    printf("[EE] PMTHL\n");
                    int rs = (opcode >> 21) & 0x1f;

                    lo = (lo >> 32) << 32;
                    hi = (hi >> 32) << 32;
                    lo1 = (lo1 >> 32) << 32;
                    hi1 = (hi1 >> 32) << 32;

                    lo |= (u32)r[rs];
                    hi |= (u32)(r[rs] >> 32);
                    lo1 |= (u32)rhi[rs];
                    hi1 |= (u32)(rhi[rs] >> 32);
                    break;
                }
                case 0x34:
                {
                    printf("[EE] PSLLH\n");
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    int sa = (opcode >> 6) & 0xf;

                    u16 halfwords_rt[8];
                    u16 halfwords_rd[8];
                    *(u64*)halfwords_rt = r[rt];
                    *(u64*)(halfwords_rt + 4) = rhi[rt];

                    for(int i = 0; i < 8; i++)
                    {
                        halfwords_rd[i] = halfwords_rt[i] << sa;
                    }

                    if(rd)
                    {
                        r[rd] = *(u64*)halfwords_rd;
                        rhi[rd] = *(u64*)(halfwords_rd + 4);
                    }
                    break;
                }
                case 0x36:
                {
                    printf("[EE] PSRLH\n");
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    int sa = (opcode >> 6) & 0xf;

                    u16 halfwords_rt[8];
                    u16 halfwords_rd[8];
                    *(u64*)halfwords_rt = r[rt];
                    *(u64*)(halfwords_rt + 4) = rhi[rt];
                    
                    for(int i = 0; i < 8; i++)
                    {
                        halfwords_rd[i] = halfwords_rt[i] >> sa;
                    }

                    if(rd)
                    {
                        r[rd] = *(u64*)halfwords_rd;
                        rhi[rd] = *(u64*)(halfwords_rd + 4);
                    }
                    break;
                }
                case 0x37:
                {
                    printf("[EE] PSRAH\n");
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    int sa = (opcode >> 6) & 0xf;

                    s16 halfwords_rt[8];
                    s16 halfwords_rd[8];
                    *(u64*)halfwords_rt = r[rt];
                    *(u64*)(halfwords_rt + 4) = rhi[rt];
                    
                    for(int i = 0; i < 8; i++)
                    {
                        halfwords_rd[i] = halfwords_rt[i] >> sa;
                    }

                    if(rd)
                    {
                        r[rd] = *(u64*)halfwords_rd;
                        rhi[rd] = *(u64*)(halfwords_rd + 4);
                    }
                    break;
                }
                case 0x3c:
                {
                    printf("[EE] PSLLW\n");
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    int sa = (opcode >> 6) & 0x1f;

                    u32 words_rt[4];
                    u32 words_rd[4];
                    *(u64*)words_rt = r[rt];
                    *(u64*)(words_rt + 2) = rhi[rt];

                    for(int i = 0; i < 4; i++)
                    {
                        words_rd[i] = words_rt[i] << sa;
                    }

                    if(rd)
                    {
                        r[rd] = *(u64*)words_rd;
                        rhi[rd] = *(u64*)(words_rd + 2);
                    }
                    break;
                }
                case 0x3e:
                {
                    printf("[EE] PSRLW\n");
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    int sa = (opcode >> 6) & 0x1f;

                    u32 words_rt[4];
                    u32 words_rd[4];
                    *(u64*)words_rt = r[rt];
                    *(u64*)(words_rt + 2) = rhi[rt];
                    
                    for(int i = 0; i < 4; i++)
                    {
                        words_rd[i] = words_rt[i] >> sa;
                    }

                    if(rd)
                    {
                        r[rd] = *(u64*)words_rd;
                        rhi[rd] = *(u64*)(words_rd + 2);
                    }
                    break;
                }
                case 0x3f:
                {
                    printf("[EE] PSRAW\n");
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    int sa = (opcode >> 6) & 0x1f;

                    s32 words_rt[8];
                    s32 words_rd[8];
                    *(u64*)words_rt = r[rt];
                    *(u64*)(words_rt + 2) = rhi[rt];
                    
                    for(int i = 0; i < 4; i++)
                    {
                        words_rd[i] = words_rt[i] >> sa;
                    }

                    if(rd)
                    {
                        r[rd] = *(u64*)words_rd;
                        rhi[rd] = *(u64*)(words_rd + 2);
                    }
                    break;
                }
            }
            break;
        }
        case 0x1e:
        {
            printf("[EE] LQ\n");
            int base = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            s32 offset = (s16)(opcode & 0xffff);
            u32 addr = r[base] + offset;
            if(rt)
            {
                u128 data = rq(addr);
                r[rt] = data.lo;
                rhi[rt] = data.hi;
            }
            break;
        }
        case 0x1f:
        {
            printf("[EE] SQ\n");
            int base = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            s32 offset = (s16)(opcode & 0xffff);
            u32 addr = r[base] + offset;
            u128 data;
            data.lo = r[rt];
            data.hi = rhi[rt];
            wq(addr, data);
            break;
        }
        case 0x20:
        {
            printf("[EE] LB\n");
            int base = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            s32 offset = (s16)(opcode & 0xffff);
            u32 addr = r[base] + offset;
            if(rt)
            {
                s64 temp = (s8)rb(addr);
                r[rt] = (u64)temp;
            }
            break;
        }
        case 0x21:
        {
            printf("[EE] LH\n");
            int base = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            s32 offset = (s16)(opcode & 0xffff);
            u32 addr = r[base] + offset;
            if(rt)
            {
                s64 temp = (s16)rh(addr);
                r[rt] = (u64)temp;
            }
            break;
        }
        case 0x22:
        {
            printf("[EE] LWL\n");
            const u32 lwl_mask[4] = {0x00ffffff, 0x0000ffff, 0x000000ff, 0};
            const u8 lwl_shift[4] = {24, 16, 8, 0};
            int base = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            s32 offset = (s16)(opcode & 0xffff);
            u32 addr = r[base] + offset;
            int shift = addr & 3;
            if(rt)
            {
                u32 data = rw(addr & ~3);
                s64 data2 = (s32)((r[rt] & lwl_mask[shift]) | (data << lwl_shift[shift]));
                r[rt] = data2;
            }
            break;
        }
        case 0x23:
        {
            printf("[EE] LW\n");
            int base = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            s32 offset = (s16)(opcode & 0xffff);
            u32 addr = r[base] + offset;
            if(rt)
            {
                s64 temp = (s32)rw(addr);
                r[rt] = (u64)temp;
            }
            break;
        }
        case 0x24:
        {
            printf("[EE] LBU\n");
            int base = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            s32 offset = (s16)(opcode & 0xffff);
            u32 addr = r[base] + offset;
            if(rt) r[rt] = rb(addr);
            break;
        }
        case 0x25:
        {
            printf("[EE] LHU\n");
            int base = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            s32 offset = (s16)(opcode & 0xffff);
            u32 addr = r[base] + offset;
            if(rt) r[rt] = rh(addr);
            break;
        }
        case 0x26:
        {
            printf("[EE] LWR\n");
            const u32 lwr_mask[4] = {0, 0xff000000, 0xffff0000, 0xffffff00};
            const u8 lwr_shift[4] = {0, 8, 16, 24};
            int base = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            s32 offset = (s16)(opcode & 0xffff);
            u32 addr = r[base] + offset;
            int shift = addr & 3;
            if(rt)
            {
                u32 data = rw(addr & ~3);
                s64 data2 = (s32)((r[rt] & lwr_mask[shift]) | (data >> lwr_shift[shift]));
                r[rt] = data2;
            }
            break;
        }
        case 0x27:
        {
            printf("[EE] LWU\n");
            int base = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            s32 offset = (s16)(opcode & 0xffff);
            u32 addr = r[base] + offset;
            if(rt) r[rt] = rw(addr);
            break;
        }
        case 0x28:
        {
            printf("[EE] SB\n");
            int base = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            s32 offset = (s16)(opcode & 0xffff);
            u32 addr = r[base] + offset;
            wb(addr, (u8)r[rt]);
            break;
        }
        case 0x29:
        {
            printf("[EE] SH\n");
            int base = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            s32 offset = (s16)(opcode & 0xffff);
            u32 addr = r[base] + offset;
            ww(addr, (u16)r[rt]);
            break;
        }
        case 0x2a:
        {
            printf("[EE] SWL\n");
            const u32 swl_mask[4] = {0x00ffffff, 0x0000ffff, 0x000000ff, 0};
            const u8 swl_shift[4] = {24, 16, 8, 0};
            int base = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            s32 offset = (s16)(opcode & 0xffff);
            u32 addr = r[base] + offset;
            int shift = addr & 3;
            u32 data = rw(addr & ~3);
            data = (r[rt] >> swl_shift[shift]) | (data & swl_mask[shift]);
            ww(addr & ~3, data);
            break;
        }
        case 0x2b:
        {
            printf("[EE] SW\n");
            int base = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            s32 offset = (s16)(opcode & 0xffff);
            u32 addr = r[base] + offset;
            ww(addr, (u32)r[rt]);
            break;
        }
        case 0x2c:
        {
            printf("[EE] SDL\n");
            const u64 sdl_mask[8] = {0x00ffffffffffffffULL, 0x0000ffffffffffffULL, 0x000000ffffffffff, 0xffffffffULL,
            0x00ffffffULL, 0x0000ffffULL, 0xff, 0};
            const u8 sdl_shift[8] = {56, 48, 40, 32, 24, 16, 8, 0};
            int base = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            s32 offset = (s16)(opcode & 0xffff);
            u32 addr = r[base] + offset;
            int shift = addr & 7;
            u64 data = rd(addr & ~7);
            data = (r[rt] >> sdl_shift[shift]) | (data & sdl_mask[shift]);
            wd(addr & ~7, data);
            break;
        }
        case 0x2d:
        {
            printf("[EE] SDR\n");
            const u64 sdr_mask[8] = {0, 0xff00000000000000ULL, 0xffff000000000000ULL, 0xffffff0000000000ULL,
            0xffffffff00000000ULL, 0xffffffffff000000ULL, 0xffffffffffff0000ULL, 0xffffffffffffff00ULL};
            const u8 sdr_shift[8] = {0, 8, 16, 24, 32, 40, 48, 56};
            int base = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            s32 offset = (s16)(opcode & 0xffff);
            u32 addr = r[base] + offset;
            int shift = addr & 7;
            u64 data = rd(addr & ~7);
            data = (r[rt] << sdr_shift[shift]) | (data & sdr_mask[shift]);
            wd(addr & ~7, data);
            break;
        }
        case 0x2e:
        {
            printf("[EE] SWR\n");
            const u32 swr_mask[4] = {0, 0xff000000, 0xffff0000, 0xffffff00};
            const u8 swr_shift[4] = {0, 8, 16, 24};
            int base = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            s32 offset = (s16)(opcode & 0xffff);
            u32 addr = r[base] + offset;
            int shift = addr & 3;
            u32 data = rw(addr & ~3);
            data = (r[rt] << swr_shift[shift]) | (data & swr_mask[shift]);
            ww(addr & ~3, data);
            break;
        }
        case 0x2f:
        {
            int base = (opcode >> 21) & 0x1f;
            int op = (opcode >> 16) & 0x1f;
            s32 offset = (s16)(opcode & 0xffff);
            u32 addr = r[base] + offset;
            printf("[EE] CACHE op %02x addr %08x\n", op, addr);
            break;
        }
        case 0x31:
        {
            printf("[EE] LWC1\n");
            int base = (opcode >> 21) & 0x1f;
            int ft = (opcode >> 16) & 0x1f;
            s32 offset = (s16)(opcode & 0xffff);
            u32 addr = (u32)r[base] + offset;
            fpr[ft].uw = rw(addr);
            break;
        }
        case 0x33:
        {
            int base = (opcode >> 21) & 0x1f;
            int hint = (opcode >> 16) & 0x1f;
            s32 offset = (s16)(opcode & 0xffff);
            u32 addr = (u32)r[base] + offset;
            printf("[EE] PREF hint %02x addr %08x\n", hint, addr);
            break;
        }
        case 0x37:
        {
            printf("[EE] LD\n");
            int base = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            s32 offset = (s16)(opcode & 0xffff);
            u32 addr = (u32)r[base] + offset;
            if(rt) r[rt] = rd(addr);
            break;
        }
        case 0x39:
        {
            printf("[EE] SWC1\n");
            int base = (opcode >> 21) & 0x1f;
            int ft = (opcode >> 16) & 0x1f;
            s32 offset = (s16)(opcode & 0xffff);
            u32 addr = (u32)r[base] + offset;
            ww(addr, fpr[ft].uw);
            break;
        }
        case 0x3f:
        {
            printf("[EE] SD\n");
            int base = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            s32 offset = (s16)(opcode & 0xffff);
            u32 addr = (u32)r[base] + offset;
            wd(addr, r[rt]);
            break;
        }
    }

    if(inc_pc) pc += 4;
    else inc_pc = true;

    if(branch_on)
    {
        if(!delay_slot)
        {
            branch_on = false;
            pc = newpc;
        }
        else delay_slot--;
    }
}
#undef printf