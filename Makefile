run: build
	./lc3vm

build:
	rm -f ./lc3vm
	g++ -Wall -std=c++17 src/*.cpp -I"./libs/" -lSDL2 -o lc3vm

