/* 
	Code cache - an array of struct cache
*/

#include "lc3disa.hpp"
#include "lc3vmwin_cache.hpp"
#include <iostream>

uint16_t cacheCount = 0;
struct lc3Cache codeCache[CACHE_SIZE_MAX];

struct lc3Cache cache_create_block(uint8_t memory[], uint16_t lc3Address)
{
	uint16_t lc3MemAddress = lc3Address;
	// uint16_t* codeBlock = (uint16_t*)malloc(sizeof(uint16_t) * CODE_BLOCK_SIZE);
	uint16_t* codeBlock = new uint16_t[CODE_BLOCK_SIZE];
	if (!codeBlock)
	{
		printf("%s at line %d: codeBlock created failure\n", __FILE__, __LINE__);
		std::cout << __FILE__ << "at line " << __LINE__ << ": codeBlock created failure" << std::endl;
	}
	uint16_t numInstr = 0;

	while (true)
	{
		uint16_t lowByte = (memory[lc3Address]) << 8;
		uint16_t highByte = memory[(lc3Address + 1)];
		uint16_t instr = highByte + lowByte;
		// printf("instr is %#06x\n", instr);
		// printf("opcode is %#04x\n", instr >> 12);
		write_16bit(codeBlock, numInstr, instr);
		numInstr++;
		/*
			find the last lc3Address that is a jump/ret/trap
		*/
		if (is_branch(get_opcode(instr)))
		{
			break;
		}
		/* We already read 2 bytes */
		lc3Address += 2;
	}

	struct lc3Cache cache = {lc3MemAddress, numInstr, codeBlock};

	return cache;
}

void cache_clear()
{
	for(uint16_t i = cacheCount; i > 0; i--)
	{
		delete codeCache[i - 1].codeBlock;
	}

	// cacheCount should be 0 by now
	cacheCount = 0;
}

void cache_add(struct lc3Cache c)
{
	if (cacheCount < CACHE_SIZE_MAX - 1)
	{
		codeCache[cacheCount] = c;
		cacheCount++;
	}
	// what if we already have CACHE_SIZE_MAX blocks?
	else
	{
		codeCache[CACHE_SIZE_MAX - 1] = c;
	}
}


int cache_find(uint16_t address)
{
	for (uint16_t i = 0; i < cacheCount; i++)
	{
		if (codeCache[i].lc3MemAddress == address)
		{
			return i;
		}
	}
	return -1;
}


/* Utility functions */

uint8_t get_opcode(uint16_t instr)
{
	return (uint8_t)((instr >> 12) & 0x000F);
}

/* returns 1 if it's a br/jmp/ret/jsr, 0 otherwise (trap is allowed to stay) */
int is_branch(uint8_t opcode)
{
	return ((opcode == 0x00) || (opcode == 0x04) || (opcode == 0x0c));
}

void write_16bit(uint16_t* targetArray, uint16_t targetIndex, uint16_t value)
{
	targetArray[targetIndex] = value;
}

