#include <cstdint>

/*
    Registers:
    - Special purpose registers:
        - PC: 16-bit, program counter
        - SP: 16-bit, stack pointer
        - IX: 16-bit, index register
        - IY: 16-bit, index register
        - I: 8-bit, interrupt page address register
        - R: 8-bit, memory refresh register
        - A: 8-bit, accumulator
        - F: 8-bit, flag register for A
        - A': 8-bit, accumulator
        - F': 8-bit, flag register for A'

    - General purpose registers:
        - B, C, D, E, B', C', D', E': 8-bit
        - BC, DE, HL, B'C', D'E', H'L': 16-bit
    
*/

uint16_t R_PC = 0;
uint16_t R_SP = 0;
uint16_t R_IX = 0;
uint16_t R_IY = 0;
uint8_t R_I = 0;
uint8_t R_R = 0;
uint8_t R_A = 0;
uint8_t R_F = 0;
uint8_t R_A1 = 0;
uint8_t R_F1 = 0;

uint8_t R_B = 0;
uint8_t R_C = 0;
uint8_t R_D = 0;
uint8_t R_E = 0;
uint8_t R_B1 = 0;
uint8_t R_C1 = 0;
uint8_t R_D1 = 0;
uint8_t R_E1 = 0;

typedef enum
{
    OP_NOP = 0,
    OP_LD_IMM_BC = 1,
    OP_LD_A_BC = 2,
    OP_INC_BC = 3,
    OP_INC_B = 4,
    OP_DEC_B = 5,
    OP_LD_IMM_B = 6,
    OP_RLCA = 7,
} main_instr_enum;

typedef struct instr
{
    int op;
    uint8_t instrSize;
    uint8_t cycles;
} instr;

