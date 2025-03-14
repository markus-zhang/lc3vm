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
    font = nullptr;
    fontSize = 14;
    initialWindowSize = {0, 0};
    minWindowSize = {0, 0};
    winPos = {0, 0};
    cursorStartIndex = 0;
    cursorEndIndex = 0;
    initialAddress = 0;
    minInitialAddress = 0;
    maxInitialAddress = 0;
    cursorStartBoundary = 0;
    cursorEndBoundary = 0;
    drawList = nullptr;
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

    minInitialAddress   = 0;
    maxInitialAddress   = CalculateMaxInitialAddress();
    cursorStartBoundary = 0;
    cursorEndBoundary   = bufferSize - 1;
    
    // FIXME: find a custom font that accepts visible space
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("./JetBrainsMono.ttf", fontSize, nullptr, io.Fonts->GetGlyphRangesDefault());
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
    
    drawList = ImGui::GetForegroundDrawList();

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
        */

        for (size_t i = initialAddress; i < initialAddress + PAGE_ROWS * PAGE_COLUMNS; i++)
        // for (size_t i = initialAddress; i < bufferSize; i++)
        {
            /*
                Starting from initialAddress, we only render a max of 512 bytes each frame - 
                This is 16 bytes per row * 32 rows hardcoded
            */
            
            // Break if i is over bufferSize - 1, 
            // because sometimes the last row doesn't always have PAGE_COLUMNS bytes
            if (i > bufferSize - 1)
            {
                break;
            }

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
                /* 
                    TODO: 
                    Figure out a way to draw contiguous rectangle instead of a lot of small rectangles
                    - We probably need to use cursorMinIndex and cursorMaxIndex
                    - What if we need to draw multiple lines?
                    - Actually, for the first cell of second row, how do we draw a cell without both left and right borders?
                */
                drawList->AddRectFilled(cursorPosUpperLeft, cursorPosLowerRight, IM_COL32(125, 175, 175, 100));
                ImGui::SameLine();
            }

            ss << ' ' << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(buffer[i].ch);
            std::string byteHex = ss.str();
            
            // EXPLAIN: We use a light green color for rectangles so text should be yellow
            if (i >= cursorMinIndex && i <= cursorMaxIndex)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));
                ImGui::Text("%s", byteHex.c_str());
                ImGui::PopStyleColor();
            }
            else
            {
                ImGui::Text("%s", byteHex.c_str());
            }
            ss.str("");
            ss.clear();

            ImGui::SameLine();

            /*
                EXPLAIN:
                If bufferSize cannot be divided by 16 (e.g. 60 bytes, so the last row is ),
                we need to move the cursor (as the cursor is not at the ASCII position)
            */

            if ((i % 16 == 15) || (i == bufferSize - 1))
            {
                ImVec2 moveToASCIIPos = ImVec2(asciiPos.x, ImGui::GetCursorScreenPos().y);
                ImGui::SetCursorPos(moveToASCIIPos);
                ImGui::Text(" ");
                int64_t leftIndex = (i / 16) * 16;
                for (int64_t j = leftIndex; j <= i; j++)
                {
                    ImGui::SameLine();
                    char ascii = buffer[j].ch;
                    // ss << (std::isprint(ascii) ? ascii : '.');
                    ascii = (std::isprint(ascii) ? ascii : '.');


                    if (j >= cursorMinIndex && j <= cursorMaxIndex)
                    {
                        // TODO: Switch the background and foreground colors of the cells "selected"
                        ImVec2 cursorPosUpperLeft = ImGui::GetCursorScreenPos();
                        ImVec2 asciiTextSize = ImGui::CalcTextSize(" 0");
                        ImVec2 cursorPosLowerRight = ImVec2(cursorPosUpperLeft.x + asciiTextSize.x, cursorPosUpperLeft.y + asciiTextSize.y);
                        // Some sort of light blue rectangle
                        drawList->AddRectFilled(cursorPosUpperLeft, cursorPosLowerRight, IM_COL32(125, 175, 175, 100));

                        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));
                        // FIXME: Use a different font to display Unicode
                        // ImGuiIO& io = ImGui::GetIO();
                        // io.Fonts->AddFontFromFileTTF("path/to/NotoSans-Regular.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesDefault());
                        if (ascii == ' ')
                        {
                            ImGui::Text("\xE2\x90\xA3");
                        }
                        else
                        {
                            ImGui::Text("%c", ascii);
                        }
                        ImGui::PopStyleColor();
                    }
                    else
                    {
                        ImGui::Text("%c", ascii);
                    }
                }
                // ImGui::SameLine();
                // std::string asciiString = ss.str();
                // ImGui::Text("%s", asciiString.c_str());
                ss.str("");
                ss.clear();
            }

            ImGui::PopID();
        }

        Input();

        ImGui::PopStyleColor();
    }

    ImGui::End();
}

