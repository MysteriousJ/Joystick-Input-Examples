#include <stdio.h>
#include <Windows.h>
#include <Dbt.h>
#include <hidsdi.h>
#include <hidpi.h>

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_INPUT) {
		unsigned int size = 0;
		unsigned int errorCode = GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER));
		RAWINPUT* input = (RAWINPUT*)malloc(size);
		unsigned int structsWritten = GetRawInputData((HRAWINPUT)lParam, RID_INPUT, input, &size, sizeof(RAWINPUTHEADER));
		if (structsWritten == -1) {
			DWORD errorCode = GetLastError();
			printf("Error code:%d\n", errorCode);
		}
		else {
			GetRawInputDeviceInfo(input->header.hDevice, RIDI_PREPARSEDDATA, 0, &size);
			_HIDP_PREPARSED_DATA* data = (_HIDP_PREPARSED_DATA*)malloc(size);
			GetRawInputDeviceInfo(input->header.hDevice, RIDI_PREPARSEDDATA, data, &size);

			HIDP_CAPS caps;
			HidP_GetCaps(data, &caps);

			printf("Values: ");
			HIDP_VALUE_CAPS* valueCaps = (HIDP_VALUE_CAPS*)malloc(caps.NumberInputValueCaps * sizeof(HIDP_VALUE_CAPS));
			HidP_GetValueCaps(HidP_Input, valueCaps, &caps.NumberInputValueCaps, data);
			for (unsigned int i = 0; i < caps.NumberInputValueCaps; ++i)
			{
				ULONG value;
				HidP_GetUsageValue(HidP_Input, valueCaps[i].UsagePage, 0, valueCaps[i].Range.UsageMin, &value, data, (PCHAR)input->data.hid.bRawData, input->data.hid.dwSizeHid);
				printf("%d:%5d ", i, value);
			}

			printf("Buttons: ");
			HIDP_BUTTON_CAPS* buttonCaps = (HIDP_BUTTON_CAPS*)malloc(caps.NumberInputButtonCaps * sizeof(HIDP_BUTTON_CAPS));
			HidP_GetButtonCaps(HidP_Input, buttonCaps, &caps.NumberInputButtonCaps, data);
			for (unsigned int i = 0; i < caps.NumberInputButtonCaps; ++i)
			{
				unsigned int buttonCount = buttonCaps->Range.UsageMax - buttonCaps->Range.UsageMin + 1;
				USAGE* usage = (USAGE*)malloc(sizeof(USAGE) * buttonCount);
				HidP_GetUsages(HidP_Input, buttonCaps[i].UsagePage, 0, usage, (PULONG)&buttonCount, data, (PCHAR)input->data.hid.bRawData, input->data.hid.dwSizeHid);
				for (unsigned int buttonIndex=0; buttonIndex < buttonCount; ++buttonIndex) {
					printf("%d ", usage[buttonIndex]);
				}
			}
		}
		free(input);
		printf("\n");

		return 0;
	}
	if (msg == WM_DEVICECHANGE) {
		return 0;
	}
	if (msg == WM_DESTROY) {
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

int main()
{
	// Create a window, as we need a window precedure to recieve raw input
	WNDCLASS wnd = { 0 };
	wnd.hInstance = GetModuleHandle(0);
	wnd.lpfnWndProc = WindowProcedure;
	wnd.lpszClassName = TEXT("Raw input test");
	RegisterClass(&wnd);
	HWND hwnd = CreateWindow(wnd.lpszClassName, 0, 0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, wnd.hInstance, 0);

	// Register window to recieve WM_DEVICECHANGE when joysticks are plugged in or taken out
	DEV_BROADCAST_DEVICEINTERFACE notificationFilter = { sizeof(DEV_BROADCAST_DEVICEINTERFACE), DBT_DEVTYP_DEVICEINTERFACE };
	RegisterDeviceNotification(0, &notificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE);

	// Enumerate devices
	unsigned int deviceCount = 0;
	GetRawInputDeviceList(0, &deviceCount, sizeof(RAWINPUTDEVICELIST));
	RAWINPUTDEVICELIST* deviceLists = (RAWINPUTDEVICELIST*)malloc(sizeof(RAWINPUTDEVICELIST) * deviceCount);
	GetRawInputDeviceList(deviceLists, &deviceCount, sizeof(RAWINPUTDEVICELIST));
	for (unsigned int i = 0; i < deviceCount; ++i) {
		unsigned int bufferSize = 100;
		char data[100] = { 0 };
		GetRawInputDeviceInfoA(deviceLists[i].hDevice, RIDI_DEVICENAME, (void*)data, &bufferSize);
		char* deviceName = (char*)data;
		printf("%s\n", deviceName);
	}

	// Register devices
	// Xbox and PS4 report as usage=5(gamepads), magic joy box reports usage=4(joysticks)
	RAWINPUTDEVICE rid;
	rid.usUsagePage = 0x01;
	rid.usUsage = 0x05;
	rid.dwFlags = RIDEV_INPUTSINK;
	rid.hwndTarget = hwnd;
	if (!RegisterRawInputDevices(&rid, 1, sizeof(RAWINPUTDEVICE))) {
		printf("Failed to register device\n");
	}
	else {
		printf("Registered device\n");
	}

	bool run = true;
	while (run) 
	{
		MSG msg;
		while (GetMessage(&msg, NULL, 0, 0)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT) {
				run = false;
			}
			
		}
	}

	return 0;
}