instr z80Instr[256] = {
    {0, 1, 4}, // 0x00: NOP
    {1, 3, 10}, // 0x01: LD BC,HHLL
    {2, 1, 7}, // 0x02: LD (BC),A
    {3, 1, 6}, // 0x03: INC BC
    {4, 1, 4}, // 0x04: INC B
    {5, 1, 4}, // 0x05: DEC B
    {6, 2, 7}, // 0x06: LD B,NN
    {7, 1, 4}, // 0x07: RLCA
    {8, 1, 4}, // 0x08: EX AF,AF’
    {9, 1, 11}, // 0x09: ADD HL,BC
    {10, 1, 7}, // 0x0A: LD A,(BC)
    {11, 1, 6}, // 0x0B: DEC BC
    {12, 1, 4}, // 0x0C: INC C
    {13, 1, 4}, // 0x0D: DEC C
    {14, 2, 7}, // 0x0E: LD C,NN
    {15, 1, 4}, // 0x0F: CA
    {16, 2, 13}, // 0x10: DJNZ NN
    {17, 3, 10}, // 0x11: LD DE,HHLL
    {18, 1, 7}, // 0x12: LD (DE),A
    {19, 1, 6}, // 0x13: INC DE
    {20, 1, 4}, // 0x14: INC D
    {21, 1, 4}, // 0x15: DEC D
    {22, 2, 7}, // 0x16: LD D,NN
    {23, 1, 4}, // 0x17: RLA
    {24, 2, 12}, // 0x18: JR NN
    {25, 1, 11}, // 0x19: ADD HL,DE
    {26, 1, 7}, // 0x1A: LD A,(DE)
    {27, 1, 6}, // 0x1B: DEC DE
    {28, 1, 4}, // 0x1C: INC E
    {29, 1, 4}, // 0x1D: DEC E
    {30, 2, 7}, // 0x1E: LD E,NN
    {31, 1, 4}, // 0x1F: RRA
    {32, 2, 12}, // 0x20: JR NZ,NN
    {33, 3, 10}, // 0x21: LD HL,HHLL
    {34, 3, 16}, // 0x22: LD (HHLL),HL
    {35, 1, 6}, // 0x23: INC HL
    {36, 1, 4}, // 0x24: INC H
    {37, 1, 4}, // 0x25: DEC H
    {38, 2, 7}, // 0x26: LD H,NN
    {39, 1, 4}, // 0x27: DAA
    {40, 2, 12}, // 0x28: JR Z,NN
    {41, 1, 11}, // 0x29: ADD HL,HL
    {42, 3, 16}, // 0x2A: LD HL,(HHLL)
    {43, 1, 6}, // 0x2B: DEC HL
    {44, 1, 4}, // 0x2C: INC L
    {45, 1, 4}, // 0x2D: DEC L
    {46, 2, 7}, // 0x2E: LD L,NN
    {47, 1, 4}, // 0x2F: CPL
    {48, 2, 12}, // 0x30: JR NC,NN
    {49, 3, 10}, // 0x31: LD SP,HHLL
    {50, 3, 16}, // 0x32: LD (HHLL),A
    {51, 1, 6}, // 0x33: INC SP
    {52, 1, 11}, // 0x34: INC (HL)
    {53, 1, 11}, // 0x35: DEC (HL)
    {54, 2, 10}, // 0x36: LD (HL),NN
    {55, 1, 4}, // 0x37: SCF
    {56, 2, 12}, // 0x38: JR C,NN
    {57, 1, 11}, // 0x39: ADD HL,SP
    {58, 3, 13}, // 0x3A: LD A,(HHLL)
    {59, 1, 6}, // 0x3B: DEC SP
    {60, 1, 4}, // 0x3C: INC A
    {61, 1, 4}, // 0x3D: DEC A
    {62, 2, 7}, // 0x3E: LD A,NN
    {63, 1, 4}, // 0x3F: CCF
    {64, 1, 4}, // 0x40: LD B,B
    {65, 1, 4}, // 0x41: LD B,C
    {66, 1, 4}, // 0x42: LD B,D
    {67, 1, 4}, // 0x43: LD B,E
    {68, 1, 4}, // 0x44: LD B,H
    {69, 1, 4}, // 0x45: LD B,L
    {70, 1, 7}, // 0x46: LD B,(HL)
    {71, 1, 4}, // 0x47: LD B,A
    {72, 1, 4}, // 0x48: LD C,B
    {73, 1, 4}, // 0x49: LD C,C
    {74, 1, 4}, // 0x4A: LD C,D
    {75, 1, 4}, // 0x4B: LD C,E
    {76, 1, 4}, // 0x4C: LD C,H
    {77, 1, 4}, // 0x4D: LD C,L
    {78, 1, 7}, // 0x4E: LD C,(HL)
    {79, 1, 4}, // 0x4F: LD C,A
    {80, 1, 4}, // 0x50: LD D,B
    {81, 1, 4}, // 0x51: LD D,C
    {82, 1, 4}, // 0x52: LD D,D
    {83, 1, 4}, // 0x53: LD D,E
    {84, 1, 4}, // 0x54: LD D,H
    {85, 1, 4}, // 0x55: LD D,L
    {86, 1, 7}, // 0x56: LD D,(HL)
    {87, 1, 4}, // 0x57: LD D,A
    {88, 1, 4}, // 0x58: LD E,B
    {89, 1, 4}, // 0x59: LD E,C
    {90, 1, 4}, // 0x5A: LD E,D
    {91, 1, 4}, // 0x5B: LD E,E
    {92, 1, 4}, // 0x5C: LD E,H
    {93, 1, 4}, // 0x5D: LD E,L
    {94, 1, 7}, // 0x5E: LD E,(HL)
    {95, 1, 4}, // 0x5F: LD E,A
    {96, 1, 4}, // 0x60: LD H,B
    {97, 1, 4}, // 0x61: LD H,C
    {98, 1, 4}, // 0x62: LD H,D
    {99, 1, 4}, // 0x63: LD H,E
    {100, 1, 4}, // 0x64: LD H,H
    {101, 1, 4}, // 0x65: LD H,L
    {102, 1, 7}, // 0x66: LD H,(HL)
    {103, 1, 4}, // 0x67: LD H,A
    {104, 1, 4}, // 0x68: LD L,B
    {105, 1, 4}, // 0x69: LD L,C
    {106, 1, 4}, // 0x6A: LD L,D
    {107, 1, 4}, // 0x6B: LD L,E
    {108, 1, 4}, // 0x6C: LD L,H
    {109, 1, 4}, // 0x6D: LD L,L
    {110, 1, 7}, // 0x6E: LD L,(HL)
    {111, 1, 4}, // 0x6F: LD L,A
    {112, 1, 7}, // 0x70: LD (HL),B
    {113, 1, 7}, // 0x71: LD (HL),C
    {114, 1, 7}, // 0x72: LD (HL),D
    {115, 1, 7}, // 0x73: LD (HL),E
    {116, 1, 7}, // 0x74: LD (HL),H
    {117, 1, 7}, // 0x75: LD (HL),L
    {118, 1, 4}, // 0x76: HALT
    {119, 1, 7}, // 0x77: LD (HL),A
    {120, 1, 4}, // 0x78: LD A,B
    {121, 1, 4}, // 0x79: LD A,C
    {122, 1, 4}, // 0x7A: LD A,D
    {123, 1, 4}, // 0x7B: LD A,E
    {124, 1, 4}, // 0x7C: LD A,H
    {125, 1, 4}, // 0x7D: LD A,L
    {126, 1, 7}, // 0x7E: LD A,(HL)
    {127, 1, 4}, // 0x7F: LD A,A
    {128, 1, 4}, // 0x80: ADD A,B
    {129, 1, 4}, // 0x81: ADD A,C
    {130, 1, 4}, // 0x82: ADD A,D
    {131, 1, 4}, // 0x83: ADD A,E
    {132, 1, 4}, // 0x84: ADD A,H
    {133, 1, 4}, // 0x85: ADD A,L
    {134, 1, 7}, // 0x86: ADD A,(HL)
    {135, 1, 4}, // 0x87: ADD A,A
    {136, 1, 4}, // 0x88: ADC A,B
    {137, 1, 4}, // 0x89: ADC A,C
    {138, 1, 4}, // 0x8A: ADC A,D
    {139, 1, 4}, // 0x8B: ADC A,E
    {140, 1, 4}, // 0x8C: ADC A,H
    {141, 1, 4}, // 0x8D: ADC A,L
    {142, 1, 7}, // 0x8E: ADC A,(HL)
    {143, 1, 4}, // 0x8F: ADC A,A
    {144, 1, 4}, // 0x90: SUB A,B
    {145, 1, 4}, // 0x91: SUB A,C
    {146, 1, 4}, // 0x92: SUB A,D
    {147, 1, 4}, // 0x93: SUB A,E
    {148, 1, 4}, // 0x94: SUB A,H
    {149, 1, 4}, // 0x95: SUB A,L
    {150, 1, 7}, // 0x96: SUB A,(HL)
    {151, 1, 4}, // 0x97: SUB A,A
    {152, 1, 4}, // 0x98: SBC A,B
    {153, 1, 4}, // 0x99: SBC A,C
    {154, 1, 4}, // 0x9A: SBC A,D
    {155, 1, 4}, // 0x9B: SBC A,E
    {156, 1, 4}, // 0x9C: SBC A,H
    {157, 1, 4}, // 0x9D: SBC A,L
    {158, 1, 7}, // 0x9E: SBC A,(HL)
    {159, 1, 4}, // 0x9F: SBC A,A
    {160, 1, 4}, // 0xA0: AND B
    {161, 1, 4}, // 0xA1: AND C
    {162, 1, 4}, // 0xA2: AND D
    {163, 1, 4}, // 0xA3: AND E
    {164, 1, 4}, // 0xA4: AND H
    {165, 1, 4}, // 0xA5: AND L
    {166, 1, 7}, // 0xA6: AND (HL)
    {167, 1, 4}, // 0xA7: AND A
    {168, 1, 4}, // 0xA8: XOR B
    {169, 1, 4}, // 0xA9: XOR C
    {170, 1, 4}, // 0xAA: XOR D
    {171, 1, 4}, // 0xAB: XOR E
    {172, 1, 4}, // 0xAC: XOR H
    {173, 1, 4}, // 0xAD: XOR L
    {174, 1, 7}, // 0xAE: XOR (HL)
    {175, 1, 4}, // 0xAF: XOR A
    {176, 1, 4}, // 0xB0: OR B
    {177, 1, 4}, // 0xB1: OR C
    {178, 1, 4}, // 0xB2: OR D
    {179, 1, 4}, // 0xB3: OR E
    {180, 1, 4}, // 0xB4: OR H
    {181, 1, 4}, // 0xB5: OR L
    {182, 1, 7}, // 0xB6: OR (HL)
    {183, 1, 4}, // 0xB7: OR A
    {184, 1, 4}, // 0xB8: CP B
    {185, 1, 4}, // 0xB9: CP C
    {186, 1, 4}, // 0xBA: CP D
    {187, 1, 4}, // 0xBB: CP E
    {188, 1, 4}, // 0xBC: CP H
    {189, 1, 4}, // 0xBD: CP L
    {190, 1, 7}, // 0xBE: CP (HL)
    {191, 1, 4}, // 0xBF: CP A
    {192, 1, 11}, // 0xC0: RET NZ
    {193, 1, 10}, // 0xC1: POP BC
    {194, 3, 10}, // 0xC2: JP NZ,HHLL
    {195, 3, 10}, // 0xC3: JP HHLL
    {196, 3, 17}, // 0xC4: CALL NZ,HHLL
    {197, 1, 10}, // 0xC5: PUSH BC
    {198, 2, 7}, // 0xC6: ADD A,NN
    {199, 1, 11}, // 0xC7: RST 00
    {200, 1, 11}, // 0xC8: RET Z
    {201, 1, 10}, // 0xC9: RET
    {202, 3, 10}, // 0xCA: JP Z,HHLL
    {203, 0, 0}, // 0xCB: Undefined
    {204, 3, 17}, // 0xCC: CALL Z,HHLL
    {205, 3, 17}, // 0xCD: CALL HHLL
    {206, 2, 7}, // 0xCE: ADC A,NN
    {207, 1, 11}, // 0xCF: RST 08
    {208, 1, 11}, // 0xD0: RET NC
    {209, 1, 10}, // 0xD1: POP DE
    {210, 3, 10}, // 0xD2: JP NC,HHLL
    {211, 2, 11}, // 0xD3: OUT (NN),A
    {212, 3, 17}, // 0xD4: CALL NC,HHLL
    {213, 1, 10}, // 0xD5: PUSH DE
    {214, 2, 7}, // 0xD6: SUB A,NN
    {215, 1, 11}, // 0xD7: RST 10
    {216, 1, 11}, // 0xD8: RET C
    {217, 1, 4}, // 0xD9: EXX
    {218, 3, 10}, // 0xDA: JP C,HHLL
    {219, 2, 11}, // 0xDB: IN A,(NN)
    {220, 3, 17}, // 0xDC: CALL C,HHLL
    {221, 0, 0}, // 0xDD: Undefined
    {222, 2, 7}, // 0xDE: SBC A,NN
    {223, 1, 11}, // 0xDF: RST 18
    {224, 1, 11}, // 0xE0: RET PO
    {225, 1, 10}, // 0xE1: POP HL
    {226, 3, 10}, // 0xE2: JP PO,HHLL
    {227, 1, 19}, // 0xE3: EX (SP),HL
    {228, 3, 17}, // 0xE4: CALL PO,HHLL
    {229, 1, 10}, // 0xE5: PUSH HL
    {230, 2, 7}, // 0xE6: AND NN
    {231, 1, 11}, // 0xE7: RST 20
    {232, 1, 11}, // 0xE8: RET PE
    {233, 1, 4}, // 0xE9: JP (HL)
    {234, 3, 10}, // 0xEA: JP PE,HHLL
    {235, 1, 4}, // 0xEB: EX DE,HL
    {236, 3, 17}, // 0xEC: CALL P,HHLL
    {237, 0, 0}, // 0xED: Undefined
    {238, 2, 7}, // 0xEE: XOR NN
    {239, 1, 11}, // 0xEF: RST 28
    {240, 1, 11}, // 0xF0: RET P
    {241, 1, 10}, // 0xF1: POP AF
    {242, 3, 10}, // 0xF2: JP P,HHLL
    {243, 1, 4}, // 0xF3: DI
    {244, 3, 17}, // 0xF4: CALL P,HHLL
    {245, 1, 10}, // 0xF5: PUSH AF
    {246, 2, 7}, // 0xF6: OR NN
    {247, 1, 11}, // 0xF7: RST 30
    {248, 1, 11}, // 0xF8: RET M
    {249, 1, 10}, // 0xF9: LD SP,HL
    {250, 3, 10}, // 0xFA: JP M,HHLL
    {251, 1, 4}, // 0xFB: EI
    {252, 3, 17}, // 0xFC: CALL M,HHLL
    {253, 0, 0}, // 0xFD: Undefined
    {254, 2, 7}, // 0xFE: CP NN
    {255, 1, 11}, // 0xFF: RST 38
};

