#pragma once

#include <imgui.h>
#include <vector>
#include <cinttypes>

#define PAGE_COLUMNS    16
#define PAGE_ROWS       32

struct charGlyph
{
    unsigned char ch;
    int r;
    int g;
    int b;
};

struct ImGuiWindowConfig
{
    bool readOnly;
    int fontSize;
    ImVec2 initialWindowSize;
    ImVec2 minWindowSize;
    ImVec2 winPos;
};

class MemoryEditor
{
public:
    /* Size and location */
    ImVec2 initialWindowSize;
    ImVec2 minWindowSize;
    ImVec2 winPos;

    std::vector<charGlyph> buffer;
    int64_t bufferSize;
    int64_t cursorStartIndex;
    int64_t cursorEndIndex;
    int64_t initialAddress;

    int64_t minInitialAddress;
    int64_t maxInitialAddress;

    // TODO: For future lock feature: user maybe only want to manipulate the cursor withint a structure
    int64_t cursorStartBoundary;
    int64_t cursorEndBoundary;

    bool readOnly;
    int fontSize;

    ImDrawList* drawList;

public:
    MemoryEditor();
    MemoryEditor(uint8_t* memory, uint64_t memorySize, const ImGuiWindowConfig& config);
    ~MemoryEditor() = default;

    void Draw();
    void Input();

    // Utility functions
    int64_t Calculate_MaxInitialAddress();
};