#include "iop.h"

void iop_cpu::init()
{
    for(int i = 0; i < 32; i++)
    {
        r[i] = 0;
    }

    pc = 0xbfc00000;
    inc_pc = true;
    delay_slot = 0;
    branch_on = false;

    cop0_count = 0;
    cop0_status.whole = 0;
    cop0_status.boot_except_vectors_rom = 1;
}

//TODO: This MMU emulation is COMPLETELY inaccurate, but it's good enough for now :/
u32 iop_cpu::translate_addr(u32 addr)
{
    if(addr >= 0x80000000 && addr < 0xa0000000) return addr - 0x80000000;
    if(addr >= 0xa0000000 && addr < 0xc0000000) return addr - 0xa0000000;
    return addr;
}

u8 iop_cpu::rb(u32 addr)
{
    u32 phys_addr = translate_addr(addr);
    return rb_real(device, phys_addr);
}

u16 iop_cpu::rh(u32 addr)
{
    u32 phys_addr = translate_addr(addr);
    return rh_real(device, phys_addr);
}

u32 iop_cpu::rw(u32 addr)
{
    u32 phys_addr = translate_addr(addr);
    return rw_real(device, phys_addr);
}

void iop_cpu::wb(u32 addr, u8 data)
{
    if(!cop0_status.isolate_cache)
    {
        u32 phys_addr = translate_addr(addr);
        wb_real(device, phys_addr, data);
    }
}

void iop_cpu::wh(u32 addr, u16 data)
{
    if(!cop0_status.isolate_cache)
    {
        u32 phys_addr = translate_addr(addr);
        wh_real(device, phys_addr, data);
    }
}

void iop_cpu::ww(u32 addr, u32 data)
{
    if(!cop0_status.isolate_cache)
    {
        u32 phys_addr = translate_addr(addr);
        ww_real(device, phys_addr, data);
    }
}

