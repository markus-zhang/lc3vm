#include "lc3vmcache.hpp"
#include "lc3disa.hpp"
#include "lc3ui.hpp"
#include <iostream>
#include <SDL2/SDL.h>
#include <cstdio>
#include <cstdint>
#include <csignal>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/termios.h>
#include <sys/mman.h>
#include <stdarg.h>

// #define DEBUG 0			 // For debugging	

#define MAX_SIZE 65536   // 64K bytes - a maximum of 65,536 instructions if every one is of 1 byte

enum {
	DEBUG_OFF = 0,
	DEBUG_ON,
	DEBUG_DIS
};

uint8_t DEBUG = DEBUG_OFF;

/* Function declarations BEGIN --------------------------------*/
// lc-3 instruction functions
void op_br(uint16_t instr);
void op_add(uint16_t instr);
void op_ld(uint16_t instr);
void op_st(uint16_t instr);
void op_jsr(uint16_t instr);
void op_and(uint16_t instr);
void op_ldr(uint16_t instr);
void op_str(uint16_t instr);
void op_rti(uint16_t instr);
void op_not(uint16_t instr);
void op_ldi(uint16_t instr);
void op_sti(uint16_t instr);
void op_jmp(uint16_t instr);
void op_res(uint16_t instr);
void op_lea(uint16_t instr);
void op_trap(uint16_t instr);

// misc. functions
void read_image(FILE* fp, uint16_t* arrayBase, uint16_t* arraySize);
void read_image_file(FILE* file);
void setup();
void shutdown();
void handle_interrupt(int signal);
void disable_input_buffering();
void restore_input_buffering();
// uint16_t sign_extended(uint16_t num, uint8_t effBits);
void update_flag(uint16_t value);
uint16_t read_memory(uint16_t index);
void write_memory(uint16_t index, uint16_t value);
uint16_t swap16(uint16_t value);
uint16_t check_key();

// trap functions
void trap_0x20();
void trap_0x21();
void trap_0x22();
void trap_0x23();
void trap_0x24();
void trap_0x25();

// interpreter run function
void interpreter_run();
void cache_run(struct lc3Cache cache);
void cache_dump(int cacheIndex, FILE* dump);

// print everywhere function
void printToCoordinates(int x, int y, const char *format, ...);

/* Function declarations END ----------------------------------*/

/* Global variables BEGIN -------------------------------------*/
// LC-3 specific BEGIN ------------------------------------------
enum
{
	R_R0 = 0, R_R1, R_R2, R_R3, R_R4, R_R5, R_R6, R_R7,
	R_PC, 
	R_COND, 
	R_COUNT
};

// Condition codes are supposed to be in R[COND]'s bit-0/1/2, to be used in BR
enum
{
	FL_POS = 1 << 0,	// P
	FL_ZRO = 1 << 1,	// Z
	FL_NEG = 1 << 2		// N
};

enum
{
    MR_KBSR = 0xFE00, /* keyboard status */
    MR_KBDR = 0xFE02  /* keyboard data */
};

// Registers
uint16_t reg[R_COUNT];

// RAM
uint16_t memory[MAX_SIZE];

// LC-3 specific END --------------------------------------------

uint16_t* binary = NULL;
uint8_t running = 1;
struct termios original_tio;

void (*instr_call_table[])(uint16_t) = {
	&op_br, &op_add, &op_ld, &op_st, &op_jsr, &op_and, &op_ldr, &op_str, 
	&op_rti, &op_not, &op_ldi, &op_sti, &op_jmp, &op_res, &op_lea, &op_trap
};

void (*disa_call_table[])(uint16_t, uint16_t) = {
	&dis_br, &dis_add, &dis_ld, &dis_st, &dis_jsr, &dis_and, &dis_ldr, &dis_str, 
	&dis_rti, &dis_not, &dis_ldi, &dis_sti, &dis_jmp, &dis_rsv, &dis_lea, &dis_trap
};


