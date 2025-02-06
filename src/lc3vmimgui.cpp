#include "globals.hpp"
#include "lc3vmwin_memory.hpp"
#include "lc3vmwin_quit_confirm.hpp"
#include "lc3vmwin_disa.hpp"
#include "lc3vmwin_loader.hpp"
#include "lc3vmwin_cache.hpp"

#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <imgui.h>
// #include <imgui/imgui_sdl.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>

/* ------- function declarations begin -------- */
// Initialization
int init();

// SDL2 and ImGui frame
void input();
void sdl_imgui_frame();

// interpreter run function
void interpreter_run();
void run();
void shutdown();
void cache_run(struct lc3Cache cache);
void cache_dump(int cacheIndex);

// lc-3 instruction functions
void op_br(uint16_t instr);
void op_add(uint16_t instr);
void op_ld(uint16_t instr);
void op_st(uint16_t instr);
void op_jsr(uint16_t instr);
void op_and(uint16_t instr);
void op_ldr(uint16_t instr);
void op_str(uint16_t instr);
void op_rti(uint16_t instr);
void op_not(uint16_t instr);
void op_ldi(uint16_t instr);
void op_sti(uint16_t instr);
void op_jmp(uint16_t instr);
void op_res(uint16_t instr);
void op_lea(uint16_t instr);
void op_trap(uint16_t instr);

// trap functions
void trap_0x20();
void trap_0x21();
void trap_0x22();
void trap_0x23();
void trap_0x24();
void trap_0x25();

/* ------- function declarations end --------*/

/* Global variables BEGIN -------------------------------------*/
uint8_t DEBUG_MODE = DEBUG_DIS;

// LC-3 specific BEGIN ------------------------------------------
enum
{
	R_R0 = 0, R_R1, R_R2, R_R3, R_R4, R_R5, R_R6, R_R7,
	R_PC, 
	R_COND, 
	R_COUNT
};

// Condition codes are supposed to be in R[COND]'s bit-0/1/2, to be used in BR
enum
{
	FL_POS = 1 << 0,	// P
	FL_ZRO = 1 << 1,	// Z
	FL_NEG = 1 << 2		// N
};

enum
{
    MR_KBSR = 0xFE00, /* keyboard status */
    MR_KBDR = 0xFE02  /* keyboard data */
};

// LC-3 specific END --------------------------------------------

uint16_t buffer[MAX_SIZE] = {0};
uint8_t running = 1;
// struct termios original_tio;

void (*instr_call_table[])(uint16_t) = {
	&op_br, &op_add, &op_ld, &op_st, &op_jsr, &op_and, &op_ldr, &op_str, 
	&op_rti, &op_not, &op_ldi, &op_sti, &op_jmp, &op_res, &op_lea, &op_trap
};

/* Global variables owned by the VM */
SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
LC3VMMemorywindow memoryWindow;
LC3VMdisawindow disaWindow;

// Registers
uint16_t reg[R_COUNT];
// RAM
uint8_t memory[(MAX_SIZE) * 2] = {0};

bool signalQuit;
bool isRunning;
bool isDebug;
bool isDisa;

int main()
{
    /* -------------------Loading LC-3 binary into memory---------------------- */
    int init_code = init();
    if (init_code != 0)
    {
        return init_code;
    }

    interpreter_run();

    shutdown();
}

/* function definitions */

int init()
{
    reg[R_COND] = FL_ZRO;
	reg[R_PC] = 0x3000;

    FILE* fp = fopen("./2048.obj", "rb");
    if (!fp)
    {
        std::cerr << "Failed to read file" << std::endl;
        exit(ERROR_LOADFILE);
    }
	uint16_t numInstr = load_memory(buffer, memory, fp);
    fclose(fp);

    /* --------------------------------Loading End----------------------------- */
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_DisplayMode displayMode;
    if (SDL_GetCurrentDisplayMode(0, &displayMode) != 0) {
        std::cerr << "SDL_GetCurrentDisplayMode Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return ERROR_VM_INIT_FAIL;
    }

    window = SDL_CreateWindow(
        "ImGui Test",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        // displayMode.w,
        // displayMode.h,
        1920,
        1080,
        SDL_WINDOW_SHOWN
    );

    renderer = SDL_CreateRenderer(window, -1, 0);

    // Initialize the ImGui context
    ImGui::CreateContext();
    // ImGuiSDL::Initialize(renderer, 1440, 900);
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    // Memory Window
    WindowConfig memoryWinConfig {true, 20, {848, 672}, {848, 672}, {0, 0}};
    memoryWindow = LC3VMMemorywindow(memory, (unsigned long)(MAX_SIZE * 2), memoryWinConfig);
    
    // Insturction Cache Window
    WindowConfig disaWinConfig {true, 20, {360, 480}, {360, 480}, {1024, 0}};
    disaWindow.Load_Config(disaWinConfig);

    signalQuit = false;
    isRunning = true;
    isDebug = false;
    isDisa = false;

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

    SDL_RenderPresent(renderer);
}

