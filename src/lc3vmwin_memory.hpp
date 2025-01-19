#ifndef _LC3VMWIN_MEMORY_HPP_
#define _LC3VMWIN_MEMORY_HPP_

#include <imgui/imgui.h>
#include <string>
#include <vector>

struct Glyph
{
    char ch;
    int r;
    int g;
    int b;
};

struct WindowConfig
{
    bool readonly;
    int fontSize;
    ImVec2 initialWindowSize;
    ImVec2 minWindowSize;
    ImVec2 winPos;
};

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

    LC3VMMemorywindow() = delete;
    LC3VMMemorywindow(char memory[], int memorySize, const WindowConfig& config);
    ~LC3VMMemorywindow() = default;

    void Draw();
    // void Modify(int index);
};

// void test()
// {
//     ImGuiIO& io = ImGui::GetIO();
//     ImFont* font = io.Fonts->AddFontDefault();
//     font->CalcTextSizeA()
// }

#endif