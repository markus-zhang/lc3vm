## How to build

Right now only supports Linux (I'm using Ubuntu LTSC 22.04, ImGui 1.91.8 WIP, SDL2 last version)

- First, install SDL2 library
- Clone this repo
- Run `make run`

## .plan

This is my night work ...

When I accomplish something or stroke down some notes, I write a * line that day. If I create a new task, I write a + line.

Whenever a bug /missing feature is mentioned during the night and I don't fix it, I make a not of it. Some things get noted many times before they get fixed.

-> means "thus", <- means "because"

--- John Carmack's student

= 2025-01-16 ===================================================

* decide upon the scope of the project - the project should first realize the various debugging windows for the LC3 VM before implementing the optional terminal <- I only care about CPU emulation through dynarec so the first priority is to be able to read the registers, the memory as well as the basic blocks and their translated code. This experiment is just a preparatio for the next system, probably CHIP-8 which does need some SDL code

+ a window for showing all LC3 registers

* The register window simply shows all registers and their values

+ a window for showing the 64K memory

* Memory is shown as HEX values. User should be able to click on any uint16_t value (i.e. two bytes) and modify them

+ a window for showing the basic blocks and their translated code

= 2025-02-08 ===================================================

Sorry I haven't updated the README for a long time, but I have made some progress:

* All ImGui code is implemented (Some not working, see below)

* Moving to a `uint8_t` memory and then move back to `uint16_t` because it's so difficult to find all small places to make the changes. Took me a whole night to move to and another to move back. Yuk!

* Fixed a very nasty bug about `isKeyPressed`. I still don't know why, but if I don't set it to `false` immediately after setting `memory[KBSR]` to 1 (see function `read_memory()`), huge lag follows each key press.

+ I'm still not 100% sure whether one key press is registered as multiple ones, frankly I don't know exactly how the game works -- e.g. after a key WASD is pressed and the board can move in that direction, some new numbers are generated -- are those new number immediately merged towards the same direction, or do they wait for the next key press? I very much think that's the first case in my program, but I'm not sure whether it follows the LC-3 code

+ However by moving the code to `uint16_t`, all ImGui widgets are broken, I need to fix it as this is the first priority.

+ I need to reorganize the code. It needs more love and I feel I use too many switches/flags for different states (e.g. `isKeyPressed`, `isDebug`, `signalQuit`, etc.) -- what if there are more windows? It's going to be a state hell.

= 2025-02-11 =======================================================

New TODOs: To make it a bit more useful, we need to add step-in debugging, otherwise the code disa window is useless.

* However by moving the code to `uint16_t`, all ImGui widgets are broken, I need to fix it as this is the first priority. -> Fixed, very small code change

+ I need to reorganize the code. It needs more love and I feel I use too many switches/flags for different states (e.g. `isKeyPressed`, `isDebug`, `signalQuit`, etc.) -- what if there are more windows? It's going to be a state hell. -> Still don't know what to do

+ Add code in lc3vmwin_disa.cpp that sends a signal to the main program that a key has been pressed to step execute 

+ Add code in lc3vmwin_disa.cpp to add a '>' character before the code to be executed if step in

+ Add a new window for registers

+ Add code in lc3vmwin_memory.cpp that adds an input box. Users can input an address and the memory window "jumps" to that address (or the nearest 0x__00)

= 2025-02-13 =========================================================

+ Add code for a console

= 2025-02-18

* Big day! I figured out why the display is lagging so much. After much debugging (most of the past week and today), I tested everything and finally figured out that the frame rate is crazingly high, and the rendering loop is blocking everything else. It was fixed by simply capping the frame rate to 60 FPS. I actually knew about this in the first day of debugging this issue but somehow didn't take it seriously.

* I figured out how to do the control sequences. It's pretty hacky and ONLY works for 2048, but I really do not have interests to pursuit a general solution.

+ Immediate next minor task: check line 983 TODO in `trap_0x22_imgui()`

+ Immediate next minor task: check line 1033 TODO in `parse_escape()`

+ Next major task: continue working on the input box in lc3vmwin_memory.cpp

= 2025-02-19

* line 1033 TODO in `parse_escape()`: Yes we can comment it all out, because after `parse_escape()` we simply return from `trap_0x22_imgui` so no characters are read

* check line 983 TODO in `trap_0x22_imgui()`: Yes we do need the `return;` becase `ch` is not updated in `parse_escape()`, so when we get out of `parse_escape()`, `ch` is still `0x1B`, and then the program goes into `parse_escape()` again with the next string, which doesn't really have `0x1B` as the first character

+ Next major task: continue working on the input box in lc3vmwin_memory.cpp

+ Next immediate minor task: fix seg fault in Char_Array_to_Number(), must be something to do with array index error

= 2025-02-23

* Fixed seg fault in Char_Array_to_Number(), must be something to do with array index error -> I don't know why buy I'm keeping using `size_t` as loop index, have I lost my mind? Probably.

* Fixed a few bugs about the memory map
    
    * memory was contrainted to uint16_t, which is WRONG because it is supposed to be 64K 16-bit number but the memory data structure has 8-bit for each memory chunk, so it should be uint32_t, from 0x00000 to 0x1ffff

    * the > button doesn't perform properly and seg fault, fixed the code

    * added the >> button, I'll modify the < part, remove the Row < button and add a Page << button. Like, who wants to scroll by row when you can scroll by page?

+ Next mmediately minor task: Remove the Row < button and add Page << button. Keep the code as comment so that we can test the Page <, Page <<, Page > and Page >> buttons easily.

+ Next immediately minor task: align (downward) the return value of `Char_Array_to_Number()` to the nearest 0x00?0, but also make sure that a whole 32 rows are shown. For example, round down 0x1034 to 0x1030 is perfectly fine as we can show all 16 rows from 0x1030, but for 0xfff1 we have to round it further down to 0xfe00 so that all 16 rows are shown.

= 2025-02-24

* Done: Remove the Row < button and add Page << button. Keep the code as comment so that we can test the Page <, Page <<, Page > and Page >> buttons easily.

* Done: align (downward) the return value of `Char_Array_to_Number()` to the nearest 0x00?0, but also make sure that a whole 32 rows are shown. For example, round down 0x1034 to 0x1030 is perfectly fine as we can show all 16 rows from 0x1030, but for 0xfff1 we have to round it further down to 0xfe00 so that all 16 rows are shown.

+ Next major task: continue working on the input box in lc3vmwin_memory.cpp

    + Most is done, but I need to use a Callback to set initialAddress when enter key is hit (I don't want initialAddress to be changed with each keystroke, going to too much)

= 2025-02-26

* Done working on the input box in lc3vmwin_memory.cpp, now it works perfectly, can jump to any address without errors

+ Add code for a console

    + The code is done, but it is a very simple console. I wonder what else we need? Might as well keep this alive for a while


+ Next major task: Add a new window for registers

+ Figure out how to use the `readonly` flag in lc3vmwin_memory. And clean up the code a bit - at least remove all those unused, buddy

= 2025-03-02

* Figured out a way to use UNION to represent different sizes of registers, so that the Regster Window widget can be a big more general (doesn't care about the reg size of the machine)

= 2025-03-03

* Completed the first version of the register watch window

+ Next major task: Add stepin debugging facility (looks like all UI code should live in the disassembler window)
    + an arrow > in the disassembler window 
    + step-in and continue button in the disassembler window

= 2025-03-04

+ Still working on the step-in debugger. I think I figured out how to stop the execution without holding the whole program

= 2025-03-05

* Step-in beautifully done! This is a major milestone.
    * Now code caches are generated properly - previously each line of code generates a new block, which is definitely wrong. Now `cache_find()` check whether the line is WITHIN the boudary of the whole block, and return index of cache as well as index of code to the caller function
    
    * I also modified cache_run() function so that it takes two parameters, the second one the index of the code, so that it doesn't have to ALWAYS execute from the beginning of the code block -> this is vital for step-ins because if it always executes from code index 0, then it never executes the rest of the code in the block!

    * I also confirmed that non-step-in still works perfectly

+ I need to think through what the next step is. I probably need to write more comments and clean things up first

+ Figure out how to use the `readonly` flag in lc3vmwin_memory.

= 2025-03-06

OK since I have reached a milestone I'm going to put this project on hold for the moment. I could work on additional features but this project has accomplished what it was planned for.

= 2025-03-07

+ I want to expand the memory editor to an independent product. I included some plans in the .cpp file but of course they are subject to change and expand.

= 2025-03-09

* Basic requirement analysis done

* I have a basic memory editor UI working. I particularly like how I can highlight the cells that are supposed to be "selected" (right now I hardcode cell 0 to 20 to be selected, just for a test)

+ Optional, but figure out how to make the rectangle contigous (no border between)

= 2025-03-11

* CTRL+SHIFT+ARROW implemented

* UP and DOWN ARROWs implemented

= 2025-03-13

* CTRL/SHIFT + UP/DOWN ARROWs implemented

* Fixed an overflow issue by switching to int64_t

* ASCII rectangels implemented

* Different colors for highlighted characters, both in HEX and in ASCII

* Restructured the code a bit, added another function Input()

+ IMPORTANT: Modify Makefile so that I can compile the memory editor as a separate unit

+ Need to load a different font to display Unicode such as visible space `␣`

+ Think about other windows that we need for a hex editor (like a window with implied file format, md65 hashed, etc.)

+ Think about a way to encode file structure (e.g. midi file should have its header, sequences, etc.). Each file format has completely different "sections" but all sections should be able to have differen color

+ Think about a way to limit cursor movement (e.g. lock the cursor within the same section)