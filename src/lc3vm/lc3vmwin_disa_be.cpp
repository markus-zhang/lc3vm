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

	dis_debug(instr, address);

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

std::string dis_add(uint16_t instr, uint16_t address)
{
	/* 
		15 14 13 12 | 11 10 9 | 8 7 6 | 5 | 4 3 | 2 1 0
		0  0  0  1  |   DR    |  SR1  | 0 | 0 0 |  SR2 
		----------------------or-----------------------
		15 14 13 12 | 11 10 9 | 8 7 6 | 5 | 4 3 2 1 0
		0  0  0  1  |   DR    |  SR   | 1 |    IMM
	*/

	std::stringstream ss;

	dis_debug(instr, address);

	ss << "ADD\t";

	int dr = (instr >> 9) & 0x0007;
	ss << 'r' << dr << '\t';
	int sr = (instr >> 6) & 0x0007;
	ss << 'r' << sr << '\t';

	uint8_t mode = (instr >> 5) & 0x0001;

	if (mode)
	{
		ss << "0x" << std::hex << std::setfill('0') << std::setw(4) << static_cast<int>(sign_extended(instr & 0x01FF, 5));
	}
	else 
	{
		int sr2 = instr & 0x0007;
		ss << 'r' << sr2;
	}

	return ss.str();
}

std::string dis_ld(uint16_t instr, uint16_t address)
{
	/* 
		15 14 13 12 | 11 10 9 | 8 7 6 5 4 3 2 1 0
		0  0  1  0  |   DR    |    PCOffset9
	*/

	std::stringstream ss;

	dis_debug(instr, address);

	ss << "LD\t";

	// Don't use uint8_t because it is unsigned char and stringstream treats it as a char, not an integer
	int dr = (instr >> 9) & 0x0007;
	ss << 'r' << dr << '\t';
	// printf("dr is: %d\n", dr);

	ss << "0x" << std::hex << std::setfill('0') << std::setw(4) << static_cast<int>(sign_extended(instr & 0x01FF, 9));
	// printf("%s\n", ss.str().c_str());

	return ss.str();
}

std::string dis_st(uint16_t instr, uint16_t address)
{
	/* 
		15 14 13 12 | 11 10 9 | 8 7 6 5 4 3 2 1 0
		0  0  1  1  |   SR    |    PCOffset9
	*/

	std::stringstream ss;

	dis_debug(instr, address);

	ss << "LD\t";

	int sr = (instr >> 9) & 0x0007;
	ss << 'r' << sr << '\t';

	ss << "0x" << std::hex << std::setfill('0') << std::setw(4) << static_cast<int>(sign_extended(instr & 0x01FF, 9));

	return ss.str();
}

std::string dis_jsr(uint16_t instr, uint16_t address)
{
	/* 
		15 14 13 12 | 11 | 10 9 8 7 6 5 4 3 2 1 0
		0  1  0  0  | 1  |      PCOffset11
		-----------------or----------------------
		15 14 13 12 | 11 | 10 9 | 8 7 6 | 5 4 3 2 1 0
		0  1  0  0  | 0  | 0  0 |   BR  | 0 0 0 0 0 0
	*/

	std::stringstream ss;

	dis_debug(instr, address);

	ss << "JSR\t";

	uint8_t mode = (instr >> 11) & 0x0001;

	if (mode)
	{
		ss << "0x" << std::hex << std::setfill('0') << std::setw(4) << static_cast<int>(sign_extended(instr & 0x07FF, 11));
	}
	else 
	{
		uint16_t br = (instr >> 6)  & 0x0007;
		ss << 'r' << br << '\t';
	}

	return ss.str();
}

std::string dis_and(uint16_t instr, uint16_t address)
{
	/* 
		15 14 13 12 | 11 10 9 | 8 7 6 | 5 | 4 3 | 2 1 0
		0  1  0  1  |    DR   |  SR1  | 0 | 0 0 |  SR2
		---------------------or------------------------
		15 14 13 12 | 11 10 9 | 8 7 6 | 5 | 4 3 2 1 0
		0  1  0  1  |    DR   |  SR1  | 1 |   imm5
	*/

	std::stringstream ss;

	dis_debug(instr, address);

	ss << "AND\t";

	int dr = (instr >> 9) & 0x0007;
	ss << 'r' << dr << '\t';
	int sr = (instr >> 6) & 0x0007;
	ss << 'r' << sr << '\t';

	uint8_t mode = (instr >> 5) & 0x0001;
	if (mode)
	{
		ss << "0x" << std::hex << std::setfill('0') << std::setw(4) << static_cast<int>(sign_extended(instr & 0x001F, 5));
	}
	else
	{
		int sr2 = instr & 0x0007;
		ss << 'r' << sr2 << '\t';
	}

	return ss.str();
}

std::string dis_ldr(uint16_t instr, uint16_t address)
{
	/* 
		15 14 13 12 | 11 10 9 | 8 7 6 | 5 4 3 2 1 0
		0  1  1  0  |   DR    | BaseR |   offset6
	*/

	std::stringstream ss;

	dis_debug(instr, address);

	ss << "LDR\t";

	int dr = (instr >> 9) & 0x0007;
	ss << 'r' << dr << '\t';
	int sr = (instr >> 6) & 0x0007;
	ss << 'r' << sr << '\t';

	ss << "0x" << std::hex << std::setfill('0') << std::setw(4) << static_cast<int>(sign_extended(instr & 0x003F, 6));

	return ss.str();
}

