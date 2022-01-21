@echo off
set options=-Zi /EHsc /MT /nologo

cl %options% src/multimedia.cpp /link kernel32.lib user32.lib Winmm.lib /OUT:multimedia.exe
cl %options% src/directinput.cpp /link kernel32.lib user32.lib dinput8.lib dxguid.lib /OUT:directinput.exe
cl %options% src/rawinput.cpp /link kernel32.lib user32.lib hid.lib /OUT:rawinput.exe
cl %options% src/rawinput_buffered.cpp /link kernel32.lib user32.lib hid.lib /OUT:rawinput_buffered.exe
cl %options% src/specialized.cpp /link kernel32.lib user32.lib hid.lib /OUT:specialized.exe
cl %options% src/xinput.cpp /link kernel32.lib user32.lib Xinput.lib /OUT:xinput.exe
cl %options% src/combined.cpp /link kernel32.lib user32.lib hid.lib Xinput.lib /OUT:combined.exe
cl %options% src/config.cpp /link kernel32.lib user32.lib dinput8.lib dxguid.lib /OUT:config.exe
cl %options% src/winrt.cpp /std:c++17 /link kernel32.lib user32.lib windowsapp.lib /OUT:winrt.exe
