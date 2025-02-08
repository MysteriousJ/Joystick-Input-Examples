all:
	mkdir -p build
	g++ src/evdev.cpp -std=c++20 -o build/evdev
	g++ src/joydev.cpp -o build/joydev
