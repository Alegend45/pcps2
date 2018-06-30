#include "iop.h"

void iop_cpu::init()
{
    for(int i = 0; i < 32; i++)
    {
        r[i] = 0;
    }

    pc = 0x1fc00000;
    inc_pc = true;
    delay_slot = 0;
    branch_on = false;
}

u32 iop_cpu::rw(u32 addr)
{
    return rw_real(device, addr);
}

void iop_cpu::ww(u32 addr, u32 data)
{
    ww_real(device, addr, data);
}

void iop_cpu::tick()
{
    u32 opcode = rw(pc);
    printf("[IOP] Opcode: %08x\n[IOP] PC: %08x\n", opcode, pc);

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
                case 0x08:
                {
                    printf("[IOP] JR\n");
                    int rs = (opcode >> 21) & 0x1f;
                    branch_on = true;
                    newpc = r[rs];
                    delay_slot = 1;
                    break;
                }
            }
            break;
        }
        case 0x05:
        {
            printf("[IOP] BNEZ\n");
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
        case 0x0c:
        {
            printf("[IOP] ANDI\n");
            int rs = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            s32 imm = (s16)(opcode & 0xffff);
            if(rt) r[rt] &= imm;
            break;
        }
        case 0x0d:
        {
            printf("[IOP] ORI\n");
            int rs = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            s32 imm = (s16)(opcode & 0xffff);
            if(rt) r[rt] |= imm;
            break;
        }
        case 0x0e:
        {
            printf("[IOP] XORI\n");
            int rs = (opcode >> 21) & 0x1f;
            int rt = (opcode >> 16) & 0x1f;
            s32 imm = (s16)(opcode & 0xffff);
            if(rt) r[rt] ^= imm;
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
                        case 0x18:
                        {
                            printf("[IOP] ERET\n");
                            break;
                        }
                        case 0x34:
                        {
                            printf("[IOP] EI\n");
                            break;
                        }
                        case 0x35:
                        {
                            printf("[IOP] DI\n");
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