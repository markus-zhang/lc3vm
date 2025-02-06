#pragma once

#include <imgui.h>
#include <cinttypes>

#define MAX_SIZE 65536

enum
{
    OP_BR = 0,
    OP_ADD,
    OP_LD,
    OP_ST,
    OP_JSR,
    OP_AND,
    OP_LDR,
    OP_STR,
    OP_RTI,
    OP_NOT,
    OP_LDI,
    OP_STI,
    OP_JMP,
    OP_RSV,
    OP_LEA,
    OP_TRAP
};

enum {
	DEBUG_OFF = 0,
	DEBUG_ON,
	DEBUG_DIS
};

extern uint8_t DEBUG_MODE;

enum Error
{
    ERROR_LOADFILE = 1,
    ERROR_LOAD_FILE_HEADER,
    ERROR_VM_INIT_FAIL
};

struct Glyph
{
    unsigned char ch;
    int r;
    int g;
    int b;
};

struct WindowConfig
{
    bool readonly;
    int fontSize;
    ImVec2 initialWindowSize;
    ImVec2 minWindowSize;
    ImVec2 winPos;
};