int main()
{
	reg[R_COND] = FL_ZRO;

	reg[R_PC] = 0x3000;

    binary = (uint16_t*)malloc(MAX_SIZE * sizeof(uint16_t));
    FILE* fp = fopen("./2048.obj", "rb");
	uint16_t numInstr = 0;

	read_image(fp, binary, &numInstr);
	fclose(fp);

	/* Start of interpreter loop */
	setup();

	interpreter_run();

	shutdown();
    return 0;
}

void read_image(FILE* fp, uint16_t* arrayBase, uint16_t* arraySize)
{
	// LC-3 image is big endian while mine is little
	// So we need to swap the bytes
	// Also the first byte is where the code should be loaded into

	if (!fp)
	{
		printf("File not read!\n");
		exit(1);
	}

	// First 2-byte is origin and we should NOT read
	// it into memory
	uint16_t org = 0;
	fread(&org, 2, 1, fp);
	org = swap16(org);

	// Then we read the rest into memory
    *arraySize = fread(arrayBase, 2, MAX_SIZE, fp);
	// uint16_t org = swap16(arrayBase[0]);
	printf("Number of instructions: %d\n", *arraySize);
	for (int i = 0; i < *arraySize; i++)
	{
		write_memory(org + i, swap16(arrayBase[i]));
	}
	// 0x3000 contains the first "instruction" -- 0x3000 itself
	// the proper way is to do 2 fread(), the first fread() gets the org
	// and then the second fread() gets the code, and PC = 0x3000 instead of 0x3001
	reg[R_PC] = org;
}

void op_br(uint16_t instr)
{
	/* 
		15 14 13 12 | 11 10 9 | 8 7 6 5 4 3 2 1 0
		0  0  0  0  | n  z  p |    PCOffset9
	*/
	uint16_t pcoffset9 = sign_extended(instr & 0x01FF, 9);
	// If at least one of the nzp bits and the matching bits in R_COND are both 1, then jump
	if (reg[R_COND] & ((instr >> 9) & 0x0007))
	{
		reg[R_PC] += pcoffset9;
	}

	// if (DEBUG == DEBUG_DIS)
	// {
	// 	printf("BR");

	// 	uint8_t n = (instr >> 11) & 0x0001;
	// 	uint8_t z = (instr >> 10) & 0x0001;
	// 	uint8_t p = (instr >> 9) & 0x0001;

	// 	if (n) {putchar('n');}
	// 	if (z) {putchar('z');}
	// 	if (p) {putchar('p');}

	// 	putchar('\t');
	// 	printf("%#06x\n", (instr & 0x01FF));
	// }
}

void op_add(uint16_t instr)
{
	/* 
		15 14 13 12 | 11 10 9 | 8 7 6 | 5 | 4 3 | 2 1 0
		0  0  0  1  |   DR    |  SR1  | 0 | 0 0 |  SR2 
		----------------------or-----------------------
		15 14 13 12 | 11 10 9 | 8 7 6 | 5 | 4 3 2 1 0
		0  0  0  1  |   DR    |  SR   | 1 |    IMM
	*/
	// if (DEBUG == DEBUG_DIS)
	// {
	// 	printf("ADD\t");
	// }

	uint8_t dr = (instr >> 9) & 0x0007;
	uint8_t sr = (instr >> 6) & 0x0007;
	uint8_t mode = (instr >> 5) & 0x0001;
	if (mode)
	{
		uint16_t imm = sign_extended(instr & 0x001F, 5);
		reg[dr] = reg[sr] + imm;
	}
	else 
	{
		uint8_t sr2 = instr & 0x0007;
		reg[dr] = reg[sr] + reg[sr2];
	}
	update_flag(reg[dr]);
}

void op_ld(uint16_t instr)
{
	/* 
		15 14 13 12 | 11 10 9 | 8 7 6 5 4 3 2 1 0
		0  0  1  0  |   DR    |    PCOffset9
	*/
	uint16_t pcoffset9 = sign_extended(instr & 0x01FF, 9);
	uint8_t dr = (instr >> 9) & 0x0007;
	// Ignore privilege bit and other security measures
	reg[dr] = read_memory(reg[R_PC] + pcoffset9);
	update_flag(reg[dr]);
}

