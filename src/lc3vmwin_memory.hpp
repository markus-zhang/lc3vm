#ifndef _LC3VMWIN_MEMORY_HPP_
#define _LC3VMWIN_MEMORY_HPP_

#include "globals.h"
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
    bool editorMode;
    int memoryEditedIndex;
    bool memoryEditedIndexLocked;
    bool quitSignal;

    LC3VMMemorywindow() = delete;
    LC3VMMemorywindow(unsigned char* memory, int memorySize, const WindowConfig& config);
    ~LC3VMMemorywindow() = default;

    void Draw();
    void Editor(ImVec2 mousePos, char* c, char original);
    unsigned char Calculate_Char(char buf[], char original);
};

// void test()
// {
//     ImGuiIO& io = ImGui::GetIO();
//     ImFont* font = io.Fonts->AddFontDefault();
//     font->CalcTextSizeA()
// }

#endif