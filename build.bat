@echo off
cls
set options=-Zi /EHsc /MT
set defines=/D"WIN32"

cl %options% %defines% "src/rawinput.cpp" /link "kernel32.lib" "user32.lib" "hid.lib" "Winmm.lib" /OUT:"rawinput.exe"