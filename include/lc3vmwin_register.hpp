#pragma once

#include "globals.hpp"
#include <string>
#include <vector>

/*
    I think it helps to be a bit more general
    - No limited number of registers
    - As many status regiaters (Z/N/P) as possible
    - No hardcoded number of digits displayed (e.g. could display 0x4501 for 16-bit, 0r 0x00004501 for 32-bit)
*/

class LC3VMRegisterWindow
{
public:
    /* 
        1. Whatever the length of a register is, it shoud/hopefully work 
        2. This requires that the registers to be passsed from the backend as an array
        3. Each Register should have a name indicated in regNames, also passed by the backend
        4. So essentially we have something like this:

        regFile = {0x0000, 0x0025, 0x0f40, 0x0600, 0xffff, 0xf010}
        regNum = 6
        fegNames = {"PC"}
    */
    int regNum;
    const void* regFile;
    const char** regNames;

    /* This is used to limit the number of digits shown, e.g. if regSizeInBytes is 4 it means 4 bytes so it's going to show 0x0000ff40 */
    int regSizeInBytes;

    ImVec2 initialWindowSize;
    ImVec2 minWindowSize;
    ImVec2 winPos;

    /* Member function lists */
    LC3VMRegisterWindow() = delete;
    LC3VMRegisterWindow(int regNum, const void* regFile, const char** regNames, int regSize, const WindowConfig& config);
    ~LC3VMRegisterWindow();

    Draw();

};