#include "lc3vmwin_loader.hpp"

uint16_t load_memory(uint16_t buffer[], uint8_t memory[], FILE* fp)
{
    uint16_t org = 0;
    lc3_loader_header(&org, 1, fp);

    org = swap16(org);
    printf("org is: %hu", org);

    // We read the rest into memory
    size_t size = fread(buffer, 2, MAX_SIZE, fp);
    printf("Number of instructions: %d\n", size);

    if (size <= 0)
    {
        std::cerr << "Failed to read binary body" << std::endl;
        exit(ERROR_LOAD_FILE_HEADER);
    }
    
    for (int i = 0; i < size; i++)
	{
        /* memory should load from org (usually 0x3000) */
        uint16_t swapped = swap16(buffer[i]);
        memory[org + i * 2] = (uint8_t)(swapped >> 8);
        memory[org + (i * 2) + 1] = (uint8_t)(swapped & 0x00FF);
	}

    // For host to write into R_PC
    return org;
}

uint16_t swap16(uint16_t value)
{
	// For translating endianness
	uint16_t result = (uint16_t)((value >> 8) & 0x00FF) + (uint16_t)((value << 8) & 0xFF00);
	return result;
}

void lc3_loader_header(uint16_t* org, size_t byteToRead, FILE* fp)
{
    fread(org, 2, byteToRead, fp);
}