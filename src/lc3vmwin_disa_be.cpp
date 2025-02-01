/*
    Serves the backend for the disassembler, outputing strings for each insturction
*/

#include "lc3vmwin_disa_be.hpp"
#include <sstream>
#include <iomanip>

std::string dis_br(uint16_t instr, uint16_t address)
{
	/* 
		15 14 13 12 | 11 10 9 | 8 7 6 5 4 3 2 1 0
		0  0  0  0  | n  z  p |    PCOffset9
	*/

    std::stringstream ss;

	// dis_debug(instr, address);

	ss << "BR";

	uint8_t n = (instr >> 11) & 0x0001;
	uint8_t z = (instr >> 10) & 0x0001;
	uint8_t p = (instr >> 9) & 0x0001;

	if (n) {ss << "n\t";}
	if (z) {ss << "z\t";}
	if (p) {ss << "p\t";}

    ss << "0x" << std::hex << std::setfill('0') << std::setw(4) << static_cast<int>(sign_extended(instr & 0x01FF, 9));

    return ss.str();
}

uint16_t sign_extended(uint16_t num, uint8_t effBits)
{
	// Sign extend num that contains effBits of bits to a full 16-bit unsigned short
	// uint16_t is good even for negative numbers because of overflow ->
	// consider 0x3000 + 0xFFFF in 16-bit, this results in 0x2FFF which is what we want

	// check whether the top effective bit is 1
	if ((num >> (effBits - 1)) & 0x0001)
	{
		// e.g. 0x003F with 6 effective bits would be a negative number,
		// we left shift 0xFFFF to make the last 6 bits 0 so the 3F part doesn't get impacted
		// then sign extend the rest as 1, results in 0xFFFF
		// If 0x003F has 7 effective bits, then it's a positive number and nothing needs to be done
		return (num | (0xFFFF << effBits));
	}
	else
	{
		return num;
	}
}