#ifndef clox_chunk_h
#define clox_chunk_h
#include "common.h"

/*
    Each instruction has a one-byte opcode;
    A "Chunk" is simply an array of instructions
*/

typedef enum
{
    OP_RETURN,
} OpCode;

typedef struct
{
    int count;
    int capacity;
    uint8_t* code;
} Chunk;

void initChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, uint8_t byte);
/* freeChunk() also re-init the chunk */
void freeChunk(Chunk* chunk);

#endif