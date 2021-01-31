#include <stdio.h>
#include <Windows.h>
#include <Dbt.h>
#include <hidsdi.h>
#include <hidpi.h>

void printRawInputData(RAWINPUT* input)
{
	UINT size;
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
		for (USHORT i = 0; i < caps.NumberInputValueCaps; ++i)
		{
			ULONG value;
			HidP_GetUsageValue(HidP_Input, valueCaps[i].UsagePage, 0, valueCaps[i].Range.UsageMin, &value, data, (PCHAR)input->data.hid.bRawData, input->data.hid.dwSizeHid);
			printf("%d:%5d ", i, value);
		}
		free(valueCaps);

		printf("Buttons: ");
		HIDP_BUTTON_CAPS* buttonCaps = (HIDP_BUTTON_CAPS*)malloc(caps.NumberInputButtonCaps * sizeof(HIDP_BUTTON_CAPS));
		HidP_GetButtonCaps(HidP_Input, buttonCaps, &caps.NumberInputButtonCaps, data);
		for (USHORT i = 0; i < caps.NumberInputButtonCaps; ++i)
		{
			ULONG usageCount = buttonCaps->Range.UsageMax - buttonCaps->Range.UsageMin + 1;
			USAGE* usages = (USAGE*)malloc(sizeof(USAGE) * usageCount);
			HidP_GetUsages(HidP_Input, buttonCaps[i].UsagePage, 0, usages, &usageCount, data, (PCHAR)input->data.hid.bRawData, input->data.hid.dwSizeHid);
			for (ULONG usageIndex=0; usageIndex < usageCount; ++usageIndex) {
				printf("%d ", usages[usageIndex]);
			}
			free(usages);
		}
		free(buttonCaps);
		printf("\n");
	}
	free(data);
}

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
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
	//RegisterRawInputDevices(deviceList, deviceCount, sizeof(RAWINPUTDEVICE));

	// Message loop
	while (1)
	{
		UINT size;
		GetRawInputBuffer(0, &size, sizeof(RAWINPUTHEADER));
		size *= 8;
		RAWINPUT* rawInput = (RAWINPUT*)malloc(size);
		UINT inputCount = GetRawInputBuffer(rawInput, &size, sizeof(RAWINPUTHEADER));
		RAWINPUT* nextInput = rawInput;
		if (inputCount > 0 && inputCount != -1)
		{
			UINT inputIndex = 0;
			while (inputIndex < inputCount)
			{
				printRawInputData(nextInput);
				nextInput = NEXTRAWINPUTBLOCK(nextInput);
				++inputIndex;
			}
		}
		DefRawInputProc(&rawInput, inputCount, sizeof(RAWINPUTHEADER));
		Sleep(16);
	}

	return 0;
}