void iop_cpu::tick()
{
    u32 opcode = rw(pc);
    printf("[IOP] Opcode: %08x\n[IOP] PC: %08x\n", opcode, pc);
    for(int i = 0; i < 32; i++)
    {
        printf("[IOP] R%d: %08x\n", i, r[i]);
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
                    printf("[IOP] SLL\n");
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    int sa = (opcode >> 6) & 0x1f;
                    if(rd) r[rd] = r[rt] << sa;
                    break;
                }
                case 0x02:
                {
                    printf("[IOP] SRL\n");
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    int sa = (opcode >> 6) & 0x1f;
                    if(rd) r[rd] = r[rt] >> sa;
                    break;
                }
                case 0x03:
                {
                    printf("[IOP] SRA\n");
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    int sa = (opcode >> 6) & 0x1f;
                    if(rd) r[rd] = (s32)r[rt] >> sa;
                    break;
                }
                case 0x04:
                {
                    printf("[IOP] SLLV\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    s32 temp = r[rt] << (r[rs] & 0x1f);
                    if(rd) r[rd] = temp;
                    break;
                }
                case 0x06:
                {
                    printf("[IOP] SRLV\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    s32 temp = r[rt] >> (r[rs] & 0x1f);
                    if(rd) r[rd] = temp;
                    break;
                }
                case 0x07:
                {
                    printf("[IOP] SRAV\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    s32 temp = (s32)r[rt] >> (r[rs] & 0x1f);
                    if(rd) r[rd] = temp;
                    break;
                }
                case 0x08:
                {
                    printf("[IOP] JR\n");
                    int rs = (opcode >> 21) & 0x1f;
                    branch_on = true;
                    newpc = r[rs];
                    delay_slot = 1;
                    break;
                }
                case 0x09:
                {
                    printf("[IOP] JALR\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    u32 return_addr = pc + 8;
                    branch_on = true;
                    newpc = r[rs];
                    delay_slot = 1;
                    if(rd) r[rd] = return_addr;
                    break;
                }
                case 0x10:
                {
                    printf("[IOP] MFHI\n");
                    int rd = (opcode >> 11) & 0x1f;
                    if(rd) r[rd] = hi;
                    break;
                }
                case 0x11:
                {
                    printf("[IOP] MTHI\n");
                    int rs = (opcode >> 21) & 0x1f;
                    hi = r[rs];
                    break;
                }
                case 0x12:
                {
                    printf("[IOP] MFLO\n");
                    int rd = (opcode >> 11) & 0x1f;
                    if(rd) r[rd] = lo;
                    break;
                }
                case 0x13:
                {
                    printf("[IOP] MTLO\n");
                    int rs = (opcode >> 21) & 0x1f;
                    lo = r[rs];
                    break;
                }
                case 0x18:
                {
                    printf("[IOP] MULT\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    s64 result = (s64)(s32)r[rs] * (s64)(s32)r[rt];
                    lo = (u32)result;
                    hi = result >> 32;
                    break;
                }
                case 0x19:
                {
                    printf("[IOP] MULTU\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    u64 result = (u64)r[rs] * (u64)r[rt];
                    lo = (u32)result;
                    hi = result >> 32;
                    break;
                }
                case 0x1a:
                {
                    printf("[IOP] DIV\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    if(!r[rt])
                    {
                        hi = r[rs];
                        if((s32)r[rs] > 0x80000000) lo = 1;
                        else lo = 0xffffffff;
                    }
                    else if(r[rs] == 0x80000000 && r[rt] == 0xffffffff)
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
                    printf("[IOP] DIVU\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    if(!r[rt])
                    {
                        lo = 0xffffffff;
                        hi = r[rs];
                    }
                    else
                    {
                        lo = r[rs] / r[rt];
                        hi = r[rs] % r[rt];
                    }
                    break;
                }
                case 0x20:
                {
                    printf("[IOP] ADD\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    if(rd) r[rd] = r[rs] + r[rt];
                    break;
                }
                case 0x21:
                {
                    printf("[IOP] ADDU\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    if(rd) r[rd] = r[rs] + r[rt];
                    break;
                }
                case 0x22:
                {
                    printf("[IOP] SUB\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    if(rd) r[rd] = r[rs] - r[rt];
                    break;
                }
                case 0x23:
                {
                    printf("[IOP] SUBU\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    if(rd) r[rd] = r[rs] - r[rt];
                    break;
                }
                case 0x24:
                {
                    printf("[IOP] AND\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    if(rd) r[rd] = r[rs] & r[rt];
                    break;
                }
                case 0x25:
                {
                    printf("[IOP] OR\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    if(rd) r[rd] = r[rs] | r[rt];
                    break;
                }
                case 0x26:
                {
                    printf("[IOP] XOR\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    if(rd) r[rd] = r[rs] ^ r[rt];
                    break;
                }
                case 0x27:
                {
                    printf("[IOP] NOR\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    if(rd) r[rd] = ~(r[rs] | r[rt]);
                    break;
                }
                case 0x2a:
                {
                    printf("[IOP] SLT\n");
                    int rs = (opcode >> 21) & 0x1f;
                    int rt = (opcode >> 16) & 0x1f;
                    int rd = (opcode >> 11) & 0x1f;
                    if(rd)
                    {
                        if((s32)r[rs] < (s32)r[rt]) r[rd] = 1;
                        else r[rd] = 0;
                    }
                    break;
                }
                case 0x2b:
                {
                    printf("[IOP] SLTU\n");
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
            }
            break;
        }
        case 0x01:
        {
            switch((opcode >> 16) & 0x1f)
            {
                case 0x00:
                {
                    printf("[IOP] BLTZ\n");
                    int rs = (opcode >> 21) & 0x1f;
                    s32 offset = (s16)(opcode & 0xffff);
                    offset <<= 2;
                    if((s32)r[rs] < 0)
                    {
                        branch_on = true;
                        newpc = pc + offset + 4;
                        delay_slot = 1;
                    }
                    break;
                }
                case 0x01:
                {
                    printf("[IOP] BGEZ\n");
                    int rs = (opcode >> 21) & 0x1f;
                    s32 offset = (s16)(opcode & 0xffff);
                    offset <<= 2;
                    if((s32)r[rs] >= 0)
                    {
                        branch_on = true;
                        newpc = pc + offset + 4;
                        delay_slot = 1;
                    }
                    break;
                }
                case 0x10:
                {
                    printf("[IOP] BLTZAL\n");
                    int rs = (opcode >> 21) & 0x1f;
                    s32 offset = (s16)(opcode & 0xffff);
                    offset <<= 2;
                    if((s32)r[rs] < 0)
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
                    printf("[IOP] BGEZAL\n");
                    int rs = (opcode >> 21) & 0x1f;
                    s32 offset = (s16)(opcode & 0xffff);
                    offset <<= 2;
                    if((s32)r[rs] >= 0)
                    {
                        branch_on = true;
                        newpc = pc + offset + 4;
                        delay_slot = 1;
                        r[31] = pc + 8;
                    }
                    break;
                }
            }
            break;
        }
        case 0x02:
        {
            printf("[IOP] J\n");
            u32 addr = (opcode & 0x3ffffff) << 2;
            addr += (pc + 4) & 0xf0000000;
            branch_on = true;
            newpc = addr;
            delay_slot = 1;
            break;
        }
        case 0x03:
        {
            printf("[IOP] JAL\n");
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
            printf("[IOP] BEQ\n");
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
            printf("[IOP] BNE\n");
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
            printf("[IOP] BLEZ\n");
            int rs = (opcode >> 21) & 0x1f;
            s32 offset = (s16)(opcode & 0xffff);
            offset <<= 2;
            if((s32)r[rs] <= 0)
            {
                branch_on = true;
                newpc = pc + offset + 4;
                delay_slot = 1;
            }
            break;
        }
        case 0x07:
        {
            printf("[IOP] BGTZ\n");
            int rs = (opcode >> 21) & 0x1f;
            s32 offset = (s16)(opcode & 0xffff);
            offset <<= 2;
            if((s32)r[rs] > 0)
            {
                branch_on = true;
                newpc = pc + offset + 4;
                delay_slot = 1;
            }
            break;
        }
        case 0x08:
        {
            printf("[IOP] ADDI\n");
            int rs = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            s32 imm = (s16)(opcode & 0xffff);
            if(rt)
            {
                s32 temp = (s32)(r[rs] + imm);
                r[rt] = (u32)temp;
            }
            break;
        }
        case 0x09:
        {
            printf("[IOP] ADDIU\n");
            int rs = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            s32 imm = (s16)(opcode & 0xffff);
            if(rt)
            {
                s32 temp = (s32)(r[rs] + imm);
                r[rt] = (u32)temp;
            }
            break;
        }
        case 0x0a:
        {
            printf("[IOP] SLTI\n");
            int rs = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            s32 imm = (s16)(opcode & 0xffff);
            if(rt)
            {
                if((s32)r[rs] < imm) r[rt] = 1;
                else r[rt] = 0;
            }
            break;
        }
        case 0x0b:
        {
            printf("[IOP] SLTIU\n");
            int rs = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            s32 imm = (s16)(opcode & 0xffff);
            if(rt)
            {
                if(r[rs] < (u32)imm) r[rt] = 1;
                else r[rt] = 0;
            }
            break;
        }
        case 0x0c:
        {
            printf("[IOP] ANDI\n");
            int rs = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            u16 imm = opcode & 0xffff;
            if(rt) r[rt] = r[rs] & imm;
            break;
        }
        case 0x0d:
        {
            printf("[IOP] ORI\n");
            int rs = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            u16 imm = opcode & 0xffff;
            if(rt) r[rt] = r[rs] | imm;
            break;
        }
        case 0x0e:
        {
            printf("[IOP] XORI\n");
            int rs = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            u16 imm = opcode & 0xffff;
            if(rt) r[rt] = r[rs] ^ imm;
            break;
        }
        case 0x0f:
        {
            printf("[IOP] LUI\n");
            int rt = (opcode >> 16) & 0x1f;
            s32 imm = (s16)(opcode & 0xffff);
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
                    printf("[IOP] MFC0\n");
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
                            if(rt) r[rt] = 0x0000001f; //TODO: hpsx64 value. VERIFY!
                            break;
                        }
                    }
                    break;
                }
                case 0x04:
                {
                    printf("[IOP] MTC0\n");
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
                            printf("[IOP] BC0F\n");
                            break;
                        }
                        case 0x01:
                        {
                            printf("[IOP] BC0T\n");
                            break;
                        }
                        case 0x02:
                        {
                            printf("[IOP] BC0FL\n");
                            break;
                        }
                        case 0x03:
                        {
                            printf("[IOP] BC0TL\n");
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
                            printf("[IOP] TLBR\n");
                            break;
                        }
                        case 0x02:
                        {
                            printf("[IOP] TLBWI\n");
                            break;
                        }
                        case 0x06:
                        {
                            printf("[IOP] TLBWR\n");
                            break;
                        }
                        case 0x10:
                        {
                            printf("[IOP] RFE\n");
                            break;
                        }
                    }
                    break;
                }
            }
            break;
        }
        case 0x20:
        {
            printf("[IOP] LB\n");
            int base = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            s32 offset = (s16)(opcode & 0xffff);
            u32 addr = r[base] + offset;
            if(rt) r[rt] = (s32)(s8)rb(addr);
            break;
        }
        case 0x21:
        {
            printf("[IOP] LH\n");
            int base = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            s32 offset = (s16)(opcode & 0xffff);
            u32 addr = r[base] + offset;
            if(rt) r[rt] = (s32)(s16)rh(addr);
            break;
        }
        case 0x22:
        {
            printf("[IOP] LWL\n");
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
                data = (r[rt] & lwl_mask[shift]) | (data << lwl_shift[shift]);
                r[rt] = data;
            }
            break;
        }
        case 0x23:
        {
            printf("[IOP] LW\n");
            int base = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            s32 offset = (s16)(opcode & 0xffff);
            u32 addr = r[base] + offset;
            if(rt) r[rt] = rw(addr);
            break;
        }
        case 0x24:
        {
            printf("[IOP] LBU\n");
            int base = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            s32 offset = (s16)(opcode & 0xffff);
            u32 addr = r[base] + offset;
            if(rt) r[rt] = rb(addr);
            break;
        }
        case 0x25:
        {
            printf("[IOP] LHU\n");
            int base = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            s32 offset = (s16)(opcode & 0xffff);
            u32 addr = r[base] + offset;
            if(rt) r[rt] = rh(addr);
            break;
        }
        case 0x26:
        {
            printf("[IOP] LWR\n");
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
                data = (r[rt] & lwr_mask[shift]) | (data >> lwr_shift[shift]);
                r[rt] = data;
            }
            break;
        }
        case 0x28:
        {
            printf("[IOP] SB\n");
            int base = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            s32 offset = (s16)(opcode & 0xffff);
            u32 addr = r[base] + offset;
            wb(addr, (u8)r[rt]);
            break;
        }
        case 0x29:
        {
            printf("[IOP] SH\n");
            int base = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            s32 offset = (s16)(opcode & 0xffff);
            u32 addr = r[base] + offset;
            wh(addr, (u16)r[rt]);
            break;
        }
        case 0x2a:
        {
            printf("[IOP] SWL\n");
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
            printf("[IOP] SW\n");
            int base = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            s32 offset = (s16)(opcode & 0xffff);
            u32 addr = r[base] + offset;
            ww(addr, r[rt]);
            break;
        }
        case 0x3e:
        {
            printf("[IOP] SWR\n");
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