#pragma once

#include "globals.hpp"
#include <string>
#include <vector>

class LC3VMMemorywindow
{
public:
    std::vector<Glyph> buffer;
    int bufferSize;
    bool readonly;
    int fontSize;
    ImVec2 initialWindowSize;
    ImVec2 minWindowSize;
    ImVec2 winPos;
    bool initialized;
    ImFont* font;
    uint16_t initialAddress;    // The first line starts from initialAddress
    std::vector<bool> selection;
    bool mouseDoubleClicked;
    ImVec2 mousePos;
    bool editorMode;    // For double clicking an address to pop-up the editor window
    size_t memoryEditedIndex;
    bool memoryEditedIndexLocked; 
    bool quitSignal;
    bool addressInputMode;  // For the inputText under the memory content

    /* We need a default constructor to write LC3VMMemorywindow window; */
    LC3VMMemorywindow();
    LC3VMMemorywindow(uint16_t* memory, size_t memorySize, const WindowConfig& config);
    ~LC3VMMemorywindow() = default;

    void Draw();
    void Editor(ImVec2 mousePos, char* c, char original);
    unsigned char Calculate_Char(char buf[], char original);
    /* A hex char array (e.g. 0F3C) to  */
    uint64_t Char_Array_to_Number(char buf[], size_t numDigits);
    uint64_t Char_to_Number(char ch);
};

// void test()
// {
//     ImGuiIO& io = ImGui::GetIO();
//     ImFont* font = io.Fonts->AddFontDefault();
//     font->CalcTextSizeA()
// }