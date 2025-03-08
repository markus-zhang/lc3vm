/*
    A revised version of the lc3vmwin_memory

    .PLAN 

    - Users can use keyboard to navigate through the memory area
        - Cursor must use very distinct foreground and background color
            - Is it possible to blink?

        - CTRL / SHIFT / ALT + Arrow keys for navigation
            - LEFT for moving the cursor back 1 byte
            - RIGHT for moving the cursor forward 1 byte
            - UP for moving the cursor back 16 byte
            - DOWN for moving the cursor forward 16 byte

            - CTRL+LEFT for moving the cursor to the first byte of the current row
            - CTRL+RIGHT for moving the cursor to the last byte of the current row

            - SHIFT+LEFT for MARKING the byte and the previous byte as SELECTED, and move the cursor to the previous byte
            - SHIFT+RIGHT for MARKING the byte and the next byte as SELECTED, and move the cursor to the previous byte
            - SHIFT+UP for MARKING the byte and its previous 16 bytes as SELECTED, and move the cursor back 16 bytes
            - SHIFT+DOWN for MARKING the byte and its next 16 bytes as SELECTED, and move the cursor forward 16 bytes

            - ALT should be reserved for advanced usage, e.g. scripted cursor movements

        - PAGEUP same as the Page < button

        - PAGEDOWN same as the Page > button

        - HOME same as the Page << button

        - END same as the Page >> button

        - ENTER key for popping up the memory edit window

        - ESC to CANCEL all selections

        - CTRL + F1 ~ F12 to mark 12 locations, and F1 ~ F12 to jump to (shit man, this is awesome!)

        - CTRL + Z for undo and CTRL + Y for redo

        - CTRL + S for save as (for safety, do not save in position), CTRL + L for load file

    - A sidebar action window that displays a stack of user actions (for undoing)

    - A readonly toggle (this should make the UI slightly different)
*/

#include <imgui.h>
#include <vector>
#include <string>
#include <cinttypes>
#include <sstream>

struct Glyph
{
    unsigned char ch;
    int r;
    int g;
    int b;
};

struct WindowConfig
{
    bool readOnly;
    int fontSize;
    ImVec2 initialWindowSize;
    ImVec2 minWindowSize;
    ImVec2 winPos;
};

class MemoryEditor
{
public:
    /* Size and location */
    ImVec2 initialWindowSize;
    ImVec2 minWindowSize;
    ImVec2 winPos;

    std::vector<Glyph> buffer;
    uint64_t bufferSize;
    uint64_t cursorIndex;
    uint64_t initialAddress;

    bool readOnly;
    int fontSize;

public:
    MemoryEditor();
    MemoryEditor(uint8_t* memory, uint64_t memorySize, const WindowConfig& config);
    ~MemoryEditor() = default;

    void Draw();
};


MemoryEditor::MemoryEditor()
{
    buffer.reserve(0);
    bufferSize = 0;
    readOnly = true;
    fontSize = 14;
    initialWindowSize = {0, 0};
    minWindowSize = {0, 0};
    winPos = {0, 0};
}

MemoryEditor::MemoryEditor(uint8_t* memory, uint64_t memorySize, const WindowConfig& config)
{
    IM_ASSERT(memory != nullptr);

    // TODO: Add a warning dialog if memorySize is over 1GB

    buffer = std::vector<Glyph>(memorySize, {0, 255, 255, 255});
    bufferSize = memorySize;

    for (size_t i = 0; i < bufferSize; i++)
    {
        buffer[i] = {(*memory), 255, 255, 255};
        memory++;
    }

    readOnly            = config.readOnly;
    fontSize            = config.fontSize;
    initialWindowSize   = config.initialWindowSize;
    minWindowSize       = config.minWindowSize;
    winPos              = config.winPos;

    cursorIndex         = 0;
    initialAddress      = 0;
}

void MemoryEditor::Draw()
{
    ImGui::GetStyle().WindowBorderSize = 2.0f;
    ImGui::SetNextWindowPos(winPos);
    ImGui::SetNextWindowSizeConstraints(minWindowSize, initialWindowSize);

    ImGui::Begin(
        "Memory Watch Window",
        nullptr,
        ImGuiWindowFlags_NoResize
    );

    /* 
        EXPLAIN: 
        Render first row -> each row shows 16 bytes
        Address    00 01 ... 0F ASCII
    */

    ImGui::SetCursorPos({20.0f, 20.0f});

    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));

    ImGui::Text("Address ");
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
    ImGui::Text("        ASCII     ");

    ImGui::PopStyleColor();
    ImGui::SameLine();

    if (ImGui::Button("Page <"))
    {
        if (initialAddress <= 0x00200)
        {
            initialAddress = 0x00000;
        }
        else
        {
            initialAddress -= 0x200;
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Page <<"))
    {
        initialAddress = 0x00000;
    }

    // Render memory glyphs

    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));

    std::stringstream ss;

    /* 
        EXPLAIN:
        This is to get correct Selectable size for each glyph 
        (note that the length also includes the prefix space)
    */
    ImVec2 textSize = ImGui::CalcTextSize(" 00");

    /*
        EXPLAIN:
        We need quite a few explanations for the drawing of the glyphs themselves

        1. We only render 512 InputText each frame (32 rows * 16 bytes per row)

        2. For each byte, we need to PushID(i) to avoid conflict IDs, and PopID() at the end

        FIXME: THIS IS JUST A TEST
        3. Header byte of each row is rendered in different color. 
    */
    for (size_t i = initialAddress; i < initialAddress + 32 * 16; i++)
    {
        /*
            Starting from initialAddress, we only render 512 selectables each frame
        */

        // PushID() to avoid conflict IDs
        ImGui::PushID(i);


        ImGui::PopID();
    }
}