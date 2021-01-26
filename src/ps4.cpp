#include <stdio.h>
#include <Windows.h>
#include <Dbt.h>
#include <hidsdi.h>
#include <hidpi.h>

struct OutputThreadData
{
	enum Type {
		type_writeFile, type_setOutputReport
	};

	volatile DWORD byteCount;
	volatile BYTE buffer[128];
	volatile Type type;
	WCHAR deviceName[128];
	CONDITION_VARIABLE conditionVariable;
	CRITICAL_SECTION criticalSection;
};

int outputThread(void* param)
{
	OutputThreadData* data = (OutputThreadData*)param;
	EnterCriticalSection(&data->criticalSection);
	while (1)
	{
		SleepConditionVariableCS(&data->conditionVariable, &data->criticalSection, INFINITE);
		HANDLE file = CreateFileW(data->deviceName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
		if (data->type == OutputThreadData::type_writeFile) {
			DWORD bytesWritten;
			WriteFile(file, (void*)data->buffer, data->byteCount, &bytesWritten, 0);
		}
		if (data->type == OutputThreadData::type_setOutputReport) {
			HidD_SetOutputReport(file, (void*)data->buffer, data->byteCount);
		}
		CloseHandle(file);
	}
	LeaveCriticalSection(&data->criticalSection);
	return 0;
}

void updatePS4Controller(BYTE rawData[], DWORD byteCount, OutputThreadData* outputThreadData)
{
	const DWORD usbInputByteCount = 64;
	const DWORD bluetoothInputByteCount = 547;

	for (unsigned int i=0; i<byteCount; ++i) {
		//printf("%02X ", rawData[i]);
	}

	int offset = 0;
	if (byteCount == bluetoothInputByteCount) offset = 2;

	BYTE leftStickX   = rawData[offset + 1];
	BYTE leftStickY   = rawData[offset + 2];
	BYTE rightStickX  = rawData[offset + 3];
	BYTE rightStickY  = rawData[offset + 4];
	BYTE leftTrigger  = rawData[offset + 8];
	BYTE rightTrigger = rawData[offset + 9];
	BYTE dpad         = 0b1111 & rawData[offset + 5];
	printf("DS4 - LX:%3d LY:%3d RX:%3d RY:%3d LT:%3d RT:%3d Dpad:%1d ", leftStickX, leftStickY, rightStickX, rightStickY, leftTrigger, rightTrigger, dpad);

	printf("Buttons: ");
	if (1 & (rawData[offset + 5] >> 4)) printf("Square ");
	if (1 & (rawData[offset + 5] >> 5)) printf("X ");
	if (1 & (rawData[offset + 5] >> 6)) printf("O ");
	if (1 & (rawData[offset + 5] >> 7)) printf("Triangle ");
	if (1 & (rawData[offset + 6] >> 0)) printf("L1 ");
	if (1 & (rawData[offset + 6] >> 1)) printf("R1 ");
	if (1 & (rawData[offset + 6] >> 2)) printf("L2 ");
	if (1 & (rawData[offset + 6] >> 3)) printf("R2 ");
	if (1 & (rawData[offset + 6] >> 4)) printf("Share ");
	if (1 & (rawData[offset + 6] >> 5)) printf("Options ");
	if (1 & (rawData[offset + 6] >> 6)) printf("L3 ");
	if (1 & (rawData[offset + 6] >> 7)) printf("R3 ");
	if (1 & (rawData[offset + 7] >> 0)) printf("PS ");
	if (1 & (rawData[offset + 7] >> 1)) printf("TouchPad ");
	printf("\n");

	// Ouput force-feedback and LED color
	offset = 0;
	if (byteCount == usbInputByteCount)
	{
		outputThreadData->type = OutputThreadData::type_writeFile;
		outputThreadData->byteCount = 32;
		outputThreadData->buffer[0] = 0x05;
		outputThreadData->buffer[1] = 0xFF;
	}
	if (byteCount == bluetoothInputByteCount)
	{
		outputThreadData->type = OutputThreadData::type_setOutputReport;
		outputThreadData->byteCount = 78;
		outputThreadData->buffer[0] = 0x11;
		outputThreadData->buffer[1] = 0XC0;
		outputThreadData->buffer[3] = 0x07;
		offset = 2;
	}
	outputThreadData->buffer[4 + offset] = rightTrigger;
	outputThreadData->buffer[5 + offset] = leftTrigger;
	outputThreadData->buffer[6 + offset] = leftStickX;
	outputThreadData->buffer[7 + offset] = leftStickY;
	outputThreadData->buffer[8 + offset] = rightStickY;
	WakeConditionVariable(&outputThreadData->conditionVariable);
}

void updateRawInput(LPARAM lParam, OutputThreadData* outputThreadData)
{
	UINT size = 0;
	UINT errorCode = GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER));
	RAWINPUT* input = (RAWINPUT*)malloc(size);
	if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, input, &size, sizeof(RAWINPUTHEADER)) > 0)
	{
		RID_DEVICE_INFO deviceInfo;
		UINT deviceInfoSize = sizeof(deviceInfo);
		bool gotInfo = GetRawInputDeviceInfo(input->header.hDevice, RIDI_DEVICEINFO, &deviceInfo, &deviceInfoSize) > 0;

		WCHAR deviceName[1024] = {0};
		UINT deviceNameLength = sizeof(deviceName)/sizeof(*deviceName);
		bool gotName = GetRawInputDeviceInfoW(input->header.hDevice, RIDI_DEVICENAME, deviceName, &deviceNameLength) > 0;

		if (gotInfo && gotName)
		{
			if (wcscmp(deviceName, outputThreadData->deviceName) != 0) {
				EnterCriticalSection(&outputThreadData->criticalSection);
				wcscpy_s(outputThreadData->deviceName, deviceName);
				LeaveCriticalSection(&outputThreadData->criticalSection);
			}
			updatePS4Controller(input->data.hid.bRawData, input->data.hid.dwSizeHid, outputThreadData);
		}
	}
	free(input);
}

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	OutputThreadData* outputThreadData = (OutputThreadData*)GetPropA(hwnd, "userData");
	if (msg == WM_INPUT) {
		updateRawInput(lParam, outputThreadData);
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

int main()
{
	// Create a window, as we need a window procedure to recieve raw input events
	WNDCLASSA wnd = {0};
	wnd.hInstance = GetModuleHandle(0);
	wnd.lpfnWndProc = WindowProcedure;
	wnd.lpszClassName = "RawInputEventWindow";
	RegisterClassA(&wnd);
	HWND hwnd = CreateWindowA(wnd.lpszClassName, 0, 0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, wnd.hInstance, 0);

	// Register devices
	RAWINPUTDEVICE rid;
	rid.usUsagePage = 0x01;
	rid.usUsage = 0x05;
	rid.dwFlags = RIDEV_INPUTSINK;
	rid.hwndTarget = hwnd;
	RegisterRawInputDevices(&rid, 1, sizeof(RAWINPUTDEVICE));

	// Startup output thread
	OutputThreadData outputThreadData = {0};
	InitializeConditionVariable(&outputThreadData.conditionVariable);
	InitializeCriticalSection(&outputThreadData.criticalSection);
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)outputThread, (LPVOID)&outputThreadData, 0, 0);

	SetPropA(hwnd, "userData", &outputThreadData);
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