void op_st(uint16_t instr)
{
	/* 
		15 14 13 12 | 11 10 9 | 8 7 6 5 4 3 2 1 0
		0  0  1  1  |   SR    |    PCOffset9
	*/
	uint16_t pcoffset9 = sign_extended(instr & 0x01FF, 9);
	uint8_t sr = (instr >> 9) & 0x0007;
	write_memory(reg[R_PC] + pcoffset9, reg[sr]);
}

void op_jsr(uint16_t instr)
{
	/* 
		15 14 13 12 | 11 | 10 9 8 7 6 5 4 3 2 1 0
		0  1  0  0  | 1  |      PCOffset11
		-----------------or----------------------
		15 14 13 12 | 11 | 10 9 | 8 7 6 | 5 4 3 2 1 0
		0  1  0  0  | 0  | 0  0 |   BR  | 0 0 0 0 0 0
	*/
	reg[R_R7] = reg[R_PC];
	uint8_t mode = (instr >> 11) & 0x0001;
	if (mode)
	{
		uint16_t pcoffset11 = sign_extended(instr & 0x07FF, 11);
		reg[R_PC] += pcoffset11;
	}
	else
	{
		uint8_t br = (instr >> 6) & 0x0007;
		reg[R_PC] = reg[br];
	}
}

void op_and(uint16_t instr)
{
	/* 
		15 14 13 12 | 11 10 9 | 8 7 6 | 5 | 4 3 | 2 1 0
		0  1  0  1  |    DR   |  SR1  | 0 | 0 0 |  SR2
		---------------------or------------------------
		15 14 13 12 | 11 10 9 | 8 7 6 | 5 | 4 3 2 1 0
		0  1  0  1  |    DR   |  SR1  | 1 |   imm5
	*/
	uint8_t mode = (instr >> 5) & 0x0001;
	uint8_t dr = (instr >> 9) & 0x0007;
	uint8_t sr = (instr >> 6) & 0x0007;
	if (mode)
	{
		uint16_t imm5 = sign_extended(instr & 0x001F, 5);
		reg[dr] = reg[sr] & imm5;
	}
	else
	{
		uint8_t sr2 = instr & 0x0007;
		reg[dr] = reg[sr] & reg[sr2];
	}
	update_flag(reg[dr]);
}

void op_ldr(uint16_t instr)
{
	/* 
		15 14 13 12 | 11 10 9 | 8 7 6 | 5 4 3 2 1 0
		0  1  1  0  |   DR    | BaseR |   offset6
	*/
	// Again ignore the security measures
	uint8_t dr = (instr >> 9) & 0x0007;
	uint8_t br = (instr >> 6) & 0x0007;
	uint16_t offset6 = sign_extended(instr & 0x003F, 6);

	reg[dr] = read_memory(reg[br] + offset6);
	update_flag(reg[dr]);
}

void op_str(uint16_t instr)
{
	/* 
		15 14 13 12 | 11 10 9 | 8 7 6 | 5 4 3 2 1 0
		0  1  1  1  |   SR    | BaseR |   offset6
	*/
	// Again ignore the security measures
	uint8_t sr = (instr >> 9) & 0x0007;
	uint8_t br = (instr >> 6) & 0x0007;
	uint16_t offset6 = sign_extended(instr & 0x003F, 6);

	write_memory(reg[br] + offset6, reg[sr]);
}

void op_rti(uint16_t instr)
{
	/* 
		15 14 13 12 | 11 10 9 8 7 6 5 4 3 2 1 0
		1  0  0  0  | 0  0  0 0 0 0 0 0 0 0 0 0
	*/
	// Technically need to work under privilege mode
	
	/*
		PC = mem[R6]; R6 is the SSP, PC is restored
		R6 = R6+1;
		TEMP = mem[R6];
		R6 = R6+1; system stack completes POP before saved PSR is restored
		PSR = TEMP; PSR is restored
		if (PSR[15] == 1)
		Saved SSP=R6 and R6=Saved USP;
	*/
	printf("Not supposed to be here!\n");
}

