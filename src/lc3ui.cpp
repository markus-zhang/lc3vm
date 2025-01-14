#include "lc3ui.hpp"
#include <iostream>

void ui_debug_info(uint16_t* reg, int screenHeight)
{
    // Save the cursor position
    std::cout << "\033[s";
    std::cout << "\033[" << screenHeight - 1 << ";0H\033[KPC:" << std::hex << reg[8] << std::endl;
     // Restore the cursor position
    std::cout << "\033[u";
    // fflush(stdout);
}