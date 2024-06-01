if not exist build mkdir build
cd build
cl ../src/rawinput_buffered.cpp /link kernel32.lib user32.lib hid.lib /OUT:rawinput_buffered.exe