void op_not(uint16_t instr)
{
	/* 
		15 14 13 12 | 11 10 9 | 8 7 6 | 5 | 4 3 2 1 0
		1  0  0  1  |   DR    |   SR  | 1 | 1 1 1 1 1
	*/
	uint8_t dr = (instr >> 9) & 0x0007;
	uint8_t sr = (instr >> 6) & 0x0007;

	reg[dr] = (~reg[sr]);
	update_flag(reg[dr]);
}

void op_ldi(uint16_t instr)
{
	/* 
		15 14 13 12 | 11 10 9 | 8 7 6 5 4 3 2 1 0
		1  0  1  0  |   DR    |    PCoffset9
	*/
	// Again we ignore the security measures
	uint16_t pcoffset9 = sign_extended(instr & 0x01FF, 9);
	uint8_t dr = (instr >> 9) & 0x0007;

	reg[dr] = read_memory(read_memory(reg[R_PC] + pcoffset9));
	update_flag(reg[dr]);
}

void op_sti(uint16_t instr)
{
	/* 
		15 14 13 12 | 11 10 9 | 8 7 6 5 4 3 2 1 0
		1  0  1  1  |   SR    |     PCoffset9
	*/
	// Again ignore the security measures
	uint8_t sr = (instr >> 9) & 0x0007;
	uint16_t pcoffset9 = sign_extended(instr & 0x01FF, 9);

	write_memory(read_memory(reg[R_PC] + pcoffset9), reg[sr]);
}

void op_jmp(uint16_t instr)
{
	/*  JMP
		15 14 13 12 | 11 10 9 | 8 7 6 | 5 4 3 2 1 0
		1  1  0  0  | 0  0  0 | BaseR | 0 0 0 0 0 0
		-------------------or----------------------
		RET
		15 14 13 12 | 11 10 9 | 8 7 6 | 5 4 3 2 1 0
		1  1  0  0  | 0  0  0 | 1 1 1 | 0 0 0 0 0 0
	*/
	uint8_t br = (instr >> 6) & 0x0007;
	// return address stored in R7 so a "jmp" to it equals RET
	reg[R_PC] = reg[br];
}

void op_res(uint16_t instr)
{
	/*  RET
		15 14 13 12 | 11 10 9 | 8 7 6 | 5 4 3 2 1 0
		1  1  0  0  | 0  0  0 | 1 1 1 | 0 0 0 0 0 0
	*/
	// reg[R_PC] = reg[R_R7];
	printf("Not supposed to be here\n");
}

void op_lea(uint16_t instr)
{
	/*
		15 14 13 12 | 11 10 9 | 8 7 6 5 4 3 2 1 0
		1  1  1  0  |    dr   |     PCoffset9
	*/
	uint16_t pcoffset9 = sign_extended(instr & 0x01FF, 9);
	uint8_t dr = (instr >> 9) & 0x0007;

	reg[dr] = reg[R_PC] + pcoffset9;
	update_flag(reg[dr]);
}

void op_trap(uint16_t instr)
{
	/*
		15 14 13 12 | 11 10 9 8 | 7 6 5 4 3 2 1 0
		1  1  1  1  | 0  0  0 0 |    trapvect8
	*/
	reg[R_R7] = reg[R_PC];

	uint8_t trapvect8 = instr & 0x00FF;
	switch (trapvect8)
	{
		case 0x20:
			// GETC
			// Read a single character from the keyboard. The character is not echoed onto the console.
			// Its ASCII code is copied into R0. The high eight bits of R0 are cleared
			trap_0x20();
			break;
		case 0x21:
			trap_0x21();
			break;
		case 0x22:
			trap_0x22();
			break;
		case 0x23:
			trap_0x23();
			break;
		case 0x24:
			trap_0x24();
			break;
		case 0x25:
			trap_0x25();
			break;
		default:
			printf("Erroneous TRAP vector!\n");
	}
}

// misc. functions
void setup()
{
	signal(SIGINT, handle_interrupt);
	signal(SIGTERM, handle_interrupt);
	signal(SIGSEGV, handle_interrupt);
	signal(SIGKILL, handle_interrupt);
	disable_input_buffering();
}

