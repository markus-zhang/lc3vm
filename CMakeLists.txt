cmake_minimum_required( VERSION 3.16 )

project( lc3vmwin VERSION 0.1 )

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set build type to Debug
set(CMAKE_BUILD_TYPE Debug)

# Include directories (for your headers and ImGui)
target_include_directories(lc3vmimgui_debug PRIVATE include libs/imgui)

# Source files
file(GLOB SRC_FILES 
    ./src/lc3vmwin_quit_confirm.cpp
    ./src/lc3vmwin_memory.cpp
    ./src/lc3vmwin_disa.cpp
    ./src/lc3vmwin_disa_be.cpp
    ./src/lc3vmwin_loader.cpp
    ./src/lc3vmwin_cache.cpp
    ./src/lc3vmimgui.cpp
)

file(GLOB IMGUI_SRC_FILES 
    ./libs/imgui/*.cpp
)

add_executable(lc3vmimgui_debug ${SRC_FILES} ${IMGUI_SRC_FILES})

# Compiler warnings and flags
target_compile_options(lc3vmimgui_debug PRIVATE
    -Wall -Wfatal-errors -Wextra -Wconversion -Wsign-conversion -pedantic-errors -g -O0
)

# Link SDL2 and SDL2_image
target_link_libraries(lc3vmimgui_debug PRIVATE SDL2 SDL2_image)

# Custom target to run the executable
add_custom_target(run_imgui_debug
    COMMAND ./lc3vmimgui_debug
    DEPENDS lc3vmimgui_debug
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)