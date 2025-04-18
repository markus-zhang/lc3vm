## How to build the memory editor

Right now only supports Linux (I'm using Ubuntu LTSC 22.04, ImGui 1.91.8 WIP, SDL2 last version)

- First, install SDL2 library
- Clone this repo
- Run `make run_me`

## How to build the LC-3 2048 emulator

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

### 2025-01-16 ===================================================

* decide upon the scope of the project - the project should first realize the various debugging windows for the LC3 VM before implementing the optional terminal <- I only care about CPU emulation through dynarec so the first priority is to be able to read the registers, the memory as well as the basic blocks and their translated code. This experiment is just a preparatio for the next system, probably CHIP-8 which does need some SDL code

+ a window for showing all LC3 registers

* The register window simply shows all registers and their values

+ a window for showing the 64K memory

* Memory is shown as HEX values. User should be able to click on any uint16_t value (i.e. two bytes) and modify them

+ a window for showing the basic blocks and their translated code

### 2025-02-08 ===================================================

Sorry I haven't updated the README for a long time, but I have made some progress:

* All ImGui code is implemented (Some not working, see below)

* Moving to a `uint8_t` memory and then move back to `uint16_t` because it's so difficult to find all small places to make the changes. Took me a whole night to move to and another to move back. Yuk!

* Fixed a very nasty bug about `isKeyPressed`. I still don't know why, but if I don't set it to `false` immediately after setting `memory[KBSR]` to 1 (see function `read_memory()`), huge lag follows each key press.

+ I'm still not 100% sure whether one key press is registered as multiple ones, frankly I don't know exactly how the game works -- e.g. after a key WASD is pressed and the board can move in that direction, some new numbers are generated -- are those new number immediately merged towards the same direction, or do they wait for the next key press? I very much think that's the first case in my program, but I'm not sure whether it follows the LC-3 code

+ However by moving the code to `uint16_t`, all ImGui widgets are broken, I need to fix it as this is the first priority.

+ I need to reorganize the code. It needs more love and I feel I use too many switches/flags for different states (e.g. `isKeyPressed`, `isDebug`, `signalQuit`, etc.) -- what if there are more windows? It's going to be a state hell.

### 2025-02-11 =======================================================

New TODOs: To make it a bit more useful, we need to add step-in debugging, otherwise the code disa window is useless.

* However by moving the code to `uint16_t`, all ImGui widgets are broken, I need to fix it as this is the first priority. -> Fixed, very small code change

+ I need to reorganize the code. It needs more love and I feel I use too many switches/flags for different states (e.g. `isKeyPressed`, `isDebug`, `signalQuit`, etc.) -- what if there are more windows? It's going to be a state hell. -> Still don't know what to do

+ Add code in lc3vmwin_disa.cpp that sends a signal to the main program that a key has been pressed to step execute 

+ Add code in lc3vmwin_disa.cpp to add a '>' character before the code to be executed if step in

+ Add a new window for registers

+ Add code in lc3vmwin_memory.cpp that adds an input box. Users can input an address and the memory window "jumps" to that address (or the nearest 0x__00)

### 2025-02-13 =========================================================

+ Add code for a console

### 2025-02-18

* Big day! I figured out why the display is lagging so much. After much debugging (most of the past week and today), I tested everything and finally figured out that the frame rate is crazingly high, and the rendering loop is blocking everything else. It was fixed by simply capping the frame rate to 60 FPS. I actually knew about this in the first day of debugging this issue but somehow didn't take it seriously.

* I figured out how to do the control sequences. It's pretty hacky and ONLY works for 2048, but I really do not have interests to pursuit a general solution.

+ Immediate next minor task: check line 983 TODO in `trap_0x22_imgui()`

+ Immediate next minor task: check line 1033 TODO in `parse_escape()`

+ Next major task: continue working on the input box in lc3vmwin_memory.cpp

### 2025-02-19

* line 1033 TODO in `parse_escape()`: Yes we can comment it all out, because after `parse_escape()` we simply return from `trap_0x22_imgui` so no characters are read

* check line 983 TODO in `trap_0x22_imgui()`: Yes we do need the `return;` becase `ch` is not updated in `parse_escape()`, so when we get out of `parse_escape()`, `ch` is still `0x1B`, and then the program goes into `parse_escape()` again with the next string, which doesn't really have `0x1B` as the first character

+ Next major task: continue working on the input box in lc3vmwin_memory.cpp

+ Next immediate minor task: fix seg fault in Char_Array_to_Number(), must be something to do with array index error

### 2025-02-23

* Fixed seg fault in Char_Array_to_Number(), must be something to do with array index error -> I don't know why buy I'm keeping using `size_t` as loop index, have I lost my mind? Probably.

* Fixed a few bugs about the memory map
    
    * memory was contrainted to uint16_t, which is WRONG because it is supposed to be 64K 16-bit number but the memory data structure has 8-bit for each memory chunk, so it should be uint32_t, from 0x00000 to 0x1ffff

    * the > button doesn't perform properly and seg fault, fixed the code

    * added the >> button, I'll modify the < part, remove the Row < button and add a Page << button. Like, who wants to scroll by row when you can scroll by page?

+ Next mmediately minor task: Remove the Row < button and add Page << button. Keep the code as comment so that we can test the Page <, Page <<, Page > and Page >> buttons easily.

+ Next immediately minor task: align (downward) the return value of `Char_Array_to_Number()` to the nearest 0x00?0, but also make sure that a whole 32 rows are shown. For example, round down 0x1034 to 0x1030 is perfectly fine as we can show all 16 rows from 0x1030, but for 0xfff1 we have to round it further down to 0xfe00 so that all 16 rows are shown.

### 2025-02-24

* Done: Remove the Row < button and add Page << button. Keep the code as comment so that we can test the Page <, Page <<, Page > and Page >> buttons easily.

* Done: align (downward) the return value of `Char_Array_to_Number()` to the nearest 0x00?0, but also make sure that a whole 32 rows are shown. For example, round down 0x1034 to 0x1030 is perfectly fine as we can show all 16 rows from 0x1030, but for 0xfff1 we have to round it further down to 0xfe00 so that all 16 rows are shown.

+ Next major task: continue working on the input box in lc3vmwin_memory.cpp

    + Most is done, but I need to use a Callback to set initialAddress when enter key is hit (I don't want initialAddress to be changed with each keystroke, going to too much)

### 2025-02-26

* Done working on the input box in lc3vmwin_memory.cpp, now it works perfectly, can jump to any address without errors

+ Add code for a console

    + The code is done, but it is a very simple console. I wonder what else we need? Might as well keep this alive for a while


+ Next major task: Add a new window for registers

+ Figure out how to use the `readonly` flag in lc3vmwin_memory. And clean up the code a bit - at least remove all those unused, buddy

### 2025-03-02

* Figured out a way to use UNION to represent different sizes of registers, so that the Regster Window widget can be a big more general (doesn't care about the reg size of the machine)

### 2025-03-03

* Completed the first version of the register watch window

+ Next major task: Add stepin debugging facility (looks like all UI code should live in the disassembler window)
    + an arrow > in the disassembler window 
    + step-in and continue button in the disassembler window

### 2025-03-04

+ Still working on the step-in debugger. I think I figured out how to stop the execution without holding the whole program

### 2025-03-05

* Step-in beautifully done! This is a major milestone.
    * Now code caches are generated properly - previously each line of code generates a new block, which is definitely wrong. Now `cache_find()` check whether the line is WITHIN the boudary of the whole block, and return index of cache as well as index of code to the caller function
    
    * I also modified cache_run() function so that it takes two parameters, the second one the index of the code, so that it doesn't have to ALWAYS execute from the beginning of the code block -> this is vital for step-ins because if it always executes from code index 0, then it never executes the rest of the code in the block!

    * I also confirmed that non-step-in still works perfectly

+ I need to think through what the next step is. I probably need to write more comments and clean things up first

+ Figure out how to use the `readonly` flag in lc3vmwin_memory.

### 2025-03-06

OK since I have reached a milestone I'm going to put this project on hold for the moment. I could work on additional features but this project has accomplished what it was planned for.

## .plan for the memory editor

### 2025-03-07

+ I want to expand the memory editor to an independent product. I included some plans in the .cpp file but of course they are subject to change and expand.

### 2025-03-09

* Basic requirement analysis done

* I have a basic memory editor UI working. I particularly like how I can highlight the cells that are supposed to be "selected" (right now I hardcode cell 0 to 20 to be selected, just for a test)

+ Optional, but figure out how to make the rectangle contigous (no border between)

### 2025-03-11

* CTRL+SHIFT+ARROW implemented

* UP and DOWN ARROWs implemented

### 2025-03-13

* CTRL/SHIFT + UP/DOWN ARROWs implemented

* Fixed an overflow issue by switching to int64_t

* ASCII rectangels implemented

* Different colors for highlighted characters, both in HEX and in ASCII

* Restructured the code a bit, added another function Input()

* Modified Makefile so that I can compile the memory editor as a separate unit

* Added a `load_file()` function to `memory_editor_demo.cpp`

* Modified memory_editor so that it can handle larger files (larger than one page)

* initialAddress now gets adjusted when cursorStartIndex moves out of boundary when we move up/down. For example if we move the cursor down one row, and it goes out of the windows boundary, we need to increment initialAddress for PAGE_COLUMNS so that the whole window moves down for one row too. We always try to maximize the view for the user (e.g. when user moves up the cursor when he is at the top view, we increase initialAddress by a whole PAGE so that the user gets a full page of view and now the cursor is at the bottom of the view, and when the user moves the cursor DOWN, we put the cursor at the top of the next view so he still gets a full page)

+ (Follow-up of previous note) Is there a better way to reset initialAddress and put it into a function instead of scattering all over the program? I'm thinking about something like this: First we detect from which direction (up or down) we are breaking through the screen boundary (initialAddress to initialAddress + PAGE - 1), and deal with it accordingly.

+ Need to load a different font to display Unicode such as visible space `␣`

+ Think about other windows that we need for a hex editor (like a window with implied file format, md65 hashed, etc.)

+ Think about a way to encode file structure (e.g. midi file should have its header, sequences, etc.). Each file format has completely different "sections" but all sections should be able to have differen color

+ Think about a way to limit cursor movement (e.g. lock the cursor within the same section)

### 2025-03-14

* Put reseting initialAddress into a function and fixed a couple of its bugs

* Added a basic scrollbar. Damn I don't really like the ImGui style scrollbar so I'll add my own.

+ Add some other stuffs in the hex window:
    + The current selected range and its size
    + An option button to open the option window??
    + An input box to enter an address, or a saved location to jump to

+ Research how to paint selected cells as unfilled rectangles -- "borders", not as filled ones, this is for the next bulletpoint -> if we already have some coloring patterns on, adding another layer of filled rectangles doesn't work very well, it's a lot easier to recognize by eyes if they have "borders"
    + I think it works like this: if it's minIndex, then draw the left vertical line, two horizontal lines, otherwise always just draw the two horizontal lines, if it's maxIndex, draw two horizontal lines and the right vertical line.

+ Meta data for cell groups
    + For example, cell 0x0010 ~ 0x02f0 belongs to the header, and should be colored as blue on red rectangles
    + We need an option to goggle meta data on/off, sometimes users only want to see the whole picture as flat data, not as groups of cells
    + When meta data is turned on, double click should automatically select the group of cell it is in

### 2025-03-15

OK I need to think through all these features -- since I need this tool to make a CPU emulator/simulator, I don't need all those features, do I?

* Implement a readonly toggle

* Implement +/- keys to change the value of selected bytes

### 2025-03-17

OK I decided to do some research -> the reason is, if I use highlight colors for selected cells, the colors may conflict with the meta data colors, so I gotta use borders to "highlight" them.

* Done researching how to paint selected cells as unfilled rectangles -- "borders", not as filled ones, this is for the next bulletpoint -> if we already have some coloring patterns on, adding another layer of filled rectangles doesn't work very well, it's a lot easier to recognize by eyes if they have "borders".

Right now the borders of the ASCII look a little bit broken but I guess I'm OK with that.

+ How to implement the meta data? I need some examples to think through.
    + Looks like most of the binary formats are like this (simplified view):
        + You have a magic number
        + Then you have something that describe the number of sections
        + For each section you also have a header that tells its length
        + And then data is embedded after the header

    + The scripting language should do this:
        + Define each "part", like magic number, header, etc.
        + Can evaluate values in the bytes and create new "parts" - e.g. read the byte to find the total number of sections
        + Can move around in bytes, e.g. `int32 numSections = read(4);`, this also means we need to track where the read starts

### 2025-03-19

+ Thinking about the meta language

### 2025-04-01

* Restructured the project

+ I'm going to switch to a new emulator project instead of writing the hex editor at the moment. Not very motivated.

### 2025-04-02

OK I think I got into another round of burn-out. I don't think the Z80 is a particularly tough CPU to emulate, and I have just started to write some code for the registers, but somehow I just couldn't muster enough motivation to write another line of code.

And a few weeks ago I had enough motivation to fight ImGui. Damn, guess I'm just not cut to do these kind of things in the long-term.