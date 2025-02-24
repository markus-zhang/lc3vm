#include "lc3vmwin_memory.hpp"
#include <iomanip>
#include <math.h> 


LC3VMMemorywindow::LC3VMMemorywindow()
{
    buffer.reserve(0);
    bufferSize = 0;
    readonly = true;
    fontSize = 14;
    initialWindowSize = {0, 0};
    minWindowSize = {0, 0};
    winPos = {0, 0};
    initialized = false;
    font = nullptr;
    initialAddress = 0x0000;
    selection.reserve(0);
    mouseDoubleClicked = false;
    mousePos = {0, 0};
    editorMode = false;
    memoryEditedIndex = 0x0000;
    memoryEditedIndexLocked = false;
    quitSignal = false;
    addressInputMode = false;
}

LC3VMMemorywindow::LC3VMMemorywindow(uint16_t* memory, size_t memorySize, const WindowConfig& config)
{
    assert(memory != nullptr);

    buffer = std::vector<Glyph>(memorySize, {0, 255, 255, 255});
    bufferSize = memorySize;
    for (size_t i = 0; i < bufferSize ;)
    {
        // if (*memory == '\0')
        // {
        //     break;
        // }    
        buffer[i] = {(unsigned char)((*memory) >> 8), 255, 255, 255};
        buffer[i + 1] = {(unsigned char)((*memory) & 0x00FF), 255, 255, 255};
        i += 2;
        memory++;
    }

    readonly            = config.readonly;
    fontSize            = config.fontSize;
    initialWindowSize   = config.initialWindowSize;
    minWindowSize       = config.minWindowSize;
    winPos              = config.winPos;

    initialized         = false;
    ImGuiIO& io = ImGui::GetIO();
    // io.AddInputCharacter(ImGuiKey_Backspace);
    ImFontConfig fontConfig;
    fontConfig.SizePixels = 14.0f;
    font = io.Fonts->AddFontDefault(&fontConfig);
    assert(font != nullptr);
    // io.Fonts->Build();
    // font = nullptr;
    initialAddress      = 0;    // This value is to be aligned down to 16-byte (0xab00)
    selection = std::vector<bool>(bufferSize, false);
    mouseDoubleClicked = false;
    mousePos = {-1, -1};
    // If the editor window is on, there are things that should be ignored, like pressing 'd' or 'esc', by the main SDL loop
    editorMode = false;
    /* 
        states for double clicking editing
        We can't put them into Draw(), because memoryEditedIndex would change with each SDL Loop (not the loop inside Draw)
        We have to put them as "global"
    */
    memoryEditedIndex = -1;
    memoryEditedIndexLocked = false;
    quitSignal = false;
    // char memoryEditedBackup = 0;
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
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBringToFrontOnFocus
    );

    // ImGui::PushFont(font);

    // Render first row
    // Address    00 01 ... 0F ASCII
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
    // I don't use "00 01 02 .. 0F\tASCII") because we need to match the same SameLine() call in the later for loop
    ImGui::PopStyleColor();
    ImGui::SameLine();
    if (ImGui::Button("Row <"))
    {
        if (initialAddress >= 0x10)
        {
            initialAddress -= 0x10;
        }
    }
    ImGui::SameLine();
    ImGui::Button("Page <");

    // Render memory glyphs

    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
    // int lines = bufferSize / 0x0F;  // Every line contains 16 bytes
    // lines += (bufferSize % 0x0F == 0 ? 0 : 1);

    std::stringstream ss;
    const char* text = " 00";
    ImVec2 textSize = ImGui::CalcTextSize(text);    // to get correct Selectable size for each glyph (note that the length also includes the prefix space)

    // static ImGuiListClipper clipper;
    // clipper.Begin(bufferSize / 16);
    // while (clipper.Step())
    // {       
    // }
    
    for (size_t i = initialAddress; i < initialAddress + 32 * 16; i++)
    {
        /*
            Starting from initialAddress, we only render 512 selectables each frame
        */

        // PushID() to avoid conflict IDs
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
        // iostream treats a char as a char so will display a char even with std::hex
        // to properly display the hex value we need to cast it to an integer first
        ss << ' ' << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(buffer[i].ch);
        std::string byteHex = ss.str();
        ss.str("");
        ss.clear();

        /* Memory display started */

        if (ImGui::Selectable(byteHex.c_str(), selection[i], ImGuiSelectableFlags_AllowDoubleClick, textSize))
        {
            selection[i] = !selection[i];
            /* Capture mouse double click */
            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                editorMode = true;
            }
            /*
                Figuring out which i we double clicked.
                We cannot open the editor window in a loop, it has to be after the loop, 
                so we need to record the index now.

                This NEEDS to be withint the Selectable() because it needs to stick with the i,
                otherwise i changes with mouse move.

                I THINK this is because Selectable() only returns true when use mouse click it,
                thus memoryEditedIndex only changes when user selects another one.

                TODO:
                - Prevent memoryEditedIndex changes with a single click
            */

            if (!memoryEditedIndexLocked && editorMode && ImGui::IsItemHovered())
            {
                memoryEditedIndex = i;
                printf("Index captured: %ld\n", memoryEditedIndex);
                memoryEditedIndexLocked = true;
            }
        }

        ImGui::SameLine();

        if (i % 16 == 15)
        {
            ImGui::Text(" ");
            // print the ASCII stuffs
            for (size_t j = i - 15; j <= i; j++)
            {
                char ascii = buffer[j].ch;
                ss << (std::isprint(ascii) ? ascii : '.');
            }
            ImGui::SameLine();
            std::string asciiString = ss.str();
            ImGui::Text("%s", asciiString.c_str());
            ss.str("");
            ss.clear();
        }

        /* Memory display ended */

        // Don't forget to PopID()
        ImGui::PopID();
    }
    
    // ImGui::SameLine();
    // if (ImGui::Button("Row >"))
    // {
    //     // A page is 32 rows, so cannot scroll past that
    //     if (initialAddress <= bufferSize - 33 * 16)    
    //     {
    //         initialAddress += 0x10;
    //     }
    // }
    ImGui::SameLine();
    if (ImGui::Button("Page >"))
    {
        /* 
            A page is 32 rows, so cannot scroll past that

            For example, the last page is from 0x1fe00 to 0x1fff0,
            so initialAddress cannot go over 0x1fe00 (bufferSize which is 0x20000 - 0x200)
        */
        uint32_t nextPageInitialAddress = initialAddress + 0x200;
        if (nextPageInitialAddress > bufferSize - 0x200)
        {
            initialAddress = bufferSize - 0x200;
        }
        else
        {
            initialAddress = nextPageInitialAddress;
        }
        // if (initialAddress <= bufferSize - 33 * 16)    
        // {
        //     initialAddress += 32 * 0x10;
        // }
        // else
        // {
        //     initialAddress = (uint32_t)(bufferSize - 32 * 16);
        // }
    }
    ImGui::SameLine();
    if (ImGui::Button("Page >>"))
    {
        initialAddress = (uint32_t)(bufferSize - 32 * 16);
    }

    ImGui::PopStyleColor();

    /* 
        Adding a TextInput to accept user input of initial address
        - Can only accept hexical numerical values
        - 5 digits (0040, 1FF00) maximum, should be capped to 0x1FFFE 9which is 0xFFFF * 2)
    */
    char userInitalAddress[6] = {0};
    // uint64_t newInitialAddress = 0; 

    ImGui::PushItemWidth(200);
    if (ImGui::InputText("Enter Address: 0x", userInitalAddress, IM_ARRAYSIZE(userInitalAddress), ImGuiInputTextFlags_CharsHexadecimal))
    {
        addressInputMode = true;
        // Just for debugging
        ImGui::Text("Address inputed: %s", userInitalAddress);
        // ImGui::Text("Address converted: %ld", Char_Array_to_Number(userInitalAddress, 5));
        // ImGui::Text("addressInputMode: %d", addressInputMode);

        uint64_t tempInitialAddress = Char_Array_to_Number(userInitalAddress, 5);

        /* First, we align tempInitialAddress to the nearest 16-byte */
        tempInitialAddress = (tempInitialAddress / 16) * 16;

        /* Then, we check whether we can show 32 rows starting with tempInitialAddress */
        if (tempInitialAddress > bufferSize - 33 * 16)    
        {
            tempInitialAddress = bufferSize - 33 * 16;
        }
        // else
        // {
        //     tempInitialAddress = (uint64_t)(bufferSize - 33 * 16);
        // }
        ImGui::Text("Address converted: 0x%07x", tempInitialAddress);
        // initialAddress = tempInitialAddress;
    }
    ImGui::PopItemWidth();
    

    /* If we are in Editor Mode, popup the editor window */

    if (editorMode)
    {
        // right now editorMode can only be cancelled by pressing ESC
        if (mousePos.x == -1)
        {
            mousePos = ImGui::GetMousePos();
        }
    }

    if (editorMode)
    {
        // This part needs to be separated from the previous if(editorMode) block
        // I haven't figured out why yet
        char buf = 0;
        char original = buffer[memoryEditedIndex].ch;
        Editor(mousePos, &buf, original);
        // dump the new value (or the original if the new value is not proper) back to the memory
        buffer[memoryEditedIndex].ch = buf;
        // Release memoryEditedIndexLocked for next edit
        memoryEditedIndexLocked = false;
    }

    // ImGui::PopFont();
    ImGui::End();
}

