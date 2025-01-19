.PHONY: build build_imgui run run_imgui

run: build
	./lc3vm

build:
	rm -f ./lc3vm
	g++ -Wall -std=c++17 src/*.cpp -I"./libs/" -lSDL2 -o lc3vm

run_imgui:
	./lc3vmimgui

build_imgui:
	rm -f ./lc3vmimgui
	g++ -Wall -Wfatal-errors -g -O0 -std=c++17 ./src/lc3vmwin_memory.cpp ./src/lc3vmimgui.cpp ./libs/imgui/*.cpp -I"./libs/" -L/usr/lib/x86_64-linux-gnu/debug/ -lSDL2 -lSDL2_image -o lc3vmimgui