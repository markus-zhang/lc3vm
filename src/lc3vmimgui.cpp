#include "lc3vmwin_memory.hpp"
#include "lc3vmwin_quit_confirm.hpp"
#include "lc3vmwin_disa.hpp"
#include "lc3vmwin_loader.hpp"

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
    unsigned char text[] = "aaabcdefghijklmnopqrstuvwxyz";
    WindowConfig memoryWinConfig {true, 20, {848, 672}, {848, 672}, {0, 0}};
    LC3VMMemorywindow memoryWindow = LC3VMMemorywindow(text, 640, memoryWinConfig);
    
    uint16_t fakeInstr[100];
    for (int i = 0; i < 100; i++)
    {
        fakeInstr[i] = 0x12ef;
    }
    WindowConfig disaWinConfig {true, 20, {320, 480}, {320, 480}, {1024, 0}};
    LC3VMdisawindow disaWindow = LC3VMdisawindow(fakeInstr, 100, 0x3000, disaWinConfig);

    bool signalQuit = false;
    bool isRunning = true;
    bool isDebug = false;
    bool isDisa = false;

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
                    signalQuit = false;
                    break;
                }
                case SDL_KEYDOWN:
                {
                    if (sdlEvent.key.keysym.sym == SDLK_ESCAPE)
                    {
                        // When editing, esc is used to close the editor mini window
                        // as editorMode is the controlling boolean
                        if (!memoryWindow.editorMode)
                        {
                            signalQuit = true;
                        }
                        else
                        {
                            // mouseDoubleClicked needs to set false, otherwise it keeps opening the editor window
                            memoryWindow.editorMode = false;
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
                    else if (sdlEvent.key.keysym.sym == SDLK_g)
                    {
                        // TODO: This is probably not a great way to disable certain keys
                        // Just imagine what happens if we have a dozen of such switches
                        if (!memoryWindow.editorMode)
                        {
                            isDisa = !isDisa;
                        }
                    }
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 21, 21, 21, 255);
        SDL_RenderClear(renderer);

        // ImGui part BEGIN --------------------------------------------------
        /*
            One important part is that all ImGui windows need to be within the same NewFrame(),
            otherwise weird shits happen - e.g. mouse doesn't work on any of the windows somehow
        */
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        if (isDebug)
        {
            memoryWindow.Draw();
        }

        if (isDisa)
        {
            disaWindow.Draw();
        }

        if (signalQuit)
        {
            signalQuit = Quit_Confirm(&isRunning);
        }

        ImGui::Render();
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
        // ImGui part END ----------------------------------------------------

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