# Directories
SRC_DIR := src
SRC_DIR_MEMORY_EDITOR := src/memory_editor
INCLUDE_DIR := include
INCLUDE_DIR_MEMORY_EDITOR := include/memory_editor
IMGUI_DIR := libs/imgui
BUILD_DIR := build
BUILD_DIR_MEMORY_EDITOR := build/memory_editor

# Compiler and flags
CXX := g++

CXXFLAGS := -Wall -Wfatal-errors -Wconversion -Wsign-conversion -pedantic-errors -g -O0 -std=c++17 \
-I$(INCLUDE_DIR) -I$(IMGUI_DIR)

CXXFLAGS_MEMORY_EDITOR := -Wall -Wfatal-errors -Wconversion -Wsign-conversion -pedantic-errors -g -O0 -std=c++17 \
-I$(INCLUDE_DIR_MEMORY_EDITOR) -I$(IMGUI_DIR)

CXXFLAGS_LIB = -g -O0 -std=c++17

# Source files
SRC_FILES := $(wildcard $(SRC_DIR)/lc3vmwin_*.cpp) $(SRC_DIR)/memory_editor.cpp
IMGUI_FILES := $(wildcard $(IMGUI_DIR)/*.cpp)

# Memory Editor Source files
SRC_FILES_MEMORY_EDITOR = $(wildcard $(SRC_DIR_MEMORY_EDITOR)/*.cpp)
IMGUI_FILES := $(wildcard $(IMGUI_DIR)/*.cpp)

# Object files
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRC_FILES))
IMGUI_OBJ_FILES := $(patsubst $(IMGUI_DIR)/%.cpp, $(BUILD_DIR)/imgui_%.o, $(IMGUI_FILES))

# Memory Editor Object files
OBJ_FILES_MEMORY_EDITOR = $(patsubst $(SRC_DIR_MEMORY_EDITOR)/%.cpp, $(BUILD_DIR_MEMORY_EDITOR)/%.o, $(SRC_FILES_MEMORY_EDITOR))

# Executable
TARGET := lc3vmimgui_debug

# Memory Editor Executable
TARGET_MEMORY_EDITOR := memory_editor

# Libraries
LIBS := -lSDL2 -lSDL2_image

# Build rules
all: $(TARGET)

# Memory Editor Build rules
all_memory_editor: $(TARGET_MEMORY_EDITOR)

# Compile lc3vmwin object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# Compile memory editor object files
$(BUILD_DIR_MEMORY_EDITOR)/%.o: $(SRC_DIR_MEMORY_EDITOR)/%.cpp
	@mkdir -p $(BUILD_DIR_MEMORY_EDITOR)
	$(CXX) $(CXXFLAGS_MEMORY_EDITOR) -I$(INCLUDE_DIR_MEMORY_EDITOR) -c $< -o $@

# Compile ImGui object files
$(BUILD_DIR)/imgui_%.o: $(IMGUI_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS_LIB) -I$(IMGUI_DIR) -c $< -o $@

# Link everything into the final executable
$(TARGET): $(OBJ_FILES) $(IMGUI_OBJ_FILES) $(SRC_DIR)/lc3vmimgui.cpp
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -I$(IMGUI_DIR) $(SRC_DIR)/lc3vmimgui.cpp $(OBJ_FILES) $(IMGUI_OBJ_FILES) $(LIBS) -o $(TARGET)

# Memory Editor Link
$(TARGET_MEMORY_EDITOR): $(OBJ_FILES_MEMORY_EDITOR) $(IMGUI_OBJ_FILES) $(SRC_DIR_MEMORY_EDITOR)/memory_editor_demo.cpp
	$(CXX) $(CXXFLAGS_MEMORY_EDITOR) $(OBJ_FILES_MEMORY_EDITOR) $(IMGUI_OBJ_FILES) $(LIBS) -o $(TARGET_MEMORY_EDITOR)

# Run the program
.PHONY: run
run: $(TARGET)
	./$(TARGET)

.PHONY: build
run: $(TARGET)

# Run memory editor
.PHONY: run_me
run_me: $(TARGET_MEMORY_EDITOR)
		./$(TARGET_MEMORY_EDITOR)

.PHONY: build_me
run_me: $(TARGET_MEMORY_EDITOR)

# Clean build files
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(TARGET)

# Clean memory editor build files
.PHONY: clean_me
clean_me:
	rm -rf $(BUILD_DIR_MEMORY_EDITOR) $(TARGET_MEMORY_EDITOR)