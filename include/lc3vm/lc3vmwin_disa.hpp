#pragma once

#include "globals.hpp"
#include "lc3vmwin_disa_be.hpp"
#include <imgui.h>
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

// std::string (*disa_call_table[])(uint16_t, uint16_t);

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
    bool stepInSignal;
    int stepInLine;

    LC3VMdisawindow();
    LC3VMdisawindow(uint16_t instrStream[], uint16_t numInstr, uint16_t address, const WindowConfig& config);
    ~LC3VMdisawindow() = default;

    void Load_Config(const WindowConfig& config);
    void Load(uint16_t instrStream[], uint16_t numInstr, uint16_t address);
    void Draw(void);
    
};