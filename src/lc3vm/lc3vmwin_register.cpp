#include "lc3vmwin_register.hpp"
#include <assert.h>
#include <sstream>
#include <iomanip>


LC3VMRegisterWindow::LC3VMRegisterWindow(int externalRegNum, const void* externalRegFile, const std::vector<std::string>& externalRegNames, int externalRegSize, int exeternalNumRegShownEachLine, const WindowConfig& config)
{
    /* Gracefully disable the window if regNum = 0 */
    if (externalRegNum <= 0 || externalRegSize <= 0 || exeternalNumRegShownEachLine <= 0)
    {
        fprintf(stderr, "Function: %s gracely shows you an error message: regNum and regSize and numRegShownEachLine should be natural numbers. \n", __func__);
        disabled = true;
        regNum = 0;
        numRegShownEachLine = 0;
    }
    else
    {
        disabled = false;
        regNum = externalRegNum;
        regNames.reserve(regNum);

        for (int i = 0; i < regNum; i++)
        {
            regNames.push_back(externalRegNames[i]);
        }

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
        numRegShownEachLine = exeternalNumRegShownEachLine;

        
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
}

void LC3VMRegisterWindow::Draw()
{
    if (disabled)
    {
        printf("Register window disabled\n");
        return;
    }

    if (!initialized)
    {
        ImGui::SetNextWindowSize(initialWindowSize, 0);
        initialized = true;
    }

    ImGui::GetStyle().WindowBorderSize = 2.0f;
    ImGui::SetNextWindowPos(winPos);
    ImGui::SetNextWindowSizeConstraints(minWindowSize, initialWindowSize);

    ImGui::Begin(
        "Register Watch Window",
        nullptr,
        ImGuiWindowFlags_NoCollapse
    );

    /*
        Left border: twenty pixels
    */
    ImGui::SetCursorPos({20.0f, 10.0f});
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));
    
    int i = 0;
    int j = 0;
    std::stringstream ss;
    
    while (i < regNum)
    {
        ImGui::SetCursorPosX(20.0f);
        ImGui::SameLine();

        ImGui::Text("%s:", regNames[i].c_str());
        ImGui::SameLine();

        int regSize = rf.regSize;
        switch (regSize)
        {   
            case 1:
            {
                ss << "0x" << std::hex << std::setfill('0') << std::setw(regSize) << (uint8_t)(rf.u.p8bit[i]) << " ";
                break;
            }
            case 2:
            {
                ss << "0x" << std::hex << std::setfill('0') << std::setw(regSize) << (uint16_t)(rf.u.p16bit[i]) << " ";
                break;
            }
            case 4:
            {
                ss << "0x" << std::hex << std::setfill('0') << std::setw(regSize) << (uint32_t)(rf.u.p32bit[i]) << " ";
                break;
            }
            case 8:
            {
                ss << "0x" << std::hex << std::setfill('0') << std::setw(regSize) << (uint64_t)(rf.u.p64bit[i]) << " ";
                break;
            }
        }
        std::string regWidget = ss.str();
        ImGui::Text("%s", regWidget.c_str());
        ss.str("");
        ss.clear();

        i += 1;
        ImGui::NewLine();
    }

    ImGui::PopStyleColor();
    ImGui::End();
}