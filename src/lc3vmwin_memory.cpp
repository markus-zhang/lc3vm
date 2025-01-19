#include "lc3vmwin_memory.hpp"
#include <imgui/imgui_sdl.h>
#include <iomanip>


LC3VMMemorywindow::LC3VMMemorywindow(char* memory, int memorySize, const WindowConfig& config)
{
    assert(memory != nullptr);

    buffer = std::vector<Glyph>(memorySize, {0, 255, 255, 255});
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
    initialAddress      = 0;    // This value is to be aligned down to 16-byte (0xab00)
    selection = std::vector<bool>(bufferSize, false);
}

void LC3VMMemorywindow::Draw()
{
    /*
        This is to be put into a rendering loop (e.g. SDL2)
        - Each time it checks initialAddress and render 20 lines of 16 bytes
        - Each byte is represented witha Selectable (thus we only have 320 of them for each frame)
        - Two buttons to move initialAddress, and one text input to enter an arbitrary initialAddress, again to be aligned down to 16-byte
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
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse
    );

    // ImGui::PushFont(font);
    // ImVec2 charSize = font->CalcTextSizeA(20.0f, FLT_MAX, 0.0f, "A");

    // Render first row
    // Address    00 01 ... 0F ASCII
    ImGui::SetCursorPos({20.0f, 20.0f});

    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
    ImGui::Text("Address\t");
    ImGui::SameLine();
    ImGui::Text(" 00");
    ImGui::SameLine();
    ImGui::Text(" 01");
    ImGui::SameLine();
    ImGui::Text(" 02");
    ImGui::SameLine();
    ImGui::Text(" 03");
    ImGui::SameLine();
    ImGui::Text(" 04");
    ImGui::SameLine();
    ImGui::Text(" 05");
    ImGui::SameLine();
    ImGui::Text(" 06");
    ImGui::SameLine();
    ImGui::Text(" 07");
    ImGui::SameLine();
    ImGui::Text(" 08");
    ImGui::SameLine();
    ImGui::Text(" 09");
    ImGui::SameLine();
    ImGui::Text(" 0A");
    ImGui::SameLine();
    ImGui::Text(" 0B");
    ImGui::SameLine();
    ImGui::Text(" 0C");
    ImGui::SameLine();
    ImGui::Text(" 0D");
    ImGui::SameLine();
    ImGui::Text(" 0E");
    ImGui::SameLine();
    ImGui::Text(" 0F");
    ImGui::SameLine();
    ImGui::Text("\tASCII");
    // 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\tASCII");
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::Button("Row Up");
    ImGui::SameLine();
    ImGui::Button("Page Up");

    // Render memory glyphs

    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
    // int lines = bufferSize / 0x0F;  // Every line contains 16 bytes
    // lines += (bufferSize % 0x0F == 0 ? 0 : 1);

    std::stringstream ss;
    const char* text = " 00";
    ImVec2 textSize = ImGui::CalcTextSize(text);    // to get correct Selectable size for each glyph (note that the length also includes the prefix space)
    
    // for (int i = 0; i < bufferSize; i++)
    // {
    //     ImGui::SetCursorPosX(20.0f);       
    //     // Header
    //     if (i % 16 == 0)
    //     {
    //         ss << "0x3000 \t";
    //     }
    //     ss << ' ';
    //     ss << std::hex << std::setfill('0');
    //     ss << std::setw(2) << static_cast<unsigned>(buffer[i].ch);
    //     if (i % 16 == 15 || i == bufferSize - 1)
    //     {
    //         ss << "\t";
    //         // ss << ASCII stuffs
    //         std::string line = ss.str();
    //         ImGui::Text("%s", line.c_str());        
    //         ss.str("");
    //         ss.clear();
    //     }
    // }

    // static ImGuiListClipper clipper;
    // clipper.Begin(bufferSize / 16);
    // while (clipper.Step())
    // {       
    // }

    for (int i = initialAddress; i < initialAddress + 20 * 16; i++)
    {
        /*
            Starting from initialAddress, we only render 320 selectables each frame
        */
        
        // Header
        if (i % 16 == 0)
        {
            ImGui::SetCursorPosX(20.0f);
            ss << "0x" << std::hex << std::setfill('0') << std::setw(4) << static_cast<int>(i) << " \t";
            std::string header = ss.str();
            ImGui::Text("%s", header.c_str());
            ss.str("");
            ss.clear();
            ImGui::SameLine();
        }
        ss << ' ' << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(buffer[i].ch);
        std::string byteHex = ss.str();

        if (ImGui::Selectable(byteHex.c_str(), selection[i], ImGuiSelectableFlags_None, textSize))
        {
            selection[i] = !selection[i];
        }
        ss.str("");
        ss.clear();
        ImGui::SameLine();

        if (i % 16 == 15)
        {
            ImGui::Text("\t");
        }
    }
    ImGui::SameLine();
    ImGui::Button("Row Down");
    ImGui::SameLine();
    ImGui::Button("Page Down");

    ImGui::PopStyleColor();

    ImGui::Separator();

    // ImGui::PopFont();
    ImGui::End();
}