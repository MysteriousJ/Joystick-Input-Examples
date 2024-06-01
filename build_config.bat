if not exist build mkdir build
cd build
cl ../src/config.cpp /link kernel32.lib user32.lib dinput8.lib dxguid.lib /OUT:config.exe