// TODO: Modify to adapt Z80 (256 instructions!)
void (*instr_call_table[])(int) = {
	&op_nop, &op_add, &op_ld, &op_st, &op_jsr, &op_and, &op_ldr, &op_str, 
	&op_rti, &op_not, &op_ldi, &op_sti, &op_jmp, &op_res, &op_lea, &op_trap
};

/*
    Helper function declarations
*/

/* 
    The Pac-Man Z80 runs at 3.072 MHZ ->
    3,072,000 cycles per second ->
    3,072 cycles per milisecond

    So my plan is to adjust the speed every 3,072 cycles (roughly 1,000 instructions),
    if the elapsed time <= 1 milisecond, i.e. 1,000 microsecond, 
    we wait until it hits 1,000 microsecond
*/

#define CYCLE_ADJUST 3072
#define WAIT_TIME 1000

void
run_op(uint8_t* memory, uint32_t address)
{
    int cycleAccumulated = 0;
    long startTime = 0;     // TODO: current time in microseconds

    while (true)
    {
        cycleAccumulated += execute_op(memory);
        if (cycleAccumulated >= CYCLE_ADJUST)
        {
            cycleAccumulated = 0;
            startTime = wait(startTime);
        }
    }
}

/*
    wait() wait until 1,000 microseconds have passed and return current time in microseconds
*/

