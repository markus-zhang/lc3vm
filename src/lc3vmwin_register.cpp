#include "lc3vmwin_register.hpp"
#include <assert.h>
#include <sstream>
#include <iomanip>

LC3VMRegisterWindow::LC3VMRegisterWindow(int externalRegNum, const void* externalRegFile, const char** externalRegNames, int externalRegSize, int exeternalNumRegShownEachLine, const WindowConfig& config)
{
    /* Gracefully disable the window if regNum = 0 */
    if (externalRegNum <= 0 || externalRegSize <= 0 || exeternalNumRegShownEachLine <= 0)
    {
        fprintf(stderr, "Function: %s gracely shows you an error message: regNum and regSize and numRegShownEachLine should be natural numbers. \n", __func__);
        disabled = true;
        regNames = nullptr;
        regNum = 0;
        numRegShownEachLine = 0;
    }
    else
    {
        if (regNum > sizeof(externalRegFile) / externalRegSize)
        {
            fprintf(stderr, "Function: %s gracely shows you an error message: regNum should be equal or smaller than # of elements in regFile. \n", __func__);
            regNum = sizeof(externalRegFile) / externalRegSize;
        }
        if (sizeof(externalRegFile) % externalRegSize != 0)
        {
            fprintf(stderr, "Function: %s gracely shows you an error message: number of elements in regFile (%d) should divide regSize (%d). May lose some data at the end. \n", __func__, sizeof(externalRegFile), externalRegSize);
        }
        disabled = false;
        regNum = regNum;
        regNames = regNames;

        rf.regSize = externalRegSize;
        switch (externalRegSize)
        {
            case 1:
            {
                rf.u.p8bit = (uint8_t*)externalRegFile;
                break;
            }
            case 2:
            {
                rf.u.p16bit = (uint16_t*)externalRegFile;
                break;
            }
            case 4:
            {
                rf.u.p32bit = (uint32_t*)externalRegFile;
                break;
            }
            case 8:
            {
                rf.u.p64bit = (uint64_t*)externalRegFile;
                break;
            }
            default:
            {
                fprintf(stderr, "Register size other than 1, 2, 4 and 8 bytes are not implemented -> this widget is not disabled\n");
                disabled = true;
                break;
            }
        }
        numRegShownEachLine = numRegShownEachLine;
    }
    this->initialWindowSize   = config.initialWindowSize;
    this->minWindowSize       = config.minWindowSize;
    this->winPos              = config.winPos;

    this->initialized = false;
}

LC3VMRegisterWindow::~LC3VMRegisterWindow()
{
    rf.u.p8bit = nullptr;
    rf.u.p16bit = nullptr;
    rf.u.p32bit = nullptr;
    rf.u.p64bit = nullptr;
    regNames = nullptr;
}

void LC3VMRegisterWindow::Draw()
{

    if (!initialized)
    {
        ImGui::SetNextWindowSize(initialWindowSize, 0);
        initialized = true;
    }

    /*
        How much space does one register take? Depending on how many bytes we want to show:
        0x0030 and 0x0000000000000030 definitely are different!
    */
    std::string text = "R0:0x";
    for (int i = 0; i < rf.regSize; i++)
    {
        text.push_back('0');
    }
    text.push_back('\t');
    /* Now we have a string like "R0:0x0000\t" to measure the length */
    ImVec2 textSize = ImGui::CalcTextSize(text.c_str());

    /* If window is not big enough (adding 40 pixels as buffer), reset window */
    if (initialWindowSize.x <= numRegShownEachLine * textSize.x + 40)
    {
        initialWindowSize.x = numRegShownEachLine * textSize.x + 40;
    }

    ImGui::GetStyle().WindowBorderSize = 2.0f;
    ImGui::SetNextWindowPos(winPos);
    ImGui::SetNextWindowSizeConstraints(minWindowSize, initialWindowSize);

    ImGui::Begin(
        "Register Watch Window",
        nullptr,
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBringToFrontOnFocus
    );

    /*
        Left border: twenty pixels
    */
    ImGui::SetCursorPos({20.0f, 10.0f});
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));
    
    int i = 0;
    int j = 0;
    std::stringstream ss;
    
    while (i < sizeof(regNames))
    {
        if (i % numRegShownEachLine != 0)
        {
            ImGui::SameLine();
            ImGui::Text("\t");
            ImGui::SameLine();
            ImGui::Text(regNames[i++]);
            ImGui::Text(":");
            ImGui::SameLine();
            ss << "0x" << std::hex << std::setfill('0') << std::setw(regSizeInBytes) << (uint64_t)(regFile[i]) << " ";
            ImGui::
        }
        else 
        {
            ImGui::Text(regNames[i++]);
        }
        
    }

    // for (size_t i = 0; /* i < sizeof(regNames)*/  i < numRegShownEachLine; i++)
    // {
    //     ImGui::Text(regNames[i]);
    // }
}