void run()
{
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

        SDL_RenderPresent(renderer);
    }
}

void shutdown()
{
    // ImGuiSDL::Deinitialize();
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void interpreter_run()
{
	while (isRunning)
	{
        input();

		uint16_t lc3Address = reg[R_PC];
		int cacheIndex = cache_find(lc3Address);

		// if cache not found, then build and insert
		if (cacheIndex == -1)
		{
			struct lc3Cache newCache = cache_create_block(memory, lc3Address);
			int newCacheIndex = cacheCount;
			cache_add(newCache);

			if (DEBUG_MODE == DEBUG_DIS)
			{
				// FILE* dump = fopen("cache_dump.txt", "a");
				cache_dump(newCacheIndex);
                // It is required to run sdl_imgui_frame() just for visually check the code
                // The code is loaded into the disa window but we must draw it on screen
                sdl_imgui_frame();
			}

			// cache_run(codeCache[newCacheIndex]);
		}
		// if found, then execute
		else
		{
			// cache_run(codeCache[cacheIndex]);
            sdl_imgui_frame();
		}
	}
}

void cache_run(struct lc3Cache cache)
{
	/*
		cache_run is different from interpreter_run in the sense
			-> that we don't use PC to find the next instruction but just run sequentially inside of the cache
			-> We still need to update the PC for the next interpreter_run() call
	*/
	for (int i = 0; i < cache.numInstr; i++)
	{
		uint16_t instr = cache.codeBlock[i];	
		uint16_t op = instr >> 12;

		// Debuggin BEGIN
		// ui_debug_info(reg, 25);
		// Debugging END

		/* LC-3 PC++ */
		reg[R_PC] += 1;	
		
        // TODO: turn this on whence the op functions are implemented
        // instr_call_table[op](instr);

        sdl_imgui_frame();
	}

}

void cache_dump(int cacheIndex)
{

	printf("--------Dumping Cache No. %d BEGIN--------\n", cacheIndex);
	if (cacheIndex >= cacheCount)
	{
		printf("Wrong cache index at: %d\n", cacheIndex);
	}
	else
	{
		// for (int i = 0; i < codeCache[cacheIndex].numInstr; i++)
		// {
		// 	uint16_t instr = codeCache[cacheIndex].codeBlock[i];	
		// 	uint16_t op = instr >> 12;
		// 	/* Call the dispatch fp */
		// 	disa_call_table[op](instr, 0x3000 + i * 2);
		// }
        disaWindow.Load(codeCache[cacheIndex].codeBlock, codeCache[cacheIndex].numInstr, codeCache[cacheIndex].lc3MemAddress);
	}
	printf("--------Dumping Cache No. %d END----------\n", cacheIndex);
	// Pause for inspection
	// fflush(stdout);
	// getchar();
}

/* Op code functions */

void op_br(uint16_t instr)
{

}

void op_add(uint16_t instr)
{

}

void op_ld(uint16_t instr)
{

}

void op_st(uint16_t instr)
{

}

void op_jsr(uint16_t instr)
{

}

void op_and(uint16_t instr)
{

}

void op_ldr(uint16_t instr)
{

}

void op_str(uint16_t instr)
{

}

void op_rti(uint16_t instr)
{

}

void op_not(uint16_t instr)
{

}

void op_ldi(uint16_t instr)
{

}

void op_sti(uint16_t instr)
{

}

void op_jmp(uint16_t instr)
{

}

void op_res(uint16_t instr)
{

}

void op_lea(uint16_t instr)
{

}

void op_trap(uint16_t instr)
{

}