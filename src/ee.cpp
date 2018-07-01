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
}

//TODO: This MMU emulation is COMPLETELY inaccurate, but it's good enough for now :/
u8 ee_cpu::rb(u32 addr)
{
    u32 phys_addr = addr & 0x1fffffff;
    return rb_real(device, phys_addr);
}

u16 ee_cpu::rh(u32 addr)
{
    u32 phys_addr = addr & 0x1fffffff;
    return rh_real(device, phys_addr);
}

u32 ee_cpu::rw(u32 addr)
{
    u32 phys_addr = addr & 0x1fffffff;
    return rw_real(device, phys_addr);
}

u64 ee_cpu::rd(u32 addr)
{
    u32 phys_addr = addr & 0x1fffffff;
    return rd_real(device, phys_addr);
}

u128 ee_cpu::rq(u32 addr)
{
    u32 phys_addr = addr & 0x1fffffff;
    return rq_real(device, phys_addr);
}

void ee_cpu::wb(u32 addr, u8 data)
{
    u32 phys_addr = addr & 0x1fffffff;
    wb_real(device, phys_addr, data);
}

void ee_cpu::wh(u32 addr, u16 data)
{
    u32 phys_addr = addr & 0x1fffffff;
    wh_real(device, phys_addr, data);
}

void ee_cpu::ww(u32 addr, u32 data)
{
    u32 phys_addr = addr & 0x1fffffff;
    ww_real(device, phys_addr, data);
}

void ee_cpu::wd(u32 addr, u64 data)
{
    u32 phys_addr = addr & 0x1fffffff;
    wd_real(device, phys_addr, data);
}

void ee_cpu::wq(u32 addr, u128 data)
{
    u32 phys_addr = addr & 0x1fffffff;
    wq_real(device, phys_addr, data);
}

void ee_cpu::tick()
{
    u32 opcode = rw(pc);
    printf("[EE] Opcode: %08x\n[EE] PC: %08x\n", opcode, pc);

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
                    s64 result = (s32)r[rs] * (s32)r[rt];
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
                    u64 result = (u32)r[rs] * (u32)r[rt];
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
                    if(!r[rt])
                    {
                        lo = 0xffffffff;
                        hi = r[rs];
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
                    if(rd) r[rd] = sa;
                    break;
                }
                case 0x29:
                {
                    printf("[EE] MTSA\n");
                    int rs = (opcode >> 21) & 0x1f;
                    sa = r[rs];
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
                    if(rd) r[rd] = r[rs] + r[rt];
                    break;
                }
                case 0x2d:
                {
                    printf("[EE] DADDU\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    if(rd) r[rd] = r[rs] + r[rt];
                    break;
                }
                case 0x2e:
                {
                    printf("[EE] DSUB\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    if(rd) r[rd] = r[rs] - r[rt];
                    break;
                }
                case 0x2f:
                {
                    printf("[EE] DSUBU\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    if(rd) r[rd] = r[rs] - r[rt];
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
                    sa = ((r[rs] & 0xf) ^ (imm & 0xf)) << 3;
                    break;
                }
                case 0x19:
                {
                    printf("[EE] MTSAH\n");
                    int rs = (opcode >> 21) & 0x1f;
                    u16 imm = opcode & 0xffff;
                    sa = ((r[rs] & 0x7) ^ (imm & 0x7)) << 4;
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
            s64 imm = (s16)(opcode & 0xffff);
            if(rt) r[rt] &= imm;
            break;
        }
        case 0x0d:
        {
            printf("[EE] ORI\n");
            int rs = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            s64 imm = (s16)(opcode & 0xffff);
            if(rt) r[rt] |= imm;
            break;
        }
        case 0x0e:
        {
            printf("[EE] XORI\n");
            int rs = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            s64 imm = (s16)(opcode & 0xffff);
            if(rt) r[rt] ^= imm;
            break;
        }
        case 0x0f:
        {
            printf("[EE] LUI\n");
            int rt = (opcode >> 16) & 0x1f;
            s64 imm = (s16)(opcode & 0xffff);
            imm <<= 16;
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
                r[rt] = r[rt] = (r[rt] & ldr_mask[shift]) | (data >> ldr_shift[shift]);;
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
            u32 addr = r[base] + offset;
            fpr[ft].uw = rw(addr);
            break;
        }
        case 0x33:
        {
            int base = (opcode >> 21) & 0x1f;
            int hint = (opcode >> 16) & 0x1f;
            s32 offset = (s16)(opcode & 0xffff);
            u32 addr = r[base] + offset;
            printf("[EE] PREF hint %02x addr %08x\n", hint, addr);
            break;
        }
        case 0x37:
        {
            printf("[EE] LD\n");
            int base = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            s32 offset = (s16)(opcode & 0xffff);
            u32 addr = r[base] + offset;
            if(rt) r[rt] = rd(addr);
            break;
        }
        case 0x39:
        {
            printf("[EE] SWC1\n");
            int base = (opcode >> 21) & 0x1f;
            int ft = (opcode >> 16) & 0x1f;
            s32 offset = (s16)(opcode & 0xffff);
            u32 addr = r[base] + offset;
            ww(addr, fpr[ft].uw);
            break;
        }
        case 0x3f:
        {
            printf("[EE] SD\n");
            int base = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            s32 offset = (s16)(opcode & 0xffff);
            u32 addr = r[base] + offset;
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