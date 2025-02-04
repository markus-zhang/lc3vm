#pragma once

/*
    A loading program that loads the binary into memory.
    The caller object should prepare the memory and passes it here
*/

#include "globals.h"
#include <string>
#include <iostream>

/*
    I'm trying to generalize the format but it could be in vain
    - header: how many bytes does the header take (for LC-3 just 2 bytes)
    - swapEndian: should we swap endian if host is x64? (for LC-3 it's yes)
    - instrSize: how many bytes does one instruction take? (for LC-3 it's 2 bytes)
*/

// TODO: Generalization failed. I don't have enough experience to generalize emulators.
// Need to untangle the code and keep it as simple as possible -> remove dataFormat
// Also, maybe use a class instead of straight C code

#define MAX_SIZE 2 << 16

struct dataFormat
{
    size_t header;
    bool swapEndian;
    size_t instrSize;
};

/* 
    load the binary at filePath into memory, returns number of bytes read.
    If endian is different from host architecture, set swapEndian to true (e.g. LC-3 to Intel x64)
*/
uint64_t load_memory(uint16_t* buffer, uint16_t* memory, std::string filePath)
{
    FILE* fp = fopen(filePath.c_str(), "rb");
    if (!fp)
    {
        std::cerr << "Failed to read file in " << filePath << std::endl;
        exit(ERROR_LOADFILE);
    }

    uint16_t org = 0;
    lc3_loader_header(&org, 1, fp);

    org = swap16(org);

    // We read the rest into memory
    int size = fread(buffer, 2, MAX_SIZE, fp);
    printf("Number of instructions: %d\n", size);

    if (size <= 0)
    {
        std::cerr << "Failed to read binary body" << std::endl;
        exit(ERROR_LOAD_FILE_HEADER);
    }
    
    for (int i = 0; i < size; i++)
	{
		write_memory(memory, org + i, swap16(buffer[i]));
	}

    // For host to write into R_PC
    return org;
}

uint16_t swap16(uint16_t value)
{
	// For translating endianness
	uint16_t result = ((value >> 8) & 0x00FF) + ((value << 8) & 0xFF00);
	return result;
}

void lc3_loader_header(uint16_t* org, size_t byteToRead, FILE* fp)
{
    fread(org, 2, byteToRead, fp);
}

void write_memory(uint16_t* memory, uint16_t index, uint16_t value)
{
	memory[index] = value;
}