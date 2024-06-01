if not exist build mkdir build
cd build
cl ../src/gameinput.cpp /link ../gameinput.lib /OUT:gameinput.exe