void shutdown()
{
	restore_input_buffering();
	cache_clear();
}

void handle_interrupt(int signal)
{
	restore_input_buffering();
	printf("\n");
	exit(-2);
}

void disable_input_buffering()
{
    tcgetattr(STDIN_FILENO, &original_tio);
    struct termios new_tio = original_tio;
    new_tio.c_lflag &= ~ICANON & ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
	
}

void restore_input_buffering()
{
    tcsetattr(STDIN_FILENO, TCSANOW, &original_tio);
}

// static uint16_t sign_extended(uint16_t num, uint8_t effBits)
// {
// 	// Sign extend num that contains effBits of bits to a full 16-bit unsigned short
// 	// uint16_t is good even for negative numbers because of overflow ->
// 	// consider 0x3000 + 0xFFFF in 16-bit, this results in 0x2FFF which is what we want

// 	// check whether the top effective bit is 1
// 	if ((num >> (effBits - 1)) & 0x0001)
// 	{
// 		// e.g. 0x003F with 6 effective bits would be a negative number,
// 		// we left shift 0xFFFF to make the last 6 bits 0 so the 3F part doesn't get impacted
// 		// then sign extend the rest as 1, results in 0xFFFF
// 		// If 0x003F has 7 effective bits, then it's a positive number and nothing needs to be done
// 		return (num | (0xFFFF << effBits));
// 	}
// 	else
// 	{
// 		return num;
// 	}
// }

void update_flag(uint16_t value)
{
	// Clear the last three bits (N/Z/P) and set P
	reg[R_COND] &= 0xFFF8;
	if (value >> 15)
	{	
		// Since value is uint16_t, cannot use if (value < 0), have to check the highest bit
		reg[R_COND] |= FL_NEG;
	}
	else if (value == 0)
	{
		reg[R_COND] |= FL_ZRO;
	}
	else
	{
		reg[R_COND] |= FL_POS;
	}
}

uint16_t read_memory(uint16_t index)
{
	// Two memory mapped registers
	if (index == MR_KBSR)
    {
        if (check_key())
        {
            memory[MR_KBSR] = (1 << 15);
            memory[MR_KBDR] = getchar();
        }
        else
        {
            memory[MR_KBSR] = 0;
        }
    }
	return memory[index];
}

void write_memory(uint16_t index, uint16_t value)
{
	memory[index] = value;
}

// trap functions
void trap_0x20()
{
	// Read a single character from the keyboard. The character is not echoed onto the console.
	// Its ASCII code is copied into R0. The high eight bits of R0 are cleared
	reg[R_R0] = (uint16_t)getchar();
	reg[R_R0] &= 0x00FF;
	update_flag(reg[R_R0]);
	ui_debug_info(reg, 25);
	fflush(stdout);
}

void trap_0x21()
{
	// Write a character in R0[7:0] to the console display.
	putc((uint8_t)reg[R_R0], stdout);
	ui_debug_info(reg, 25);
	fflush(stdout);
}

void trap_0x22()
{
	// Write a string of ASCII characters to the console display. The characters are
	// contained in consecutive memory locations, one character per memory location,
	// starting with the address specified in R0. Writing terminates with the occurrence of
	// x0000 in a memory location.

	for (uint16_t i = reg[R_R0]; ;i++)
	{
		char ch = read_memory(i);
		if (ch == 0)
		{
			break;
		}
		else
		{
			putc(ch, stdout);
		}
	}
	ui_debug_info(reg, 25);
	fflush(stdout);
}

void trap_0x23()
{
	// Print a prompt on the screen and read a single character from the keyboard. 
	// The character is echoed onto the console monitor, and its ASCII code is copied into R0.
	// The high eight bits of R0 are cleared.
	
	printf("> ");
	reg[R_R0] = (uint16_t)fgetc(stdin);
	reg[R_R0] &= 0x00FF;
	putc((uint8_t)reg[R_R0], stdout);
	ui_debug_info(reg, 25);
	fflush(stdout);
	update_flag(reg[R_R0]);
}

