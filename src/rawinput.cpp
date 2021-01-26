#include <stdio.h>
#include <Windows.h>
#include <Dbt.h>
#include <hidsdi.h>
#include <hidpi.h>

void printRawInputData(LPARAM lParam)
{
	UINT size = 0;
	UINT errorCode = GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER));
	RAWINPUT* input = (RAWINPUT*)malloc(size);
	UINT structsWritten = GetRawInputData((HRAWINPUT)lParam, RID_INPUT, input, &size, sizeof(RAWINPUTHEADER));
	if (structsWritten == -1) {
		DWORD errorCode = GetLastError();
		printf("Error code:%d\n", errorCode);
	}
	else {
		GetRawInputDeviceInfo(input->header.hDevice, RIDI_PREPARSEDDATA, 0, &size);
		_HIDP_PREPARSED_DATA* data = (_HIDP_PREPARSED_DATA*)malloc(size);
		UINT bytesWritten = GetRawInputDeviceInfo(input->header.hDevice, RIDI_PREPARSEDDATA, data, &size);
		if (bytesWritten > 0)
		{
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
			free(valueCaps);

			printf("Buttons: ");
			HIDP_BUTTON_CAPS* buttonCaps = (HIDP_BUTTON_CAPS*)malloc(caps.NumberInputButtonCaps * sizeof(HIDP_BUTTON_CAPS));
			HidP_GetButtonCaps(HidP_Input, buttonCaps, &caps.NumberInputButtonCaps, data);
			for (unsigned int i = 0; i < caps.NumberInputButtonCaps; ++i)
			{
				unsigned int buttonCount = buttonCaps->Range.UsageMax - buttonCaps->Range.UsageMin + 1;
				USAGE* usages = (USAGE*)malloc(sizeof(USAGE) * buttonCount);
				HidP_GetUsages(HidP_Input, buttonCaps[i].UsagePage, 0, usages, (PULONG)&buttonCount, data, (PCHAR)input->data.hid.bRawData, input->data.hid.dwSizeHid);
				for (unsigned int buttonIndex=0; buttonIndex < buttonCount; ++buttonIndex) {
					printf("%d ", usages[buttonIndex]);
				}
				free(usages);
			}
			free(buttonCaps);
		}
		free(data);
	}
	free(input);
	printf("\n");
}

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_INPUT) {
		printRawInputData(lParam);
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

	// Register devices
	RAWINPUTDEVICE deviceDesc;
	deviceDesc.usUsagePage = 0x01;
	deviceDesc.dwFlags = RIDEV_INPUTSINK;
	deviceDesc.hwndTarget = hwnd;

	RAWINPUTDEVICE deviceList[2];
	deviceDesc.usUsage = 0x04;
	deviceList[0] = deviceDesc;
	deviceDesc.usUsage = 0x05;
	deviceList[1] = deviceDesc;

	UINT deviceCount = sizeof(deviceList)/sizeof(*deviceList);
	RegisterRawInputDevices(deviceList, deviceCount, sizeof(RAWINPUTDEVICE));

	// Message loop
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