void MemoryEditor::Input()
{
    /* 
        Keypresses:
        Check this piece of code for reference:
        https://github.com/WerWolv/ImHex/blob/00cf8ecb18b2024ba375c353ce9680d33512f65a/libs/ImGui/include/imgui_memory_editor.h#L260

        Move initialAddress if any of the selected area reaches off screen addresses
    */

    if (ImGui::IsWindowFocused())
    {
        bool isCtrlDown = false;
        bool isShiftDown = false;
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
            /* 
                EXPLAIN:
                CTRL + SHIFT + RIGHT ARROW for contiguous selection towards the end of row 
                No need to adjust initialAddress as we are not moving vertically
            */
            if (isCtrlDown && isShiftDown)
            {
                cursorEndIndex = cursorEndIndex | 0x0F;
            }
            /* 
                EXPLAIN:
                CTRL + RIGHT ARROW for jumping to end of row 
                No need to adjust initialAddress as we are not moving vertically
            */
            else if (isCtrlDown)
            {
                /* EXPLAIN: Check for last row that has less than 16 cells */
                if ((cursorStartIndex | 0x0F) > bufferSize - 1)
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
            /*
                EXPLAIN:
                RIGHT ARROW moves the cursor to the next cell
                We also adjust initialAddress if needed
            */
            else
            {
                if (cursorStartIndex < bufferSize - 1)
                {
                    cursorStartIndex += 1;
                }

                if (cursorStartIndex >= (initialAddress + PAGE_ROWS * PAGE_COLUMNS))
                {
                    initialAddress += PAGE_COLUMNS;
                }

                // Always reset cursorEndIndex, just in case there are multiple selected cells
                cursorEndIndex = cursorStartIndex;
            }
        }

        if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow))
        {
            /* 
                EXPLAIN: 
                CTRL + SHIFT + LEFT ARROW for contiguous selection towards the end of row 
                NOTE: always fix cursorStartIndex and move cursorEndIndex

                No need to adjust initialAddress as we are not moving vertically
            */
            if (isCtrlDown && isShiftDown)
            {
                cursorEndIndex &= (int64_t)0xFFFFFFFFFFFFFFF0;
            }
            /* 
                EXPLAIN: 
                CTRL + LEFT ARROW for jumping to the first cell of row 

                No need to adjust initialAddress as we are not moving vertically
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
                We also check screen boundaries (initialAddress -> initialAddress + 32 * 16 - 1) 
            */
            else
            {
                if (cursorStartIndex > 0)
                {
                    cursorStartIndex -= 1;

                    if (cursorStartIndex < initialAddress)
                    {
                        initialAddress -= PAGE_COLUMNS;
                    }
                }
                // Always reset cursorEndIndex, just in case there are multiple selected cells
                cursorEndIndex = cursorStartIndex;
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

                    We need to adjust initialAddress as we are now moving vertically --

                */

                if (cursorEndIndex + 16 <= bufferSize - 1)
                {
                    cursorEndIndex += 16;
                }
                else
                {
                    cursorEndIndex = bufferSize - 1;
                }

                ResetInitialAddress(BREAKTHROUGH_ENDINDEX);
            }
            else if (isCtrlDown)
            {
                /*
                    EXPLAIN:
                    Just turn a page if possible, otherwise take the last cell. 
                    This is for fast scrolling -> 512 bytes per keystroke
                */
                cursorStartIndex = initialAddress + PAGE_ROWS * PAGE_COLUMNS + (cursorStartIndex & 0x0F);
                if (cursorStartIndex > bufferSize - 1)
                {
                    cursorStartIndex = bufferSize - 1;
                }
                cursorEndIndex = cursorStartIndex;
                ResetInitialAddress(BREAKTHROUGH_STARTINDEX);
            }
            else
            {
                /*
                    EXPLAIN:
                    If possible, go down one row to the same column, otherwise go to the last cell
                    Exception: if we are already at the last row, DON'T go to the last cell, it feels weird

                    We also need to adjust initialAddress if cursorStartIndex goes out of the bigger boundary
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
                }
                // Always reset cursorEndIndex, just in case there are multiple selected cells
                cursorEndIndex = cursorStartIndex;

                // Reset initialAddress based on cursorStartIndex if out of window
                ResetInitialAddress(BREAKTHROUGH_STARTINDEX);
            }
        }

        if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))
        {
            if (isShiftDown)
            {
                /*
                    EXPLAIN:
                    SHIFT + UP ARROW = adding the previous 16 cells after cursorEndIndex into selection

                    We also need to move initialAddress up one row if needed
                */

                if (cursorEndIndex - 16 >= 0)
                {
                    cursorEndIndex -= 16;
                }
                else
                {
                    cursorEndIndex = 0;
                }

                ResetInitialAddress(BREAKTHROUGH_ENDINDEX);
            }
            else if (isCtrlDown)
            {
                /*
                    EXPLAIN:
                    Just move up one page, or go to the first cell if that's not possible
                */
                cursorStartIndex = initialAddress - PAGE_ROWS * PAGE_COLUMNS + (cursorStartIndex & 0x0F);
                if (cursorStartIndex < 0)
                {
                    cursorStartIndex = 0;
                }
                cursorEndIndex = cursorStartIndex;
                ResetInitialAddress(BREAKTHROUGH_STARTINDEX);
            }
            else 
            {
                /*
                    EXPLAIN:
                    If possible, go up one row to the same column, otherwise go to the first cell
                    Exception: if we are already at the first row, DON'T go to the first cell, it feels weird

                    We also need to adjust initialAddress if cursorStartIndex goes out of the smaller boundary
                */
                if (cursorStartIndex > 0x0F)
                {
                    if (cursorStartIndex < 16)
                    {
                        cursorStartIndex = 0;
                    }
                    else
                    {
                        cursorStartIndex -= 16;
                    }

                    // if (cursorStartIndex < initialAddress)
                    // {
                    //     initialAddress -= PAGE_COLUMNS;
                    // }
                }
                // Always reset cursorEndIndex, just in case there are multiple selected cells
                cursorEndIndex = cursorStartIndex;

                // Reset initialAddress based on cursorStartIndex if out of window
                // When we reset when cursor is oving up to the previous screen, we want maximum
                // visibility for the user, so initialAddress should be PAGE_ROWS rows away
                // cursorStartIndex
                ResetInitialAddress(BREAKTHROUGH_STARTINDEX);
            }
        }
    }
}

