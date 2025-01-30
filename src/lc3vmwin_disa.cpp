#include "lc3vmwin_disa.hpp"

LC3VMdisawindow::LC3VMdisawindow(uint16_t instrStream[], uint16_t numInstr, uint16_t address, const WindowConfig& config)
{
    initialAddress = address;
    numInstructions = numInstr;

    instructionStream = std::vector<uint16_t>(instrStream, instrStream + numInstr);

    selection = {0};

    initialWindowSize = config.initialWindowSize;
    minWindowSize = config.minWindowSize;
    winPos = config.winPos;

    ImGuiIO& io = ImGui::GetIO();
    ImFontConfig fontConfig;
    fontConfig.SizePixels = 14.0f;
    font = io.Fonts->AddFontDefault(&fontConfig);
    assert(font != nullptr);

    initialized = false;
}

void LC3VMdisawindow::Load(uint16_t instrStream[], uint16_t numInstr, uint16_t address)
{
    instructionStream = std::vector<uint16_t>(instrStream, instrStream + numInstr);
    initialAddress = address;
    numInstructions = numInstr;
}

void LC3VMdisawindow::Draw(void)
{
    /*
        It should show a row of following items for each instruction (uint16_t):

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
        "Disassembly Window",
        nullptr,
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBringToFrontOnFocus
    );

    for (auto instr : instructionStream)
    {
        ImGui::Text("%hu#06x", instr);
        // ImGui::SameLine();
    }

    ImGui::End();
}