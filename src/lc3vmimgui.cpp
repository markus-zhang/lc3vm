#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <imgui/imgui.h>
#include <imgui/imgui_sdl.h>
#include <imgui/imgui_impl_sdl.h>

int main()
{
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window* window = SDL_CreateWindow(
        "ImGui Test",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1440,
        900,
        SDL_WINDOW_SHOWN
    );

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

    // Initialize the ImGui context
    ImGui::CreateContext();
    ImGuiSDL::Initialize(renderer, 1440, 900);

    bool isRunning = true;
    bool isDebug = false;

    while(isRunning)
    {
        // Process Input
        SDL_Event sdlEvent;
        while (SDL_PollEvent(&sdlEvent))
        {
            // Handle ImGui SDL input
            ImGui_ImplSDL2_ProcessEvent(&sdlEvent);
            ImGuiIO& io = ImGui::GetIO();

            int mouseX, mouseY;

            const int buttons = SDL_GetMouseState(&mouseX, &mouseY);
            io.MousePos = ImVec2(mouseX, mouseY);
            io.MouseDown[0] = buttons & SDL_BUTTON(SDL_BUTTON_LEFT);
            io.MouseDown[1] = buttons & SDL_BUTTON(SDL_BUTTON_RIGHT);

            // Handle core SDL events
            switch (sdlEvent.type)
            {
                case SDL_QUIT:
                {
                    isRunning = false;
                    break;
                }
                case SDL_KEYDOWN:
                {
                    if (sdlEvent.key.keysym.sym == SDLK_ESCAPE)
                    {
                        isRunning = false;
                    }
                    else if (sdlEvent.key.keysym.sym == SDLK_d)
                    {
                        isDebug = !isDebug;
                    }
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 21, 21, 21, 255);
        SDL_RenderClear(renderer);
        // ImGui part
        if (isDebug)
        {
            ImGui::NewFrame();

            ImGui::SetNextWindowSize({800, 600}, 0);
            ImGui::GetStyle().WindowBorderSize = 2.0f;
            ImGui::SetNextWindowPos({0, 0});

            ImGui::Begin(
                "ImGui Test Window 1", 
                nullptr, 
                ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)
            ;
            // ImGui::NewFrame();
            // ImGui::ShowDemoWindow();

            // ImGui widgets begin
            // ImGui::SetCursorPos({0, 0});
            
            ImGui::PushItemWidth(800.0f);
            ImGui::Separator();
            ImGui::PopItemWidth();
            ImGui::Button("This is a button", {256, 32});
            ImGui::SetCursorPos({0, 480});
            ImGui::LabelText("How to set the size?", nullptr);
            ImGui::End();
            // ImGui widgets end

            ImGui::SetNextWindowSize({240, 600}, 0);
            ImGui::GetStyle().WindowBorderSize = 2.0f;
            ImGui::SetNextWindowPos({1200, 0});

            ImGui::Begin(
                "ImGui Test Window 2", 
                nullptr, 
                ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)
            ;
            ImGui::End();

            ImGui::Render();
            ImGuiSDL::Render(ImGui::GetDrawData());
        }

        // Draw a rectangle for test
        // SDL_Rect rect = {16, 16, 256, 256};
        // SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        // SDL_RenderDrawRect(renderer, &rect);
        // SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        // SDL_RenderFillRect(renderer, &rect);

        SDL_RenderPresent(renderer);
    }

    ImGuiSDL::Deinitialize();
    ImGui::DestroyContext();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}