int64_t MemoryEditor::CalculateMaxInitialAddress()
{
    /*
        EXPLAIN:
        First, we go back to the first cell of the last row, because we already know bufferSize;
        Next, we try to go up 31 (which is PAGE_ROWS - 1) rows;
        And if it becomes negative, we just set it to 0 (whole buffer is less than one page)
    */
    
    if (bufferSize <= PAGE_ROWS * PAGE_COLUMNS)
    {
        return 0;
    }

    int64_t maxInitialAddress = (bufferSize - 1) & (int64_t)0xFFFFFFFFFFFFFFF0;
    maxInitialAddress -= (PAGE_COLUMNS * (PAGE_ROWS - 1));

    if (maxInitialAddress < 0)
    {
        return 0;
    }
    else
    {
        return maxInitialAddress;
    }
}

void MemoryEditor::ResetInitialAddress(enum BREAKTHROUGH_TYPE bType)
{
    /*
        Call this function AFTER you changed cursorStartIndex/cursorEndIndex
    */

    if (bType == BREAKTHROUGH_STARTINDEX)
    {
        if (cursorStartIndex < initialAddress)
        {
            /*
                EXPLAIN:
                Now we are probably breaking UP (to a lower address page)
            */
            
            initialAddress = (cursorStartIndex & (int64_t)0xFFFFFFFFFFFFFFF0) - (PAGE_ROWS - 1) * PAGE_COLUMNS;
            if (initialAddress < 0)
            {
                initialAddress = 0;
            }
        }
        else if (cursorStartIndex >= initialAddress + PAGE_ROWS * PAGE_COLUMNS)
        {
            /*
                EXPLAIN:
                Now we are probably breaking DOWN (to a higher address page)
            */

            initialAddress = cursorStartIndex & (int64_t)0xFFFFFFFFFFFFFFF0;
            if (initialAddress > maxInitialAddress)
            {
                initialAddress = maxInitialAddress;
            }
        }
    }
    else if (bType == BREAKTHROUGH_ENDINDEX)
    {
        if (cursorEndIndex < initialAddress)
        {
            // initialAddress -= PAGE_COLUMNS;
            initialAddress = initialAddress - (PAGE_ROWS - 1) * PAGE_COLUMNS;
            if (initialAddress < 0)
            {
                initialAddress = 0;
            }
        }
        else if (cursorEndIndex >= initialAddress + PAGE_ROWS * PAGE_COLUMNS)
        {
            initialAddress += PAGE_COLUMNS;
        }
    }
}