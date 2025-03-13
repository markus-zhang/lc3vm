/*
    A revised version of the lc3vmwin_memory

    .PLAN 

    - Users can use keyboard to navigate through the memory area
        - Cursor must use very distinct foreground and background color
            - Is it possible to blink?
        - Cursor has two indices: Start and End
            - Everything between the two should have a rect border

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

#include "memory_editor.hpp"
#include <string>
#include <sstream>
#include <iomanip>


MemoryEditor::MemoryEditor()
{
    buffer.reserve(0);
    bufferSize = 0;
    readOnly = true;
    fontSize = 14;
    initialWindowSize = {0, 0};
    minWindowSize = {0, 0};
    winPos = {0, 0};
    cursorStartIndex = 0;
    cursorEndIndex = 0;
    initialAddress = 0;
}

MemoryEditor::MemoryEditor(uint8_t* memory, uint64_t memorySize, const ImGuiWindowConfig& config)
{
    IM_ASSERT(memory != nullptr);

    // TODO: Add a warning dialog if memorySize is over 1GB

    buffer = std::vector<charGlyph>(memorySize, {0, 255, 255, 255});

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

    cursorStartIndex    = 0;
    cursorEndIndex      = 0;
    initialAddress      = 0;
}

void MemoryEditor::Draw()
{
    ImGui::GetStyle().WindowBorderSize = 2.0f;
    ImGui::SetNextWindowPos(winPos);
    ImGui::SetNextWindowSizeConstraints(minWindowSize, initialWindowSize);

    /*
        EXPLAIN:
        I don't know for sure, but looks like we add all lines, rectangles into a draw list
        https://github.com/WerWolv/ImHex/blob/00cf8ecb18b2024ba375c353ce9680d33512f65a/libs/ImGui/include/imgui_memory_editor.h#L212C9-L212C60
    */
    ImDrawList* drawList = ImGui::GetForegroundDrawList();

    if (ImGui::Begin("Memory Editor Window", nullptr, ImGuiWindowFlags_None))
    {

        /* 
            EXPLAIN: 
            Render first row -> each row shows 16 bytes
            Address    00 01 ... 0F ASCII
        */

        ImGui::SetCursorPos({20.0f, 20.0f});

        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));

        /* 
            EXPLAIN:
            This is to get correct Selectable size for each glyph 
            (note that the length also includes the prefix space)
        */
        ImVec2 textSize = ImGui::CalcTextSize(" 00 ");

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
        /*
            EXPLAIN:
            Get the Cursor X to draw the last line of ASCII if bufferSize does not divide 16,
            in that case we need to move Cursor X to the same X position
        */
        ImVec2 asciiPos = ImGui::GetCursorScreenPos();
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
            We need quite a few explanations for the drawing of the glyphs themselves

            1. We only render 512 InputText each frame (32 rows * 16 bytes per row)

            2. For each byte, we need to PushID(i) to avoid conflict IDs, and PopID() at the end

            TODO: THIS IS JUST A TEST
            3. Header byte of each row is rendered in different color. 
        */

        for (size_t i = initialAddress; i < initialAddress + bufferSize; i++)
        {
            /*
                Starting from initialAddress, we only render a max of 512 bytes each frame - 
                This is 16 bytes per row * 32 rows hardcoded
            */

            // EXPLAIN: PushID() to avoid conflict IDs
            ImGui::PushID(i);

            // Header
            if (i % 16 == 0)
            {
                ImGui::SetCursorPosX(20.0f);
                ss << "0x" << std::hex << std::setfill('0') << std::setw(5) << static_cast<int>(i) << " ";
                std::string header = ss.str();
                ImGui::Text("%s", header.c_str());
                ss.str("");
                ss.clear();
                ImGui::SameLine();
            }

            /*
                EXPLAIN:
                This is how we render the rectangles for "selected" cells.
                Basically every cell within the cursor range should be "selected"
                
                StartIndex is not necessarily <= EndIndex
            */
            
            int64_t cursorMinIndex = (cursorStartIndex <= cursorEndIndex ? cursorStartIndex : cursorEndIndex);
            int64_t cursorMaxIndex = (cursorStartIndex <= cursorEndIndex ? cursorEndIndex : cursorStartIndex);

            if (i >= cursorMinIndex && i <= cursorMaxIndex)
            {
                // TODO: Switch the background and foreground colors of the cells "selected"
                ImGui::SameLine();
                ImVec2 cursorPosUpperLeft = ImGui::GetCursorScreenPos();
                ImVec2 cursorPosLowerRight = ImVec2(cursorPosUpperLeft.x + textSize.x, cursorPosUpperLeft.y + textSize.y);
                // Some sort of light blue rectangle
                drawList->AddRect(cursorPosUpperLeft, cursorPosLowerRight, IM_COL32(147, 181, 196, 255));
                ImGui::SameLine();
            }

            ss << ' ' << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(buffer[i].ch);
            std::string byteHex = ss.str();
            ImGui::Text("%s", byteHex.c_str());
            ss.str("");
            ss.clear();

            ImGui::SameLine();

            /*
                EXPLAIN:
                If bufferSize cannot be divided by 16 (e.g. 60 bytes, so the last row is ),
                we need to move the cursor (as the cursor is not at the ASCII position)
            */

            if (i % 16 == 15 || i == initialAddress + bufferSize - 1)
            {
                ImVec2 moveToASCIIPos = ImVec2(asciiPos.x, ImGui::GetCursorScreenPos().y);
                ImGui::SetCursorPos(moveToASCIIPos);
                ImGui::Text(" ");
                int64_t leftIndex = (i / 16) * 16;
                for (size_t j = leftIndex; j <= i; j++)
                {
                    ImGui::SameLine();
                    char ascii = buffer[j].ch;
                    // ss << (std::isprint(ascii) ? ascii : '.');
                    ascii = (std::isprint(ascii) ? ascii : '.');

                    if (j >= cursorMinIndex && j <= cursorMaxIndex)
                    {
                        // TODO: Switch the background and foreground colors of the cells "selected"
                        ImGui::SameLine();
                        ImVec2 cursorPosUpperLeft = ImGui::GetCursorScreenPos();
                        ImVec2 asciiTextSize = ImGui::CalcTextSize(" 0");
                        ImVec2 cursorPosLowerRight = ImVec2(cursorPosUpperLeft.x + asciiTextSize.x, cursorPosUpperLeft.y + asciiTextSize.y);
                        // Some sort of light blue rectangle
                        drawList->AddRect(cursorPosUpperLeft, cursorPosLowerRight, IM_COL32(147, 181, 196, 125));
                        ImGui::SameLine();
                    }

                    ImGui::Text("%c", ascii);
                }
                // ImGui::SameLine();
                // std::string asciiString = ss.str();
                // ImGui::Text("%s", asciiString.c_str());
                ss.str("");
                ss.clear();
            }

            ImGui::PopID();
        }

        ImGui::PopStyleColor();
    }

    /* 
        TODO: Implement keypress
        Check this piece of code for reference:
        https://github.com/WerWolv/ImHex/blob/00cf8ecb18b2024ba375c353ce9680d33512f65a/libs/ImGui/include/imgui_memory_editor.h#L260
    */

    if (ImGui::IsWindowFocused())
    {
        bool isCtrlDown = false;
        bool isShiftDown = false;
        // TODO: Use flags isCtrlDown and isShiftDown
        if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
        {
            isCtrlDown = true;
        }

        if (ImGui::IsKeyDown(ImGuiKey_LeftShift))
        {
            isShiftDown = true;
        }

        if (ImGui::IsKeyPressed(ImGuiKey_RightArrow))
        {
            /* CTRL + SHIFT + RIGHT ARROW for contiguous selection towards the end of row */
            if (isCtrlDown && isShiftDown)
            {
                cursorEndIndex = cursorEndIndex | 0x0F;
            }
            /* CTRL + RIGHT ARROW for jumping to end of row */
            else if (isCtrlDown)
            {
                /* EXPLAIN: Check for last row that has less than 16 cells */
                if (cursorStartIndex | 0x0F > bufferSize - 1)
                {
                    cursorStartIndex = bufferSize - 1;
                }
                else
                {
                    cursorStartIndex = cursorStartIndex | 0x0F;
                }
                cursorEndIndex = cursorStartIndex;
            }
            /* 
                EXPLAIN:
                SHIFT + RIGHT ARROW for contiguous selection with the next address
            */
            else if (isShiftDown)
            {
                /*
                    EXPLAIN:
                    Reset cursorEndIndex in case there are cells already selected
                */
                // cursorEndIndex = cursorStartIndex;
                if (cursorEndIndex < bufferSize - 1)
                {
                    cursorEndIndex++;
                }
            }
            /* ARROW for moving the cursor */
            else
            {
                if (cursorEndIndex < bufferSize - 1)
                {
                    cursorStartIndex++;
                    cursorEndIndex = cursorStartIndex;
                }
            }
        }

        if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow))
        {
            /* 
                EXPLAIN: 
                CTRL + SHIFT + LEFT ARROW for contiguous selection towards the end of row 
                NOTE: always fix cursorStartIndex and move cursorEndIndex
            */
            if (isCtrlDown && isShiftDown)
            {
                cursorEndIndex &= (int64_t)0xFFFFFFFFFFFFFFF0;
            }
            /* 
                EXPLAIN: 
                CTRL + LEFT ARROW for jumping to the first cell of row 
            */
            else if (isCtrlDown)
            {
                cursorStartIndex &= (int64_t)0xFFFFFFFFFFFFFFF0;
                cursorEndIndex = cursorStartIndex;
            }
            /* 
                EXPLAIN: 
                SHIFT + LEFT ARROW for contiguous selection with the previous address 
                NOTE: always fix cursorStartIndex and move cursorEndIndex
            */
            else if (isShiftDown)
            {
                if (cursorEndIndex > 0)
                {
                    cursorEndIndex--;
                }
            }
            /* 
                EXPLAIN: 
                LEFT ARROW for moving the cursor to the previous address 
            */
            else
            {
                if (cursorEndIndex > 0)
                {
                    cursorStartIndex--;
                    cursorEndIndex = cursorStartIndex;
                }
            }
        }

        if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))
        {
            if (isShiftDown)
            {
                /*
                    EXPLAIN:
                    SHIFT + DOWN ARROW = adding the next 16 cells after cursorEndIndex into selection
                    if cursorEndIndex <= cursorStartIndex, reset cursorEndIndex first
                    if cursorEndIndex >= cursorStartIndex, no need to reset
                */

                if (cursorEndIndex + 16 <= bufferSize - 1)
                {
                    cursorEndIndex += 16;
                }
                else
                {
                    cursorEndIndex = bufferSize - 1;
                }
            }
            else
            {
                /*
                    EXPLAIN:
                    If possible, go down one row to the same column, otherwise go to the last cell
                    Exception: if we are already at the last row, DON'T go to the last cell, it feels weird
                */
                if ((cursorStartIndex | 0x0F) != ((bufferSize - 1) | 0x0F))
                {
                    if (cursorStartIndex + 16 > bufferSize - 1)
                    {
                        cursorStartIndex = bufferSize - 1;
                    }
                    else
                    {
                        cursorStartIndex += 16;
                    }
                    cursorEndIndex = cursorStartIndex;
                }
            }
        }

        if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))
        {
            if (isShiftDown)
            {
                /*
                    EXPLAIN:
                    SHIFT + UP ARROW = adding the previous 16 cells after cursorEndIndex into selection
                */

                if (cursorEndIndex - 16 >= 0)
                {
                    cursorEndIndex -= 16;
                }
                else
                {
                    cursorEndIndex = 0;
                }
            }
            else 
            {
                /*
                    EXPLAIN:
                    If possible, go up one row to the same column, otherwise go to the first cell
                    Exception: if we are already at the first row, DON'T go to the first cell, it feels weird
                */
               printf("Shift is NOT down!\n");
                if (cursorStartIndex > 0x0F)
                {
                    int64_t cursorStartIndexTemp = cursorStartIndex - 16;
                    if (cursorStartIndexTemp < 0)
                    {
                        cursorStartIndexTemp = 0;
                    }
                    cursorStartIndex = cursorStartIndexTemp;
                    cursorEndIndex = cursorStartIndex;
                }
            }
        }

        
    }

    ImGui::End();
}