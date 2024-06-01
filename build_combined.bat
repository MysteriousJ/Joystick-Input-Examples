if not exist build mkdir build
cd build
cl ../src/combined.cpp /link kernel32.lib user32.lib hid.lib Xinput.lib /OUT:combined.exe