unsigned char LC3VMMemorywindow::Calculate_Char(char buf[], char original)
{
    if (!buf)
    {
        return original;
    }
    /* From char[3], use the first two indices to calculate the value */
    // 2a -> 2 is c1 and a is c0
    char c1 = buf[0];
    char c0 = buf[1];
    unsigned char result;

    if (c1 >= '0' && c1 <= '9')
    {
        result = (unsigned char)((c1 - '0') * 16);
    }
    else if (c1 >= 'A' && c1 <= 'F')
    {
        result = (unsigned char)((c1 - 'A' + 10) * 16);
    }
    else if (c1 >= 'a' && c1 <= 'f')
    {
        result = (unsigned char)((c1 - 'a' + 10) * 16);
    }
    else 
    {
        return original;
    }

    if (c0 >= '0' && c0 <= '9')
    {
        result += (unsigned char)(c0 - '0');
    }
    else if (c0 >= 'A' && c0 <= 'F')
    {
        result += (unsigned char)(c0 - 'A' + 10);
    }
    else if (c0 >= 'a' && c0 <= 'f')
    {
        result += (unsigned char)(c0 - 'a' + 10);
    }
    else 
    {
        return original;
    }
    return result;
}

void LC3VMMemorywindow::Editor(ImVec2 mousePos, char* c, char original)
{
    std::stringstream ss;
    ImGui::SetNextWindowPos(mousePos);
    ImGui::SetNextWindowSizeConstraints({192, 80}, {192, 80});
    if (!ImGui::Begin("Memory Editor", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
    {
        ImGui::End();
    }
    else
    {
        ss << "0x" << std::hex << std::setfill('0') << std::setw(4) << static_cast<int>(memoryEditedIndex);
        std::string inputLabel = ss.str();

        // Need 3 chars to capture 2 chars + '\0'
        char newValue[3] = {0};
        // printf("%d\n", memoryEditedIndex);
        /* 
            TODO: 
            ! somehow the label part doesn't work, maybe memoryEditedIndex is not correctly captured
            ! probably need to prevent window gone by mouse clicking (done by using ImGuiWindowFlags_NoBringToFrontOnFocus)
            ! need to figure out how to use backspace to remove char
            ! need to figure out how to close editor window when ESCAPE is pressed
            ! need to figure out how to extract the first two chars and convert that to a char and dump back into buffer[inputLabel]
        */
        if (
            ImGui::InputText(
                inputLabel.c_str(), newValue, IM_ARRAYSIZE(newValue), ImGuiInputTextFlags_CharsHexadecimal
            )
        )
        {
            ImGui::Text("%s", newValue);
            ImGui::SameLine();
            // Just for debugging
            ImGui::Text("%d", Calculate_Char(newValue, original));
        }
        *c = Calculate_Char(newValue, original);
        ImGui::End();
    }
}

uint64_t LC3VMMemorywindow::Char_Array_to_Number(char buf[], size_t numDigits)
{
    // Debug
    printf("Hex buffer: %s\n", buf);
    printf("First character: %c\n", buf[0]);

    /* Cap the number of hex digits to 4, i.e. 4 bytes */
    if (numDigits > 4)
    {
        numDigits = 4;
    }

    uint64_t result = 0;
    int power = 0;

    int i = numDigits - 1;
    /* Find the first (highest) byte that is not NULL */
    while (buf[i] == '\0')
    {
        i -= 1;
    }
    printf("i is %d\n", i);

    for (; i >= 0; i--)
    {
        uint64_t value = Char_to_Number(buf[i]);
        if (value == 0xff)
        {
            // Complain, must be 0-9, a-f or A-F, but do not exit
            fprintf(stderr, "Error reading hex buffer: char is %c\n", buf[i]);
            // exit(ERROR_VALUE);
            value = 0;
        }
        result += value * (pow(16, power++));
    }

    return result;
}

uint64_t LC3VMMemorywindow::Char_to_Number(char ch)
{
    if (ch >= '0' && ch <= '9')
    {
        return (uint64_t)(ch - '0');
    }
    else if (ch >= 'A' && ch <= 'F')
    {
        return (uint64_t)(ch - 'A' + 10);
    }
    else if (ch >= 'a' && ch <= 'f')
    {
        return (uint64_t)(ch - 'a' + 10);
    }
    else 
    {
        return 0xff;
    }
}