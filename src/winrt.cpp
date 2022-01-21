#include <list>
#include <winrt/Windows.Gaming.Input.h>
#include <winrt/Windows.Foundation.h>
#include <ppl.h>
#include <Windows.h>

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
	wnd.hCursor = LoadCursorA(0, MAKEINTRESOURCEA(32512));
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
		Sleep(16);
		MSG message;
		while (PeekMessageA(&message, 0, 0, 0, true)) {
			TranslateMessage(&message);
			DispatchMessageA(&message);
		}

		concurrency::critical_section::scoped_lock {controllerListLock};
		for (auto& joystick : joysticks)
		{
			if (joystick.connected)
			{
				wprintf(L"Joystick - %s, ", joystick.rawGameController.DisplayName().c_str());
		
				bool buttons[64];
				winrt::Windows::Gaming::Input::GameControllerSwitchPosition switches[64];
				double axes[64];
		
				joystick.rawGameController.GetCurrentReading(buttons, switches, axes);
				printf("Axes - ");
				for (int i = 0; i < joystick.rawGameController.AxisCount(); ++i) {
					printf("%d:%f ", i, axes[i]);
				}
				printf("Switches - ");
				for (int i = 0; i < joystick.rawGameController.SwitchCount(); ++i) {
					printf("%d:%d ", i, switches[i]);
				}
				printf("Buttons - ");
				for (int i = 0; i < joystick.rawGameController.ButtonCount(); ++i) {
					if (buttons[i]) printf("%d ", i);
				}
				puts("");
			}
		}
	}
	return 0;
}
