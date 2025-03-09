#pragma once

#include <imgui.h>
#include <vector>
#include <cinttypes>

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
    uint64_t bufferSize;
    uint64_t cursorStartIndex;
    uint64_t cursorEndIndex;
    uint64_t initialAddress;

    bool readOnly;
    int fontSize;

public:
    MemoryEditor();
    MemoryEditor(uint8_t* memory, uint64_t memorySize, const ImGuiWindowConfig& config);
    ~MemoryEditor() = default;

    void Draw();
};