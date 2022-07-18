#define QWORD uint64_t
#include <stdio.h>
#include <windows.h>
#include <Dbt.h>
#include <hidsdi.h>
#include <hidpi.h>
#include <inttypes.h>

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

int main()
{
	WNDCLASS wnd = {0};
	wnd.hInstance = GetModuleHandle(0);
	wnd.lpfnWndProc = DefWindowProc;
	wnd.lpszClassName = TEXT("Raw input test");
	RegisterClass(&wnd);
	HWND hwnd = CreateWindow(wnd.lpszClassName, 0, 0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, wnd.hInstance, 0);

	RAWINPUTDEVICE deviceList[2] = {0};
	
	deviceList[0].usUsagePage = 0x01;
	deviceList[0].usUsage = 0x04;
	deviceList[0].dwFlags = RIDEV_INPUTSINK;
	deviceList[0].hwndTarget = hwnd;

	deviceList[1].usUsagePage = 0x01;
	deviceList[1].usUsage = 0x05;
	deviceList[1].dwFlags = RIDEV_INPUTSINK;
	deviceList[1].hwndTarget = hwnd;

	UINT deviceCount = sizeof(deviceList)/sizeof(*deviceList);
	RegisterRawInputDevices(deviceList, deviceCount, sizeof(RAWINPUTDEVICE));

	while (1)
	{
		UINT size;
		GetRawInputBuffer(0, &size, sizeof(RAWINPUTHEADER));
		size *= 8;
		RAWINPUT* rawInput = (RAWINPUT*)malloc(size);
		UINT inputCount = GetRawInputBuffer(rawInput, &size, sizeof(RAWINPUTHEADER));
		if (inputCount != -1)
		{
			RAWINPUT* nextInput = rawInput;
			for (UINT i=0; i<inputCount; ++i)
			{
				printRawInputData(nextInput);
				nextInput = NEXTRAWINPUTBLOCK(nextInput);
			}
		}
		free(rawInput);
		Sleep(16);
	}

	return 0;
}
