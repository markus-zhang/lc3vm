#pragma once

#include <cstdint>
#include <string>


#define DISA_DEBUG_LEVEL 1

void dis_debug(uint16_t instr, uint16_t address);
std::string dis_br(uint16_t instr, uint16_t address);
std::string dis_add(uint16_t instr, uint16_t address);
std::string dis_ld(uint16_t instr, uint16_t address);
std::string dis_st(uint16_t instr, uint16_t address);
std::string dis_jsr(uint16_t instr, uint16_t address);
std::string dis_and(uint16_t instr, uint16_t address);
std::string dis_ldr(uint16_t instr, uint16_t address);
std::string dis_str(uint16_t instr, uint16_t address);
std::string dis_rti(uint16_t instr, uint16_t address);
std::string dis_not(uint16_t instr, uint16_t address);
std::string dis_ldi(uint16_t instr, uint16_t address);
std::string dis_sti(uint16_t instr, uint16_t address);
std::string dis_jmp(uint16_t instr, uint16_t address);
std::string dis_rsv(uint16_t instr, uint16_t address);
std::string dis_lea(uint16_t instr, uint16_t address);
std::string dis_trap(uint16_t instr, uint16_t address);

uint16_t sign_extended(uint16_t num, uint8_t effBits);