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

// struct dataFormat
// {
//     size_t header;
//     bool swapEndian;
//     size_t instrSize;
// };

/* 
    load the binary at filePath into memory, returns number of bytes read.
    If endian is different from host architecture, set swapEndian to true (e.g. LC-3 to Intel x64)
*/
uint64_t load_memory(uint16_t buffer[], uint8_t memory[], FILE* fp);

uint16_t swap16(uint16_t value);

void lc3_loader_header(uint16_t* org, size_t byteToRead, FILE* fp);