# Directories
SRC_DIR := src
INCLUDE_DIR := include
IMGUI_DIR := libs/imgui
BUILD_DIR := build

# Compiler and flags
CXX := g++

CXXFLAGS := -Wall -Wfatal-errors -Wconversion -Wsign-conversion -pedantic-errors -g -O0 -std=c++17 \
-I$(INCLUDE_DIR) -I$(IMGUI_DIR)

CXXFLAGS_LIB = -g -O0 -std=c++17

# Source files
SRC_FILES := $(wildcard $(SRC_DIR)/lc3vmwin_*.cpp) $(SRC_DIR)/memory_editor.cpp
IMGUI_FILES := $(wildcard $(IMGUI_DIR)/*.cpp)

# Object files
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRC_FILES))
IMGUI_OBJ_FILES := $(patsubst $(IMGUI_DIR)/%.cpp, $(BUILD_DIR)/imgui_%.o, $(IMGUI_FILES))

# Executable
TARGET := lc3vmimgui_debug

# Libraries
LIBS := -lSDL2 -lSDL2_image

# Build rules
all: $(TARGET)

# Compile lc3vmwin object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# Compile ImGui object files
$(BUILD_DIR)/imgui_%.o: $(IMGUI_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS_LIB) -I$(IMGUI_DIR) -c $< -o $@

# Link everything into the final executable
$(TARGET): $(OBJ_FILES) $(IMGUI_OBJ_FILES) $(SRC_DIR)/lc3vmimgui.cpp
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -I$(IMGUI_DIR) $(SRC_DIR)/lc3vmimgui.cpp $(OBJ_FILES) $(IMGUI_OBJ_FILES) $(LIBS) -o $(TARGET)

# Run the program
.PHONY: run
run: $(TARGET)
	./$(TARGET)

.PHONY: build
run: $(TARGET)

# Clean build files
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(TARGET)
