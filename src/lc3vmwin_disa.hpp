#pragma once

#include "globals.h"
#include <imgui/imgui.h>
#include <string>
#include <vector>

/*
    In lc3disa.cpp, the disassembly occurs at the instruction level:
    One uint16_t gets passed and then the disassembled code is printed out.

    This is not very useful to us. We would prefer the code buffered. 
    We assume that we get the following information:
    - uint16_t address - the address of the first instruction (the instructions are supposed to be contiguous in memory)

    - uint16_t numInstructions - number of instructions

    - uint16_t* instructions - the instructions themselves, as an array
*/

class LC3VMdisawindow
{
public:
    uint16_t initialAddress;
    uint16_t numInstructions;
    std::vector<uint16_t> instructionStream;    // In future may use a struct to hold an instruction
    std::vector<bool> selection;

    ImVec2 initialWindowSize;
    ImVec2 minWindowSize;
    ImVec2 winPos;
    ImFont* font;

    bool initialized;

    LC3VMdisawindow() = delete;
    LC3VMdisawindow(uint16_t instrStream[], uint16_t numInstr, uint16_t address, const WindowConfig& config);
    ~LC3VMdisawindow() = default;

    void Load(uint16_t instrStream[], uint16_t numInstr, uint16_t address);
    std::string Disa(uint16_t instr);
    void Draw(void);
    
};