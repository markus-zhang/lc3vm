.PHONY: build build_imgui_debug build_imgui_release build_test run run_imgui_debug run_imgui_release

run: build
	./lc3vm

build:
	rm -f ./lc3vm
	g++ -Wall -std=c++17 src/*.cpp -I"./libs/" -lSDL2 -o lc3vm

run_imgui_debug:
	./lc3vmimgui_debug

run_imgui_release:
	./lc3vmimgui_release

build_imgui_debug:
	rm -f ./lc3vmimgui_debug
	g++ -Wall -Wfatal-errors -g -O0 -std=c++17 \
	./src/lc3vmwin_quit_confirm.cpp \
	./src/lc3vmwin_memory.cpp \
	./src/lc3vmwin_disa.cpp \
	./src/lc3vmwin_disa_be.cpp \
	./src/lc3vmwin_loader.cpp \
	./src/lc3vmwin_cache.cpp \
	./src/lc3vmimgui.cpp \
	./libs/imgui/*.cpp \
	-I"./libs/" \
	-L/usr/lib/x86_64-linux-gnu/debug/ \
	-lSDL2 -lSDL2_image \
	-o lc3vmimgui_debug

build_imgui_release:
	rm -f ./lc3vmimgui_release
	g++ -Wall -Wfatal-errors -O0 -std=c++17 \
	./src/lc3vmwin_quit_confirm.cpp \
	./src/lc3vmwin_memory.cpp \
	./src/lc3vmwin_disa.cpp \
	./src/lc3vmwin_disa_be.cpp \
	./src/lc3vmwin_loader.cpp \
	./src/lc3vmwin_cache.cpp \
	./src/lc3vmimgui.cpp \
	./libs/imgui/*.cpp \
	-I"./libs/"
	-lSDL2 -lSDL2_image \
	-o lc3vmimgui_release

build_test:
	rm -f ./test
	g++ -Wall -Wfatal-errors -g -O0 -std=c++17 ./src/test.cpp -o test