#include "ee.h"

void ee_cpu::init()
{
    for(int i = 0; i < 32; i++)
    {
        r[i] = 0;
        rhi[i] = 0;
    }

    pc = 0x1fc00000;
    inc_pc = true;
    delay_slot = 0;
    branch_on = false;
}

u32 ee_cpu::rw(u32 addr)
{
    return rw_real(device, addr);
}

void ee_cpu::ww(u32 addr, u32 data)
{
    ww_real(device, addr, data);
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
                case 0x08:
                {
                    printf("[EE] JR\n");
                    int rs = (opcode >> 21) & 0x1f;
                    branch_on = true;
                    newpc = (u32)r[rs];
                    delay_slot = 1;
                    break;
                }
            }
            break;
        }
        case 0x05:
        {
            printf("[EE] BNEZ\n");
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
                    }
                    break;
                }
                case 0x04:
                {
                    printf("[EE] MTC0\n");
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
                        case 0x34:
                        {
                            printf("[EE] EI\n");
                            break;
                        }
                        case 0x35:
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