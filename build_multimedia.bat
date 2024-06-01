if not exist build mkdir build
cd build
cl ../src/multimedia.cpp /link kernel32.lib user32.lib Winmm.lib /OUT:multimedia.exe
