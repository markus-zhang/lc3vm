# Directories
SRC_DIR_LC3VM := src/lc3vm
SRC_DIR_MEMORY_EDITOR := src/memory_editor
INCLUDE_DIR_LC3VM := include/lc3vm include/memory_editor
INCLUDE_FLAGS_LC3VM := $(addprefix -I, $(INCLUDE_DIR_LC3VM))
INCLUDE_DIR_MEMORY_EDITOR := include/memory_editor
IMGUI_DIR := libs/imgui
BUILD_DIR_LC3VM := build/lc3vm
BUILD_DIR_MEMORY_EDITOR := build/memory_editor
BUILD_DIR_IMGUI := build/imgui

# Compiler and flags
CXX := g++

CXXFLAGS_LC3VM := -Wall -Wfatal-errors -Wconversion -Wsign-conversion -pedantic-errors -g -O0 -std=c++17 \
$(INCLUDE_FLAGS_LC3VM) -I$(IMGUI_DIR)

CXXFLAGS_MEMORY_EDITOR := -Wall -Wfatal-errors -Wconversion -Wsign-conversion -pedantic-errors -g -O0 -std=c++17 \
-I$(INCLUDE_DIR_MEMORY_EDITOR) -I$(IMGUI_DIR)

CXXFLAGS_LIB = -g -O0 -std=c++17

# Source files
SRC_FILES_LC3VM := $(wildcard $(SRC_DIR_LC3VM)/lc3vmwin_*.cpp) $(SRC_DIR_MEMORY_EDITOR)/memory_editor.cpp
IMGUI_FILES := $(wildcard $(IMGUI_DIR)/*.cpp)

# Memory Editor Source files
SRC_FILES_MEMORY_EDITOR = $(wildcard $(SRC_DIR_MEMORY_EDITOR)/*.cpp)
IMGUI_FILES := $(wildcard $(IMGUI_DIR)/*.cpp)

# Object files
OBJ_FILES_LC3VM := $(patsubst $(SRC_DIR_LC3VM)/%.cpp, $(BUILD_DIR_LC3VM)/%.o, $(SRC_FILES_LC3VM))
IMGUI_OBJ_FILES := $(patsubst $(IMGUI_DIR)/%.cpp, $(BUILD_DIR_IMGUI)/imgui_%.o, $(IMGUI_FILES))

# Memory Editor Object files
OBJ_FILES_MEMORY_EDITOR = $(patsubst $(SRC_DIR_MEMORY_EDITOR)/%.cpp, $(BUILD_DIR_MEMORY_EDITOR)/%.o, $(SRC_FILES_MEMORY_EDITOR))

# Executable
TARGET_LC3VM := lc3vmimgui_debug

# Memory Editor Executable
TARGET_MEMORY_EDITOR := memory_editor

# Libraries
LIBS := -lSDL2 -lSDL2_image

# Build rules
lc3vm: $(TARGET_LC3VM)

# Memory Editor Build rules
memory_editor: $(TARGET_MEMORY_EDITOR)

# Compile lc3vmwin object files
$(BUILD_DIR_LC3VM)/%.o: $(SRC_DIR_LC3VM)/%.cpp
	@mkdir -p $(BUILD_DIR_LC3VM)
	$(CXX) $(CXXFLAGS_LC3VM) -c $< -o $@

# Compile memory editor object files
$(BUILD_DIR_MEMORY_EDITOR)/%.o: $(SRC_DIR_MEMORY_EDITOR)/%.cpp
	@mkdir -p $(BUILD_DIR_MEMORY_EDITOR)
	$(CXX) $(CXXFLAGS_MEMORY_EDITOR) -c $< -o $@

# Compile ImGui object files
$(BUILD_DIR_IMGUI)/imgui_%.o: $(IMGUI_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR_IMGUI)
	$(CXX) $(CXXFLAGS_LIB) -I$(IMGUI_DIR) -c $< -o $@

# Link everything into the final executable
$(TARGET_LC3VM): $(OBJ_FILES_LC3VM) $(IMGUI_OBJ_FILES) $(SRC_DIR_LC3VM)/lc3vmimgui.cpp
	$(CXX) $(CXXFLAGS_LC3VM) $(SRC_DIR_LC3VM)/lc3vmimgui.cpp $(OBJ_FILES_LC3VM) $(IMGUI_OBJ_FILES) $(LIBS) -o $(TARGET_LC3VM)

# Memory Editor Link
$(TARGET_MEMORY_EDITOR): $(OBJ_FILES_MEMORY_EDITOR) $(IMGUI_OBJ_FILES) $(SRC_DIR_MEMORY_EDITOR)/memory_editor_demo.cpp
	$(CXX) $(CXXFLAGS_MEMORY_EDITOR) $(OBJ_FILES_MEMORY_EDITOR) $(IMGUI_OBJ_FILES) $(LIBS) -o $(TARGET_MEMORY_EDITOR)

# Run the program
.PHONY: run_lc3vm
run_lc3vm: $(TARGET_LC3VM)
	./$(TARGET_LC3VM)

.PHONY: build_lc3vm
build_lc3vm: $(TARGET_LC3VM)

# Run memory editor
.PHONY: run_me
run_me: $(TARGET_MEMORY_EDITOR)
		./$(TARGET_MEMORY_EDITOR)

.PHONY: build_me
build_me: $(TARGET_MEMORY_EDITOR)

# Clean lc3vm build files
.PHONY: clean_lc3vm
clean_lc3vm:
	rm -rf $(BUILD_DIR_LC3VM) $(TARGET_LC3VM)

# Clean memory editor build files
.PHONY: clean_me
clean_me:
	rm -rf $(BUILD_DIR_MEMORY_EDITOR) $(TARGET_MEMORY_EDITOR)