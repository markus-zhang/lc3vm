#include "lc3vmwin_quit_confirm.hpp"

void Quit_Confirm(bool* isRunning)
{

    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    // Shown in the middle of 1920*1080 window
    ImVec2 size = {256, 80};
    ImGui::SetNextWindowPos({(1920 - size.x) / 2, (1080 - size.y) / 2});
    ImGui::SetNextWindowSizeConstraints(size, size);

    if (!ImGui::Begin("Confirm", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
    {
        ImGui::End();
        *isRunning = true;
    }
    else
    {
        ImGui::Text("Confirm to quit?");
        ImGui::SameLine();
        if (ImGui::Button("Yes"))
        {
            ImGui::End();
            *isRunning = false;
            return;
        }
        ImGui::SameLine();
        if (ImGui::Button("No"))
        {
            ImGui::End();
            *isRunning = true;
            return;
        }
    }
    ImGui::End();
    // *isRunning = true;
}