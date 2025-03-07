#pragma once

#include <cstdint>

#define CACHE_SIZE_MAX 	1024	// I figured 1024 code blocks should be kinda enough
#define CODE_BLOCK_SIZE 256		// 256 instructions without jmp/ret/jsr/trap? No way...

struct lc3Cache
{
	uint16_t 	lc3MemAddress;
	int 		numInstr;
	uint16_t* 	codeBlock;
};

/* EXPLAIN: For cache_find(), need to return index of cache and index of code */
struct codeLocation
{
	int cacheIndex;
	int codeIndex;
};

// tracks the count of codeBlocks
extern uint16_t cacheCount;

extern struct lc3Cache codeCache[];

struct lc3Cache cache_create_block(uint16_t memory[], uint16_t lc3Address);
void cache_clear();
void cache_add(struct lc3Cache c);
// int cache_find(uint16_t address);
struct codeLocation cache_find(uint16_t address);

/* Utility functions */
uint8_t get_opcode(uint16_t instr);
int is_branch(uint8_t opcode);
void write_16bit(uint16_t* targetArray, uint16_t targetIndex, uint16_t value);
bool address_in_block(uint16_t address, uint16_t cacheIndex);