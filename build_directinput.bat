if not exist build mkdir build
cd build
cl ../src/directinput.cpp /link kernel32.lib user32.lib dinput8.lib dxguid.lib /OUT:directinput.exe
