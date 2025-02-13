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