std::string dis_str(uint16_t instr, uint16_t address)
{
	/* 
		15 14 13 12 | 11 10 9 | 8 7 6 | 5 4 3 2 1 0
		0  1  1  1  |   SR    | BaseR |   offset6
	*/
	std::stringstream ss;

	dis_debug(instr, address);

	ss << "STR\t";

	int sr = (instr >> 9) & 0x0007;
	ss << 'r' << sr << '\t';
	int br = (instr >> 6) & 0x0007;
	ss << 'r' << br << '\t';

	ss << "0x" << std::hex << std::setfill('0') << std::setw(4) << static_cast<int>(sign_extended(instr & 0x003F, 6));

	return ss.str();
}

std::string dis_rti(uint16_t instr, uint16_t address)
{
	/* Not used */
	/* 
		15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0
		1  0  0  0  0  0  0 0 0 0 0 0 0 0 0 0
	*/
	return std::string("RTI not used");
}

std::string dis_not(uint16_t instr, uint16_t address)
{
	/* 
		15 14 13 12 | 11 10 9 | 8 7 6 | 5 | 4 3 2 1 0
		1  0  0  1  |   DR    |   SR  | 1 | 1 1 1 1 1
	*/
	std::stringstream ss;

	dis_debug(instr, address);

	ss << "NOT\t";

	int sr = (instr >> 9) & 0x0007;
	ss << 'r' << sr << '\t';
	int br = (instr >> 6) & 0x0007;
	ss << 'r' << br << '\t';

	return ss.str();
}

std::string dis_ldi(uint16_t instr, uint16_t address)
{
	/* 
		15 14 13 12 | 11 10 9 | 8 7 6 5 4 3 2 1 0
		1  0  1  0  |   DR    |    PCoffset9
	*/
	std::stringstream ss;

	dis_debug(instr, address);

	ss << "LDI\t";

	int dr = (instr >> 9) & 0x0007;
	ss << 'r' << dr << '\t';

	ss << "0x" << std::hex << std::setfill('0') << std::setw(4) << static_cast<int>(sign_extended(instr & 0x01FF, 9));

	return ss.str();
}

std::string dis_sti(uint16_t instr, uint16_t address)
{
	/* 
		15 14 13 12 | 11 10 9 | 8 7 6 5 4 3 2 1 0
		1  0  1  1  |   SR    |     PCoffset9
	*/
	std::stringstream ss;

	dis_debug(instr, address);

	ss << "STI\t";

	int sr = (instr >> 9) & 0x0007;
	ss << 'r' << sr << '\t';

	ss << "0x" << std::hex << std::setfill('0') << std::setw(4) << static_cast<int>(sign_extended(instr & 0x01FF, 9));

	return ss.str();
}

std::string dis_jmp(uint16_t instr, uint16_t address)
{
	/*  JMP
		15 14 13 12 | 11 10 9 | 8 7 6 | 5 4 3 2 1 0
		1  1  0  0  | 0  0  0 | BaseR | 0 0 0 0 0 0
		-------------------or----------------------
		RET
		15 14 13 12 | 11 10 9 | 8 7 6 | 5 4 3 2 1 0
		1  1  0  0  | 0  0  0 | 1 1 1 | 0 0 0 0 0 0
	*/
	std::stringstream ss;

	dis_debug(instr, address);

	/* This instruction is a bit special, could be RET if BaseR is R7 */
	int br = (instr >> 6) & 0x0007;

	if (br == 7)
	{
		/* RET */
		ss << "RET";
	}
	else 
	{
		/* JMP */
		ss << "RET\t";

		int sr = (instr >> 6) & 0x0007;
		ss << 'r' << sr << '\t';
	}

	return ss.str();
}

std::string dis_rsv(uint16_t instr, uint16_t address)
{
	/* Reserved */
	/* 
		15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0
		1  1  0  1  x  x  x x x x x x x x x x
	*/
	return std::string("Reserved");
}

std::string dis_lea(uint16_t instr, uint16_t address)
{
	/*
		15 14 13 12 | 11 10 9 | 8 7 6 5 4 3 2 1 0
		1  1  1  0  |    dr   |     PCoffset9
	*/
	std::stringstream ss;

	dis_debug(instr, address);

	ss << "LEA\t";

	int dr = (instr >> 9) & 0x0007;
	ss << 'r' << dr << '\t';

	ss << "0x" << std::hex << std::setfill('0') << std::setw(4) << static_cast<int>(sign_extended(instr & 0x01FF, 9));

	return ss.str();
}

std::string dis_trap(uint16_t instr, uint16_t address)
{
	/*
		15 14 13 12 | 11 10 9 8 | 7 6 5 4 3 2 1 0
		1  1  1  1  | 0  0  0 0 |    trapvect8
	*/
	std::stringstream ss;

	dis_debug(instr, address);

	uint8_t trapvect8 = (uint8_t)(instr & 0x00FF);
	switch (trapvect8)
	{
		case 0x20:
			ss << "GETC";
			break;
		case 0x21:
			ss << "OUT";
			break;
		case 0x22:
			ss << "PUTS";
			break;
		case 0x23:
			ss << "IN";
			break;
		case 0x24:
			ss << "PUTSP";
			break;
		case 0x25:
			ss << "HALT";
			break;
		default:
			ss << "Erroneous TRAP vector";
	}

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

void dis_debug(uint16_t instr, uint16_t address)
{

	if (DISA_DEBUG_LEVEL == 2)
	{
		// Show current instruction in hex and mnenomics
		// printf("Instruction to be executed - %#06x\n", instr);
		// printf("15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00\n");
		for (int i = 15; i >= 0; i--)
		{
			printf("%hhu  ", (instr >> i) & 0x01);
		}
		printf("\n");

		// First we print the address
		printf("Address: %#06x\t", (unsigned int)address);
		printf("%#06x\t", instr);
	}
}