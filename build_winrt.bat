if not exist build mkdir build
cd build
cl /std:c++17 /EHsc ../src/winrt.cpp /link kernel32.lib user32.lib windowsapp.lib /OUT:winrt.exe 
