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
    main_instr_enum op;
    uint8_t instrSize;
    uint8_t cycles;
};

