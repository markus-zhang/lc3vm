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
    stepInSignal = false;
    stepInLine = 0;
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
        // printf("Initial Address: %d\n", initialAddress);
        u_int16_t instr = instructionStream[i];
        if (i == stepInLine)
        {
            ImGui::Text(">>");
            ImGui::SameLine();
        }
        // EXPLAIN: Why + i, not + i*2? After all each instr is 2 bytes
        // The reason is, the VM uses 16-bit addressing so each "1" address has 2 bytes
        // This is different from the memory window, which uses the standard byte memory addressing
        // This is also why the address will never execeed 0xFFFF, while the memory window needs double the address space
        ImGui::Text("%#06x\t", initialAddress + i); 
        ImGui::SameLine();
        ImGui::Text("%#06x\t", instr);
        ImGui::SameLine();
        std::string disaOutput = disa_call_table[instr >> 12](instr, initialAddress);
        ImGui::Text("%s", disaOutput.c_str());
    }

    /* Add a Continue button to let the code run */
    if (ImGui::Button("Execution/Step-in"))
    {
        stepInSignal = !stepInSignal;
    }

    ImGui::End();
}