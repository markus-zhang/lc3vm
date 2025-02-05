#include "lc3vmwin_disa.hpp"


std::string (*disa_call_table[])(uint16_t, uint16_t) = {
	&dis_br, &dis_add, &dis_ld, &dis_st, &dis_jsr, &dis_and, &dis_ldr, &dis_str, 
	&dis_rti, &dis_not, &dis_ldi, &dis_sti, &dis_jmp, &dis_rsv, &dis_lea, &dis_trap
};

LC3VMdisawindow::LC3VMdisawindow()
{
    initialAddress = 0x0000;
    numInstructions = 0;
    instructionStream.reserve(0);
    selection.reserve(0);

    initialWindowSize = {0, 0};
    minWindowSize = {0, 0};
    winPos = {0, 0};

    font = nullptr;
    initialized = false;
}

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

void LC3VMdisawindow::Load_Config(const WindowConfig& config)
{
    initialWindowSize = config.initialWindowSize;
    minWindowSize = config.minWindowSize;
    winPos = config.winPos;
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

    ImGui::Text("Address\t");
    ImGui::SameLine();
    ImGui::Text("Instruction\t");

    for (int i = 0; i < numInstructions; i++)
    {
        u_int16_t instr = instructionStream[i];
        ImGui::Text("%#06x\t", initialAddress + i * 2); // each instr is 2 bytes
        ImGui::SameLine();
        ImGui::Text("%#06x\t", instr);
        ImGui::SameLine();
        std::string disaOutput = disa_call_table[instr >> 12](instr, initialAddress);
        ImGui::Text("%s", disaOutput.c_str());
    }

    ImGui::End();
}