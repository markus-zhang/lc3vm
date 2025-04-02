#ifndef _LC3DISA_HPP_
#define _LC3DISA_HPP_

#include <cstdint>

#define DEBUG_LEVEL 1

void dis_debug(uint16_t instr, uint16_t address);
void dis_br(uint16_t instr, uint16_t address);
void dis_add(uint16_t instr, uint16_t address);
void dis_ld(uint16_t instr, uint16_t address);
void dis_st(uint16_t instr, uint16_t address);
void dis_jsr(uint16_t instr, uint16_t address);
void dis_and(uint16_t instr, uint16_t address);
void dis_ldr(uint16_t instr, uint16_t address);
void dis_str(uint16_t instr, uint16_t address);
void dis_rti(uint16_t instr, uint16_t address);
void dis_not(uint16_t instr, uint16_t address);
void dis_ldi(uint16_t instr, uint16_t address);
void dis_sti(uint16_t instr, uint16_t address);
void dis_jmp(uint16_t instr, uint16_t address);
void dis_rsv(uint16_t instr, uint16_t address);
void dis_lea(uint16_t instr, uint16_t address);
void dis_trap(uint16_t instr, uint16_t address);

uint16_t sign_extended(uint16_t num, uint8_t effBits);


#endif