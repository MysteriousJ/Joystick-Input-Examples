all:
	mkdir -p build
	g++ src/evdev.cpp -o build/evdev
	g++ src/joydev.cpp -o build/joydev
