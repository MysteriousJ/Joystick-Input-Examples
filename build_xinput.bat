if not exist build mkdir build
cd build
cl ../src/xinput.cpp /link Xinput.lib /OUT:xinput.exe
