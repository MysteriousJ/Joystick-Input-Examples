@echo off
cls
set options=-Zi /EHsc /MT

cl %options% "src/rawinput.cpp" /link "kernel32.lib" "user32.lib" "hid.lib" "Winmm.lib" /OUT:"rawinput.exe"
cl %options% "src/xinput.cpp" /link "kernel32.lib" "user32.lib" "Xinput.lib" /OUT:"xinput.exe"
cl %options% "src/multimedia.cpp" /link "kernel32.lib" "user32.lib" "Winmm.lib" /OUT:"multimedia.exe"
