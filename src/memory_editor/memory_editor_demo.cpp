#include "memory_editor_globals.hpp"
#include "memory_editor.hpp"

#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>

#include <chrono>
#include <ctime>
#include <iomanip>
#include <string>

/* ------- function declarations begin -------- */
// Initialization
int init();

// SDL2 and ImGui frame
void input();
void sdl_imgui_frame();

// interpreter run function
void run();
void shutdown();

/* ------- function declarations end --------*/

char buffer[MAX_SIZE] = {0};
bool running = 1;


/* Global variables owned by the VM */
SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;

MemoryEditor me;

ImGuiTextBuffer consoleBuffer;

bool isRunning;


int main()
{
    /* -------------------Loading LC-3 binary into memory---------------------- */
    int init_code = init();
    if (init_code != 0)
    {
        return init_code;
    }

    run();

    shutdown();
}

/* function definitions */

int init()
{
    // FILE* fp = fopen("./2048.obj", "rb");
    // if (!fp)
    // {
    //     std::cerr << "Failed to read file" << std::endl;
    //     exit(1);
    // }
	// int64_t numInstr = load_memory(buffer, memory, fp);
    // fclose(fp);

    /* --------------------------------Loading End----------------------------- */
    SDL_Init(SDL_INIT_EVERYTHING);

	SDL_SetHint(SDL_HINT_RENDER_VSYNC, "0");

    SDL_DisplayMode displayMode;
    if (SDL_GetCurrentDisplayMode(0, &displayMode) != 0) {
        std::cerr << "SDL_GetCurrentDisplayMode Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return ERROR_VM_INIT_FAIL;
    }

    window = SDL_CreateWindow(
        "Memory Editor Demo",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Initialize the ImGui context
    ImGui::CreateContext();
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

	// FIXME: Just for testing memory editor, remove afterwards
	ImGuiWindowConfig memoryEditorConfig {true, 20, {864, 720}, {864, 720}, {0, 0}};
	uint8_t testString[] = "abcdefghijklmnopqrstuvmxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	me = MemoryEditor(testString, 53, memoryEditorConfig);

    isRunning = true;

    return 0;
}

void input()
{
    // Process Input
    SDL_Event sdlEvent;
    while (SDL_PollEvent(&sdlEvent))
    {
        // Handle ImGui SDL input
        ImGui_ImplSDL2_ProcessEvent(&sdlEvent);
        ImGuiIO& io = ImGui::GetIO();

        // ImGui has priority if user is interacting with it
        if (io.WantCaptureKeyboard)
        {
            continue;
        }

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
                break;
            }
			default:
				break;
        }
    }
}

void sdl_imgui_frame()
{
    /* ----------------------- RENDERING PART -------------------------- */

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

	me.Draw();

    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
    
    // ImGui part END ----------------------------------------------------

    SDL_RenderPresent(renderer);
}

void shutdown()
{
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void run()
{   
    Uint32 startTime = SDL_GetTicks();

	while (isRunning)
	{
        input();

        Uint32 now = SDL_GetTicks();
		// EXPLAIN: Cap rendering to 60 fps, otherwise the program lags too much
		if (now - startTime >= MSPF)
		{
        	sdl_imgui_frame();
			startTime = now;
		}
    }
}