long 
wait(long startTime)
{
    /*
        Pseudo code:
        endTime = now;
        while (endTime - startTime <= WAIT_TIME) 
        {
            endTime = now;
        }
        return endTime;
    */

    // TODO: Implement this function based on the pseudo code
    return 0;
}

int
execute_op(uint8_t* memory)
{
    /*
        Read memory from address, use R_PC to read one operation, execute it, return number of cycles spent.
        Instruction functions should increment R_PC accordingly.

        Pseudo code:

        firstByte = memory[R_PC];
        switch (firstByte)
        {        
            case (0xCB):
            {
                op = memory[R_PC + 1];
                instr_call_table_cb(op);
                break;
            }
            case (0xDD):
            {
                op = memory[R_PC + 1];
                instr_call_table_dd(op);
                break;
            }
            case (0xED):
            {
                op = memory[R_PC + 1];
                instr_call_table_ed(op);
                break;
            }
            case (0xFD):
            {
                op = memory[R_PC + 1];
                instr_call_table_fd(op);
                break;
            }
            default:
            {
                instr_call_table(firstByte);
                break;
            }
        }

        return (z80Instr[op]).cycles
    */

    // TODO: Implement this function based on the pseudo code
    return 0;
}

void
op_nop0x00()
{
    /*
        Increment PC by 1
    */
    
    R_PC += (z80Instr[0].instrSize);
}

void
op_ld0x01()
{
    /*
        ld bc, nn: bc <- nn
        First byte is 0x01 (op)
        Read the 2nd byte into B and the 3nd byte into C
    */

    R_PC += (z80Instr[1].instrSize);
}

void
op_ld0x02()
{
    /*
        ld (bc), a: (bc) <- a
        Load a into memory address 16-bit combo bc points to
        memory[bc] = a
    */
}