if not exist build mkdir build
cd build
cl ../src/rawinput.cpp /link kernel32.lib user32.lib hid.lib /OUT:rawinput.exe
