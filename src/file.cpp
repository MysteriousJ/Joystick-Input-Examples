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
	}
	free(input);
	printf("\n");
}

int main()
{
	HANDLE device = 0;
	HIDP_CAPS caps = {0};
	_HIDP_PREPARSED_DATA* preparseData = 0;
	BYTE* inputReportBuffer = 0;

	// Enumerate devices and pick the first "joystick" or "game controller"
	unsigned int deviceCount = 0;
	GetRawInputDeviceList(0, &deviceCount, sizeof(RAWINPUTDEVICELIST));
	RAWINPUTDEVICELIST* deviceLists = (RAWINPUTDEVICELIST*)malloc(sizeof(RAWINPUTDEVICELIST) * deviceCount);
	GetRawInputDeviceList(deviceLists, &deviceCount, sizeof(RAWINPUTDEVICELIST));
	for (unsigned int i = 0; i < deviceCount; ++i) {
		RID_DEVICE_INFO deviceInfo;
		UINT size = sizeof(deviceInfo);
		GetRawInputDeviceInfoA(deviceLists[i].hDevice, RIDI_DEVICEINFO, &deviceInfo, &size);
		if (deviceInfo.dwType == RIM_TYPEHID && deviceInfo.hid.usUsagePage == 1) {
			if (deviceInfo.hid.usUsage == 4 || deviceInfo.hid.usUsage == 5)
			{
				WCHAR deviceName[1024] = {0};
				UINT deviceNameLength = sizeof(deviceName)/sizeof(*deviceName);
				bool gotName = GetRawInputDeviceInfoW(deviceLists[i].hDevice, RIDI_DEVICENAME, deviceName, &deviceNameLength) > 0;

				GetRawInputDeviceInfo(deviceLists[i].hDevice, RIDI_PREPARSEDDATA, 0, &size);
				preparseData = (_HIDP_PREPARSED_DATA*)malloc(size);
				bool gotPreparseInfo = GetRawInputDeviceInfo(deviceLists[i].hDevice, RIDI_PREPARSEDDATA, preparseData, &size) > 0;

				if (gotName && gotPreparseInfo) {
					device = CreateFileW(deviceName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
					HidP_GetCaps(preparseData, &caps);
					inputReportBuffer = (BYTE*)malloc(caps.InputReportByteLength);
				}
				free(preparseData);
				break;
			}
		}
	}

	bool run = true;
	while (run) 
	{
		if (device) {
			DWORD bytesReadCount;
			ReadFile(device, inputReportBuffer, caps.InputReportByteLength, &bytesReadCount, NULL);
			for (unsigned int i=0; i<bytesReadCount; ++i) {
				printf("%02X ", inputReportBuffer[i]);
			}
			printf("\n");

			//printf("Values: ");
			//HIDP_VALUE_CAPS* valueCaps = (HIDP_VALUE_CAPS*)malloc(caps.NumberInputValueCaps * sizeof(HIDP_VALUE_CAPS));
			//HidP_GetValueCaps(HidP_Input, valueCaps, &caps.NumberInputValueCaps, preparseData);
			//for (unsigned int i = 0; i < caps.NumberInputValueCaps; ++i)
			//{
			//	ULONG value;
			//	HidP_GetUsageValue(HidP_Input, valueCaps[i].UsagePage, 0, valueCaps[i].Range.UsageMin, &value, preparseData, (PCHAR)inputReportBuffer, bytesReadCount);
			//	printf("%d:%5d ", i, value);
			//}
			//
			//printf("Buttons: ");
			//HIDP_BUTTON_CAPS* buttonCaps = (HIDP_BUTTON_CAPS*)malloc(caps.NumberInputButtonCaps * sizeof(HIDP_BUTTON_CAPS));
			//HidP_GetButtonCaps(HidP_Input, buttonCaps, &caps.NumberInputButtonCaps, preparseData);
			//for (unsigned int i = 0; i < caps.NumberInputButtonCaps; ++i)
			//{
			//	unsigned int buttonCount = buttonCaps->Range.UsageMax - buttonCaps->Range.UsageMin + 1;
			//	USAGE* usage = (USAGE*)malloc(sizeof(USAGE) * buttonCount);
			//	HidP_GetUsages(HidP_Input, buttonCaps[i].UsagePage, 0, usage, (PULONG)&buttonCount, preparseData, (PCHAR)inputReportBuffer, bytesReadCount);
			//	for (unsigned int buttonIndex=0; buttonIndex < buttonCount; ++buttonIndex) {
			//		printf("%d ", usage[buttonIndex]);
			//	}
			//}
			printf("\n");
		}
		Sleep(16);
	}

	return 0;
}

