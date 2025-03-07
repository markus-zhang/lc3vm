/* 
	Code cache - an array of struct cache
*/

#include "lc3disa.hpp"
#include "lc3vmwin_cache.hpp"
#include <iostream>

uint16_t cacheCount = 0;
struct lc3Cache codeCache[CACHE_SIZE_MAX];

struct lc3Cache cache_create_block(uint16_t memory[], uint16_t lc3Address)
{
	uint16_t lc3MemAddress = lc3Address;
	uint16_t* codeBlock = new uint16_t[CODE_BLOCK_SIZE];
	if (!codeBlock)
	{
		printf("%s at line %d: codeBlock created failure\n", __FILE__, __LINE__);
		std::cout << __FILE__ << "at line " << __LINE__ << ": codeBlock created failure" << std::endl;
	}
	uint16_t numInstr = 0;

	while (true)
	{
		write_16bit(codeBlock, numInstr, memory[lc3Address]);
		numInstr++;
		/*
			EXPLAIN: 
			Find the last lc3Address that is a jump/ret/trap. 
			Code blocks always stop at such instructions.
		*/
		if (is_branch(get_opcode(memory[lc3Address])))
		{
			break;
		}
		/* 
			EXPLAIN: 
			Each memory[i] is 16-bit so it is enough to just increment 1, not 2,
			to fetch the next instruction.
		*/
		lc3Address += 1;
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
	/* 
		EXPLAIN:
		What if we already have CACHE_SIZE_MAX blocks? 
		Then we simply recycle the last chunk again and again.
		Bad luck then.
	*/
	else
	{
		codeCache[CACHE_SIZE_MAX - 1] = c;
	}
}


// int cache_find(uint16_t address)
// {
// 	for (uint16_t i = 0; i < cacheCount; i++)
// 	{
// 		if (codeCache[i].lc3MemAddress == address)
// 		{
// 			return i;
// 		}
// 	}
// 	return -1;
// }

struct codeLocation cache_find(uint16_t address)
{
	for (uint16_t i = 0; i < cacheCount; i++)
	{
		if (address_in_block(address, i))
		{
			return {i, address - codeCache[i].lc3MemAddress};
		}
	}
	return {-1, -1};
}


/* Utility functions */

bool address_in_block(uint16_t address, uint16_t cacheIndex)
{
	return (
		(address >= codeCache[cacheIndex].lc3MemAddress) && 
		(address <= codeCache[cacheIndex].lc3MemAddress + codeCache[cacheIndex].numInstr - 1)
	);
}

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