void trap_0x24()
{
	/*
		Write a string of ASCII characters to the console. 
		The characters are contained in consecutive memory locations, 
		two characters per memory location, starting with the address specified in R0. 

		The ASCII code contained in bits [7:0] of a memory
		location is written to the console first. 
		
		Then the ASCII code contained in bits [15:8] of that memory location is written to the console. 
		
		(A character string consisting of
		an odd number of characters to be written will have x00 in bits [15:8] of the
		memory location containing the last character to be written.) Writing terminates
		with the occurrence of x0000 in a memory location.
	*/
	for (uint16_t i = reg[R_R0]; ;i++)
	{
		uint16_t value = read_memory(i);
		if (value == 0)
		{
			break;
		}
		else
		{
			putc((uint8_t)(value & 0x00FF), stdout);
			putc(((uint8_t)(value >> 8)), stdout);
			ui_debug_info(reg, 25);
		}
	}
	fflush(stdout);
}

void trap_0x25()
{
	// Halt execution and print a message on the console.
	printf("\nSystem HALT\n");
	running = 0;
}

void interpreter_run()
{
	while (running)
	{
		uint16_t lc3Address = reg[R_PC];
		int cacheIndex = cache_find(lc3Address);

		// if cache not found, then build and insert
		if (cacheIndex == -1)
		{
			struct lc3Cache newCache = cache_create_block(memory, lc3Address);
			int newCacheIndex = cacheCount;
			cache_add(newCache);

			if (DEBUG == DEBUG_DIS)
			{
				FILE* dump = fopen("cache_dump.txt", "a");
				cache_dump(newCacheIndex, dump);
			}

			cache_run(codeCache[newCacheIndex]);
		}
		// if found, then execute
		else
		{
			cache_run(codeCache[cacheIndex]);
		}
	}
}

void cache_run(struct lc3Cache cache)
{
	/*
		cache_run is different from interpreter_run in the sense
			-> that we don't use PC to find the next instruction but just run sequentially inside of the cache
			-> We still need to update the PC for the next interpreter_run() call
	*/
	for (int i = 0; i < cache.numInstr; i++)
	{
		uint16_t instr = cache.codeBlock[i];	
		uint16_t op = instr >> 12;

		// Debuggin BEGIN
		ui_debug_info(reg, 25);
		// Debugging END

		/* LC-3 PC++ */
		reg[R_PC] += 1;	
		instr_call_table[op](instr);
	}

}

void cache_dump(int cacheIndex, FILE* dump)
{
	if (!dump)
	{
		printf("DEBUG LEVEL SET TO DIS BUT NO DUMP FILE LOCATED. STOP\n");
		return;
	}

	fprintf(dump, "--------Dumping Cache No. %d BEGIN--------\n", cacheIndex);
	if (cacheIndex >= cacheCount)
	{
		fprintf(dump, "Wrong cache index at: %d\n", cacheIndex);
	}
	else
	{
		for (int i = 0; i < codeCache[cacheIndex].numInstr; i++)
		{
			uint16_t instr = codeCache[cacheIndex].codeBlock[i];	
			uint16_t op = instr >> 12;
			/* Call the dispatch fp */
			disa_call_table[op](instr, 0x3000 + i * 2);
		}
	}
	printf("--------Dumping Cache No. %d END----------\n", cacheIndex);
	// Pause for inspection
	// fflush(stdout);
	// getchar();
}

uint16_t swap16(uint16_t value)
{
	// For translating endianness
	uint16_t result = ((value >> 8) & 0x00FF) + ((value << 8) & 0xFF00);
	return result;
}

uint16_t check_key()
{
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    return select(1, &readfds, NULL, NULL, &timeout) != 0;
}

// https://stackoverflow.com/questions/1670891/how-can-i-print-a-string-to-the-console-at-specific-coordinates-in-c
void printToCoordinates(int x, int y, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    printf("\033[%d;%dH", x, y);
    vprintf(format, args);
    va_end(args);
    fflush(stdout);
}