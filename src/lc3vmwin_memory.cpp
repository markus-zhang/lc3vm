#include "lc3vmwin_memory.hpp"
#include <imgui/imgui_sdl.h>
#include <iomanip>


LC3VMMemorywindow::LC3VMMemorywindow(char* memory, int memorySize, const WindowConfig& config)
{
    assert(memory != nullptr);

    buffer.reserve(memorySize);
    bufferSize = memorySize;
    while (*memory != '\0')
    {    
        buffer.push_back({*memory, 255, 255, 255});
        memory++;
    }

    readonly            = config.readonly;
    fontSize            = config.fontSize;
    initialWindowSize   = config.initialWindowSize;
    minWindowSize       = config.minWindowSize;
    winPos              = config.winPos;

    initialized         = false;
    ImGuiIO& io = ImGui::GetIO();
    ImFontConfig fontConfig;
    fontConfig.SizePixels = 20.0f;  // 15pt 
    font = io.Fonts->AddFontDefault(&fontConfig);
    assert(font != nullptr);
    // io.Fonts->Build();
    // font = nullptr;
}

void LC3VMMemorywindow::Draw()
{
    /*
        This is to be put into a rendering loop (e.g. SDL2)
    */

    if (!initialized)
    {
        ImGui::SetNextWindowSize(initialWindowSize, 0);
        initialized = true;
    }

    ImGui::GetStyle().WindowBorderSize = 2.0f;
    ImGui::SetNextWindowPos(winPos);
    ImGui::SetNextWindowSizeConstraints(minWindowSize, initialWindowSize);

    ImGui::Begin(
        "Memory Watch Window",
        nullptr,
        ImGuiWindowFlags_NoCollapse
    );

    // ImGui::PushFont(font);
    // ImVec2 charSize = font->CalcTextSizeA(20.0f, FLT_MAX, 0.0f, "A");

    // Render first row
    // Address    00 01 ... 0F ASCII
    ImGui::SetCursorPos({20.0f, 20.0f});

    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
    ImGui::Text("Address\t 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\tASCII");
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::Button("Switch Encoding");

    // Render memory glyphs

    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
    // int lines = bufferSize / 0x0F;  // Every line contains 16 bytes
    // lines += (bufferSize % 0x0F == 0 ? 0 : 1);

    std::stringstream ss;
    
    for (int i = 0; i < bufferSize; i++)
    {
        ImGui::SetCursorPosX(20.0f);
        
        // Header
        if (i % 16 == 0)
        {
            ss << "0x3000 \t";
        }
        ss << ' ';
        ss << std::hex << std::setfill('0');
        ss << std::setw(2) << static_cast<unsigned>(buffer[i].ch);

        if (i % 16 == 15)
        {
            ss << "\t";
            // ss << ASCII stuffs
            std::string line = ss.str();
            ImGui::Text("%s", line.c_str());
        
            ss.str("");
            ss.clear();
        }
    }

    ImGui::PopStyleColor();

    ImGui::Separator();

    // ImGui::PopFont();
    ImGui::End();
}