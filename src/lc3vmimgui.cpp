#include "lc3vmwin_memory.hpp"
#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <imgui/imgui.h>
// #include <imgui/imgui_sdl.h>
#include <imgui/imgui_impl_sdl2.h>
#include <imgui/imgui_impl_sdlrenderer2.h>

int main()
{
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_DisplayMode displayMode;
    if (SDL_GetCurrentDisplayMode(0, &displayMode) != 0) {
        std::cerr << "SDL_GetCurrentDisplayMode Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "ImGui Test",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        // displayMode.w,
        // displayMode.h,
        1920,
        1080,
        SDL_WINDOW_SHOWN
    );

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

    // Initialize the ImGui context
    ImGui::CreateContext();
    // ImGuiSDL::Initialize(renderer, 1440, 900);
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    // Memory Window
    char text[] = "abcdefghijklmnopqrstuvwxyz";
    WindowConfig winConfig {true, 20, {1024, 768}, {800, 600}, {0, 0}};
    LC3VMMemorywindow memoryWindow = LC3VMMemorywindow(text, 640, winConfig);

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
                        // When editing, esc is used to close the editor mini window
                        if (!memoryWindow.editorMode)
                        {
                            isRunning = false;
                        }
                    }
                    else if (sdlEvent.key.keysym.sym == SDLK_d)
                    {
                        // We don't want the whole debug window closed when user types 'd', do we?
                        if (!memoryWindow.editorMode)
                        {
                            isDebug = !isDebug;
                        }
                    }
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 21, 21, 21, 255);
        SDL_RenderClear(renderer);
        // ImGui part
        if (isDebug)
        {
            ImGui_ImplSDLRenderer2_NewFrame();
            ImGui_ImplSDL2_NewFrame();
            ImGui::NewFrame();
            memoryWindow.Draw();


            // if (!windowInitialized)
            // {
            //     ImGui::SetNextWindowSize(initialWindowSize, 0);
            //     windowInitialized = true;
            // }
            // ImGui::GetStyle().WindowBorderSize = 2.0f;
            // ImGui::SetNextWindowPos({0, 0});
            // ImGui::SetNextWindowSizeConstraints(minWindowSize, initialWindowSize);

            // ImGui::Begin(
            //     "ImGui Test Window 1", 
            //     nullptr, 
            //     // ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse
            //     ImGuiWindowFlags_NoCollapse
            // );
            // // ImGui::NewFrame();
            // ImGui::ShowDemoWindow();

            // // ImGui widgets begin
            // // ImGui::SetCursorPos({0, 0});
            // ImVec2 currentSize = ImGui::GetWindowSize();

            // ImGui::PushItemWidth(currentSize.x);
            // ImGui::Separator();
            // ImGui::PopItemWidth();
            // ImGui::Button("This is a button", {256, 32});
            // ImGui::SetCursorPos({0, 480});
            // ImGui::LabelText("How to set the size?", nullptr);
            
            // // if (currentSize.x < 640)
            // // {
            // //     ImGui::SetWindowSize({640, currentSize.y});
            // // }
            // // if (currentSize.y < 480)
            // // {
            // //     ImGui::SetWindowSize({currentSize.x, 480});
            // // }
            // ImGui::End();
            // // ImGui widgets end

            // ImGui::SetNextWindowSize({240, 600}, 0);
            // ImGui::GetStyle().WindowBorderSize = 2.0f;
            // ImGui::SetNextWindowPos({1200, 0});

            // ImGui::Begin(
            //     "ImGui Test Window 2", 
            //     nullptr, 
            //     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)
            // ;
            // ImGui::End();

            ImGui::Render();
            // ImGuiSDL::Render(ImGui::GetDrawData());
            ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
        }

        // Draw a rectangle for test
        // SDL_Rect rect = {16, 16, 256, 256};
        // SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        // SDL_RenderDrawRect(renderer, &rect);
        // SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        // SDL_RenderFillRect(renderer, &rect);

        SDL_RenderPresent(renderer);
    }

    // ImGuiSDL::Deinitialize();
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}