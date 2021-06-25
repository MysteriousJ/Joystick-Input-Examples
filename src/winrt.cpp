#pragma comment(lib, "windowsapp")
#include <list>
#include <winrt/Windows.Gaming.Input.h>
#include <ppl.h>
#include <Windows.h>

// Xbox controller registers as both a Gamepad and a RawGameController.
// Xbox controllers require the window to be focused to produce input.
// It does at least use XInput for RawGameController when used with XBox controllers, so shoulder triggers are separate.
// PS4 controller returns all zeroes for button labels and is not seen as a haptics device.
// My PS3 fightstick does not register as an ArcadeStick.
// You can continue to access a controller after it has been unplugged. Just returns 0 for all the inputs.
// This error happens when linking with subsystem:console: onecoreuap\xbox\devices\api\winrt\pnpdevicewatcher.cpp(500)\Windows.Gaming.Input.dll!7B6C2CC8: (caller: 7B6C2675) ReturnHr(1) tid(4ae8) 80070006 The handle is invalid.
// If you unplug the controller while it is being used, these errors happen:
//    onecoreuap\xbox\devices\api\winrt\xusbdevice.cpp(788)\Windows.Gaming.Input.dll!7B6CE984: (caller: 7B6CDE70) ReturnHr(2) tid(1468) 80070016 The device does not recognize the command.
//    onecoreuap\xbox\devices\api\winrt\providermanagerworker.cpp(345)\Windows.Gaming.Input.dll!7B6C415D: (caller: 7B6C40B6) ReturnHr(3) tid(1468) 80070016 The device does not recognize the command.
// Closing the window while accessing DisplayName cause it to throw an exception.

struct Joystick
{
	bool connected;
	winrt::Windows::Gaming::Input::RawGameController rawGameController;
};

int main()
{
	WNDCLASSA wnd = {0};
	wnd.hInstance = GetModuleHandleA(0);
	wnd.lpfnWndProc = DefWindowProcA;
	wnd.lpszClassName = "Input Focus Window";
	wnd.hCursor = LoadCursorW(0, IDC_ARROW);
	RegisterClassA(&wnd);
	HWND hwnd = CreateWindowA(wnd.lpszClassName, wnd.lpszClassName, WS_OVERLAPPEDWINDOW|WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 300, 200, 0, 0, wnd.hInstance, 0);

	std::vector<Joystick> joysticks;
	concurrency::critical_section controllerListLock;

	winrt::Windows::Gaming::Input::RawGameController::RawGameControllerAdded([&joysticks, &controllerListLock](winrt::Windows::Foundation::IInspectable const&, winrt::Windows::Gaming::Input::RawGameController const& addedController)
		{
			concurrency::critical_section::scoped_lock {controllerListLock};
			for (auto& joystick : joysticks) {
				if (!joystick.connected) {
					joystick.rawGameController = addedController;
					joystick.connected = true;
					return;
				}
			}
			joysticks.push_back({true, addedController});
		});

	winrt::Windows::Gaming::Input::RawGameController::RawGameControllerRemoved([&joysticks, &controllerListLock](winrt::Windows::Foundation::IInspectable const&, winrt::Windows::Gaming::Input::RawGameController const& removedController)
		{
			concurrency::critical_section::scoped_lock {controllerListLock};
			for (auto& joystick : joysticks) {
				if (joystick.rawGameController == removedController) {
					joystick.connected = false;
					return;
				}
			}
		});

	while (1) {
		MSG message;
		while (PeekMessageA(&message, 0, 0, 0, true)) {
			TranslateMessage(&message);
			DispatchMessageA(&message);
		}

		winrt::Windows::Foundation::Collections::IVectorView<winrt::Windows::Gaming::Input::RawGameController> joystickList = winrt::Windows::Gaming::Input::RawGameController::RawGameControllers();
		for (unsigned int joystickIndex=0; joystickIndex<joystickList.Size(); ++joystickIndex)
		{
			bool buttons[64];
			winrt::Windows::Gaming::Input::GameControllerSwitchPosition switches[64];
			double axes[64];

			joystickList.GetAt(joystickIndex).GetCurrentReading(buttons, switches, axes);
			printf("Axes - ");
			for (int i = 0; i < joystickList.GetAt(joystickIndex).AxisCount(); ++i) {
				printf("%d:%f ", i, axes[i]);
			}
			printf("Switches - ");
			for (int i = 0; i < joystickList.GetAt(joystickIndex).SwitchCount(); ++i) {
				printf("%d:%d ", i, switches[i]);
			}
			printf("Buttons - ");
			for (int i = 0; i < joystickList.GetAt(joystickIndex).ButtonCount(); ++i) {
				if (buttons[i]) printf("%d ", i);
			}
			puts("");
		}

		//concurrency::critical_section::scoped_lock {controllerListLock};
		//for (auto& joystick : joysticks)
		//{
		//	if (joystick.connected)
		//	{
		//		wprintf(L"Joystick - %s, ", joystick.rawGameController.DisplayName().c_str());
		//
		//		bool buttons[64];
		//		winrt::Windows::Gaming::Input::GameControllerSwitchPosition switches[64];
		//		double axes[64];
		//
		//		joystick.rawGameController.GetCurrentReading(buttons, switches, axes);
		//		printf("Axes - ");
		//		for (int i = 0; i < joystick.rawGameController.AxisCount(); ++i) {
		//			printf("%d:%f ", i, axes[i]);
		//		}
		//		printf("Switches - ");
		//		for (int i = 0; i < joystick.rawGameController.SwitchCount(); ++i) {
		//			printf("%d:%d ", i, switches[i]);
		//		}
		//		printf("Buttons - ");
		//		for (int i = 0; i < joystick.rawGameController.ButtonCount(); ++i) {
		//			if (buttons[i]) printf("%d ", i);
		//		}
		//		puts("");
		//	}
		//}
		Sleep(16);
	}
	return 0;
}
