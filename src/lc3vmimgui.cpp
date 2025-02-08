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
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>
#include <signal.h>
#include <termios.h>

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

uint16_t read_memory(uint16_t index);
uint16_t read_uint16_t(uint16_t index);
void write_memory(uint16_t index, uint16_t value);

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

void update_flag(uint16_t value);

// trap functions
void trap_0x20();
void trap_0x21();
void trap_0x22();
void trap_0x23();
void trap_0x24();
void trap_0x25();

/* ------- function declarations end --------*/

/* Global variables BEGIN -------------------------------------*/
uint8_t DEBUG_MODE = DEBUG_OFF;

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

void (*instr_call_table[])(uint16_t) = {
	&op_br, &op_add, &op_ld, &op_st, &op_jsr, &op_and, &op_ldr, &op_str, 
	&op_rti, &op_not, &op_ldi, &op_sti, &op_jmp, &op_res, &op_lea, &op_trap
};

/* Global variables owned by the VM */
SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
LC3VMMemorywindow memoryWindow;
LC3VMdisawindow disaWindow;
bool keyPressed;
uint8_t lastKeyPressed;
struct termios original_tio;

// Registers
uint16_t reg[R_COUNT];
// RAM
uint16_t memory[MAX_SIZE] = {0};

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
    keyPressed = false;
    lastKeyPressed = 0;

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
        1920,
        1080,
        SDL_WINDOW_SHOWN
    );

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    // Initialize the ImGui context
    ImGui::CreateContext();
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    // Memory Window
    WindowConfig memoryWinConfig {true, 20, {848, 672}, {848, 672}, {0, 0}};
    memoryWindow = LC3VMMemorywindow(memory, (size_t)(MAX_SIZE * 2), memoryWinConfig);
    
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
        // ImGuiIO& io = ImGui::GetIO();

        // int mouseX, mouseY;

        // const Uint32 buttons = SDL_GetMouseState(&mouseX, &mouseY);
        // io.MousePos = ImVec2(mouseX, mouseY);
        // io.MouseDown[0] = buttons & SDL_BUTTON(SDL_BUTTON_LEFT);
        // io.MouseDown[1] = buttons & SDL_BUTTON(SDL_BUTTON_RIGHT);

        // Handle core SDL events
        switch (sdlEvent.type)
        {
            case SDL_QUIT:
            {
                signalQuit = false;
                break;
            }
            case SDL_KEYUP:
            {
                keyPressed = false;
                break;
            }
            case SDL_KEYDOWN:
            {
                keyPressed = true;
                lastKeyPressed = sdlEvent.key.keysym.sym & 0x00FF;
                printf("Key pressed\n");

                if (sdlEvent.key.keysym.sym == SDLK_ESCAPE)
                {
                    keyPressed = true;
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
                else if (sdlEvent.key.keysym.sym == SDLK_1)
                {
                    // We don't want the whole debug window closed when user types 'd', do we?
                    if (!memoryWindow.editorMode)
                    {
                        isDebug = !isDebug;
                    }
                }
                else if (sdlEvent.key.keysym.sym == SDLK_2)
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
    // HACK: Check if any UI needs rendering 
    if (!isDebug && !isDisa && !signalQuit)
    {
        return;
    }
    // Uint32 startTime = SDL_GetTicks();    
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

    // Uint32 endTime = SDL_GetTicks();
    // printf("[DEBUG] Frame Time: %d ms\n", endTime - startTime);
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
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void interpreter_run()
{
    static int frameCounter = 0;
	while (isRunning)
	{
        input();

        sdl_imgui_frame();

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
				cache_dump(newCacheIndex); 
			}
			cache_run(codeCache[newCacheIndex]);
		}
		// if found, then execute
		else
		{
			cache_run(codeCache[cacheIndex]);
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

 		reg[R_PC] += 1;	
		
        instr_call_table[op](instr);
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
        disaWindow.Load(codeCache[cacheIndex].codeBlock, codeCache[cacheIndex].numInstr, codeCache[cacheIndex].lc3MemAddress);
	}
	printf("--------Dumping Cache No. %d END----------\n", cacheIndex);
}

/* Op code functions */

void op_br(uint16_t instr)
{
	/* 
		15 14 13 12 | 11 10 9 | 8 7 6 5 4 3 2 1 0
		0  0  0  0  | n  z  p |    PCOffset9
	*/
	uint16_t pcoffset9 = sign_extended(instr & 0x01FF, 9);
	// If at least one of the nzp bits and the matching bits in R_COND are both 1, then jump
	if (reg[R_COND] & ((instr >> 9) & 0x0007))
	{
		reg[R_PC] += pcoffset9;
	}
}

void op_add(uint16_t instr)
{
	/* 
		15 14 13 12 | 11 10 9 | 8 7 6 | 5 | 4 3 | 2 1 0
		0  0  0  1  |   DR    |  SR1  | 0 | 0 0 |  SR2 
		----------------------or-----------------------
		15 14 13 12 | 11 10 9 | 8 7 6 | 5 | 4 3 2 1 0
		0  0  0  1  |   DR    |  SR   | 1 |    IMM
	*/

	uint8_t dr = (instr >> 9) & 0x0007;
	uint8_t sr = (instr >> 6) & 0x0007;
	uint8_t mode = (instr >> 5) & 0x0001;
	if (mode)
	{
		uint16_t imm = sign_extended(instr & 0x001F, 5);
		reg[dr] = reg[sr] + imm;
	}
	else 
	{
		uint8_t sr2 = instr & 0x0007;
		reg[dr] = reg[sr] + reg[sr2];
	}
	update_flag(reg[dr]);
}

void op_ld(uint16_t instr)
{
	/* 
		15 14 13 12 | 11 10 9 | 8 7 6 5 4 3 2 1 0
		0  0  1  0  |   DR    |    PCOffset9
	*/
	uint16_t pcoffset9 = sign_extended(instr & 0x01FF, 9);
	uint8_t dr = (instr >> 9) & 0x0007;
	// Ignore privilege bit and other security measures
	reg[dr] = read_memory(reg[R_PC] + pcoffset9);
	update_flag(reg[dr]);
}

void op_st(uint16_t instr)
{
	/* 
		15 14 13 12 | 11 10 9 | 8 7 6 5 4 3 2 1 0
		0  0  1  1  |   SR    |    PCOffset9
	*/
	uint16_t pcoffset9 = sign_extended(instr & 0x01FF, 9);
	uint8_t sr = (instr >> 9) & 0x0007;
	write_memory(reg[R_PC] + pcoffset9, reg[sr]);
}

void op_jsr(uint16_t instr)
{
	/* 
		15 14 13 12 | 11 | 10 9 8 7 6 5 4 3 2 1 0
		0  1  0  0  | 1  |      PCOffset11
		-----------------or----------------------
		15 14 13 12 | 11 | 10 9 | 8 7 6 | 5 4 3 2 1 0
		0  1  0  0  | 0  | 0  0 |   BR  | 0 0 0 0 0 0
	*/
	reg[R_R7] = reg[R_PC];
	uint8_t mode = (instr >> 11) & 0x0001;
	if (mode)
	{
		uint16_t pcoffset11 = sign_extended(instr & 0x07FF, 11);
		reg[R_PC] += pcoffset11;
	}
	else
	{
		uint8_t br = (instr >> 6) & 0x0007;
		reg[R_PC] = reg[br];
	}
}

void op_and(uint16_t instr)
{
	/* 
		15 14 13 12 | 11 10 9 | 8 7 6 | 5 | 4 3 | 2 1 0
		0  1  0  1  |    DR   |  SR1  | 0 | 0 0 |  SR2
		---------------------or------------------------
		15 14 13 12 | 11 10 9 | 8 7 6 | 5 | 4 3 2 1 0
		0  1  0  1  |    DR   |  SR1  | 1 |   imm5
	*/
	uint8_t mode = (instr >> 5) & 0x0001;
	uint8_t dr = (instr >> 9) & 0x0007;
	uint8_t sr = (instr >> 6) & 0x0007;
	if (mode)
	{
		uint16_t imm5 = sign_extended(instr & 0x001F, 5);
		reg[dr] = reg[sr] & imm5;
	}
	else
	{
		uint8_t sr2 = instr & 0x0007;
		reg[dr] = reg[sr] & reg[sr2];
	}
	update_flag(reg[dr]);
}

void op_ldr(uint16_t instr)
{
	/* 
		15 14 13 12 | 11 10 9 | 8 7 6 | 5 4 3 2 1 0
		0  1  1  0  |   DR    | BaseR |   offset6
	*/
	// Again ignore the security measures
	uint8_t dr = (instr >> 9) & 0x0007;
	uint8_t br = (instr >> 6) & 0x0007;
	uint16_t offset6 = sign_extended(instr & 0x003F, 6);

	reg[dr] = read_memory(reg[br] + offset6);
	update_flag(reg[dr]);
}

void op_str(uint16_t instr)
{
	/* 
		15 14 13 12 | 11 10 9 | 8 7 6 | 5 4 3 2 1 0
		0  1  1  1  |   SR    | BaseR |   offset6
	*/
	// Again ignore the security measures
	uint8_t sr = (instr >> 9) & 0x0007;
	uint8_t br = (instr >> 6) & 0x0007;
	uint16_t offset6 = sign_extended(instr & 0x003F, 6);

	write_memory(reg[br] + offset6, reg[sr]);
}

void op_rti(uint16_t instr)
{
	/* 
		15 14 13 12 | 11 10 9 8 7 6 5 4 3 2 1 0
		1  0  0  0  | 0  0  0 0 0 0 0 0 0 0 0 0
	*/
	// Technically need to work under privilege mode
	printf("Not supposed to be here!\n");
}

void op_not(uint16_t instr)
{
	/* 
		15 14 13 12 | 11 10 9 | 8 7 6 | 5 | 4 3 2 1 0
		1  0  0  1  |   DR    |   SR  | 1 | 1 1 1 1 1
	*/
	uint8_t dr = (instr >> 9) & 0x0007;
	uint8_t sr = (instr >> 6) & 0x0007;

	reg[dr] = (~reg[sr]);
	update_flag(reg[dr]);
}

void op_ldi(uint16_t instr)
{
	/* 
		15 14 13 12 | 11 10 9 | 8 7 6 5 4 3 2 1 0
		1  0  1  0  |   DR    |    PCoffset9
	*/
	// Again we ignore the security measures
	uint16_t pcoffset9 = sign_extended(instr & 0x01FF, 9);
	uint8_t dr = (instr >> 9) & 0x0007;

	reg[dr] = read_memory(read_memory(reg[R_PC] + pcoffset9));
	update_flag(reg[dr]);
}

void op_sti(uint16_t instr)
{
	/* 
		15 14 13 12 | 11 10 9 | 8 7 6 5 4 3 2 1 0
		1  0  1  1  |   SR    |     PCoffset9
	*/
	// Again ignore the security measures
	uint8_t sr = (instr >> 9) & 0x0007;
	uint16_t pcoffset9 = sign_extended(instr & 0x01FF, 9);

	write_memory(read_memory(reg[R_PC] + pcoffset9), reg[sr]);
}

void op_jmp(uint16_t instr)
{
	/*  JMP
		15 14 13 12 | 11 10 9 | 8 7 6 | 5 4 3 2 1 0
		1  1  0  0  | 0  0  0 | BaseR | 0 0 0 0 0 0
		-------------------or----------------------
		RET
		15 14 13 12 | 11 10 9 | 8 7 6 | 5 4 3 2 1 0
		1  1  0  0  | 0  0  0 | 1 1 1 | 0 0 0 0 0 0
	*/
	uint8_t br = (instr >> 6) & 0x0007;
	// return address stored in R7 so a "jmp" to it equals RET
	reg[R_PC] = reg[br];
}

void op_res(uint16_t instr)
{
	/*  RET
		15 14 13 12 | 11 10 9 | 8 7 6 | 5 4 3 2 1 0
		1  1  0  0  | 0  0  0 | 1 1 1 | 0 0 0 0 0 0
	*/
	// reg[R_PC] = reg[R_R7];
	printf("Not supposed to be here\n");
}

void op_lea(uint16_t instr)
{
	/*
		15 14 13 12 | 11 10 9 | 8 7 6 5 4 3 2 1 0
		1  1  1  0  |    dr   |     PCoffset9
	*/
	uint16_t pcoffset9 = sign_extended(instr & 0x01FF, 9);
	uint8_t dr = (instr >> 9) & 0x0007;

	reg[dr] = reg[R_PC] + pcoffset9;
	update_flag(reg[dr]);
}

void op_trap(uint16_t instr)
{
	/*
		15 14 13 12 | 11 10 9 8 | 7 6 5 4 3 2 1 0
		1  1  1  1  | 0  0  0 0 |    trapvect8
	*/
	reg[R_R7] = reg[R_PC];

	uint8_t trapvect8 = instr & 0x00FF;
	switch (trapvect8)
	{
		case 0x20:
			// GETC
			// Read a single character from the keyboard. The character is not echoed onto the console.
			// Its ASCII code is copied into R0. The high eight bits of R0 are cleared
			trap_0x20();
			break;
		case 0x21:
			trap_0x21();
			break;
		case 0x22:
			trap_0x22();
			break;
		case 0x23:
			trap_0x23();
			break;
		case 0x24:
			trap_0x24();
			break;
		case 0x25:
			trap_0x25();
			break;
		default:
			printf("Erroneous TRAP vector!\n");
	}
}

void update_flag(uint16_t value)
{
	// Clear the last three bits (N/Z/P) and set P
	reg[R_COND] &= 0xFFF8;
	if (value >> 15)
	{	
		// Since value is uint16_t, cannot use if (value < 0), have to check the highest bit
		reg[R_COND] |= FL_NEG;
	}
	else if (value == 0)
	{
		reg[R_COND] |= FL_ZRO;
	}
	else
	{
		reg[R_COND] |= FL_POS;
	}
}

uint16_t read_memory(uint16_t index)
{
	// Two memory mapped registers
	if (index == MR_KBSR)
    {
        if (keyPressed)
        {
            write_memory(MR_KBSR, 1 << 15);
            memory[MR_KBDR] = lastKeyPressed;
            /* 
                WHY ?
                If I don't disable it here, the input is insanely lagged
            */
            keyPressed = false;
        }
        else
        {
            write_memory(MR_KBSR, 0);
        }
        return memory[MR_KBSR];
    }
	return memory[index];
}

uint16_t read_uint16_t(uint16_t index)
{
    return (uint16_t)(memory[index]) | ((uint16_t)(memory[index + 1]) << 8);
}

void write_memory(uint16_t index, uint16_t value)
{
    memory[index] = value;
}

// trap functions
void trap_0x20()
{
	// Read a single character from the keyboard. The character is not echoed onto the console.
	// Its ASCII code is copied into R0. The high eight bits of R0 are cleared
    reg[R_R0] = lastKeyPressed & 0x00FF;
}

void trap_0x21()
{
	// Write a character in R0[7:0] to the console display.
	putc((uint8_t)reg[R_R0], stdout);
	// ui_debug_info(reg, 25);
	fflush(stdout);
}

void trap_0x22()
{
	// Write a string of ASCII characters to the console display. The characters are
	// contained in consecutive memory locations, one character per memory location,
	// starting with the address specified in R0. Writing terminates with the occurrence of
	// x0000 in a memory location.

	for (uint16_t i = reg[R_R0]; ;i++)
	{
		char ch = read_memory(i);
		if (ch == 0)
		{
			break;
		}
		else
		{
			putc(ch, stdout);
		}
	}
	// ui_debug_info(reg, 25);
	fflush(stdout);
}

void trap_0x23()
{
	// Print a prompt on the screen and read a single character from the keyboard. 
	// The character is echoed onto the console monitor, and its ASCII code is copied into R0.
	// The high eight bits of R0 are cleared.
	
	printf("> ");
	reg[R_R0] = (uint16_t)fgetc(stdin);
	reg[R_R0] &= 0x00FF;
	putc((uint8_t)reg[R_R0], stdout);
	// ui_debug_info(reg, 25);
	fflush(stdout);
	update_flag(reg[R_R0]);
}

void trap_0x24()
{
	/*
		Write a string of ASCII characters to the console. 
		The characters are contained in consecutive memory locations, 
		two characters per memory location, starting with the address specified in R0. 

		The ASCII code contained in bits [7:0] of a memory
		location is written to the console first. 
		
		Then the ASCII code contained in bits [15:8] of that memory location is written to the console. 
		
		(A character string consisting of
		an odd number of characters to be written will have x00 in bits [15:8] of the
		memory location containing the last character to be written.) Writing terminates
		with the occurrence of x0000 in a memory location.
	*/
	for (uint16_t i = reg[R_R0]; ;i++)
	{
		uint16_t value = read_memory(i);
		if (value == 0)
		{
			break;
		}
		else
		{
			putc((uint8_t)(value & 0x00FF), stdout);
			putc(((uint8_t)(value >> 8)), stdout);
			// ui_debug_info(reg, 25);
		}
	}
	fflush(stdout);
}

void trap_0x25()
{
	// Halt execution and print a message on the console.
	printf("\nSystem HALT\n");
	isRunning = false;
}