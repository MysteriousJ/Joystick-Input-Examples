@echo off
cls
set options=-Zi /EHsc /MT

cl %options% "src/multimedia.cpp" /link "kernel32.lib" "user32.lib" "Winmm.lib" /OUT:"multimedia.exe"
cl %options% "src/directinput.cpp" /link "kernel32.lib" "user32.lib" "dinput8.lib" "dxguid.lib" /OUT:"directinput.exe"
cl %options% "src/rawinput.cpp" /link "kernel32.lib" "user32.lib" "hid.lib" /OUT:"rawinput.exe"
cl %options% "src/ps4.cpp" /link "kernel32.lib" "user32.lib" "hid.lib" /OUT:"ps4.exe"
cl %options% "src/xinput.cpp" /link "kernel32.lib" "user32.lib" "Xinput.lib" /OUT:"xinput.exe"
cl %options% "src/combined.cpp" /link "kernel32.lib" "user32.lib" "hid.lib" "Xinput.lib" /OUT:"combined.exe"
