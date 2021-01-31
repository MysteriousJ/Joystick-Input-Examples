#include <stdio.h>
#include <Windows.h>
#include <Dbt.h>
#include <hidsdi.h>
#include <hidpi.h>
#include <inttypes.h>

struct OutputData
{
	BYTE buffer[128];
	HANDLE file;
	OVERLAPPED overlapped;
};

static DWORD ps_crc32(const BYTE* data, DWORD size, DWORD initial)
{
	DWORD r = ~initial;
	for (DWORD i=0; i<size; i++)
	{
		r ^= *data++;
		for (int i = 0; i < 8; i++)
		{
			r = (r >> 1) ^ (r & 1 ? 0xedb88320 : 0);
		}
	}
	return ~r;
}

void updatePS4Controller(BYTE rawData[], DWORD byteCount, WCHAR* deviceName, OutputData* outputData)
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

	BYTE battery = rawData[offset + 12];
	int16_t gyroX  = rawData[offset + 13] | (rawData[offset + 14] << 8);
	int16_t gyroY  = *(int8_t*)(rawData + offset + 15);
	int16_t gyroZ  = *(int8_t*)(rawData + offset + 27);
	int16_t accelX = rawData[offset + 19] | (rawData[offset + 20] << 8);
	int16_t accelY = *(int8_t*)(rawData + offset + 21);
	int16_t accelZ = *(int8_t*)(rawData + offset + 23);
	int16_t touch1X = ((rawData[offset + 37] & 0x0F) << 8) | rawData[offset + 36];
	int16_t touch1Y = ((rawData[offset + 37]) >> 4) | (rawData[offset + 38] << 4);
	printf("Battery:%3d GyroX:%4d GyroY:%4d GyroZ:%4d AccelX:% 6d AccelY:%4d AccelZ:%4d Touch1X:%4d Touch1Y:%4d", battery, gyroX, gyroY, gyroZ, accelX, accelY, accelZ, touch1X, touch1Y);

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
	int headerSize = 0;
	int outputByteCount = 0;
	if (byteCount == usbInputByteCount)
	{
		outputByteCount = 32;
		outputData->buffer[0] = 0x05;
		outputData->buffer[1] = 0xFF;
	}
	if (byteCount == bluetoothInputByteCount)
	{
		outputByteCount = 78;
		outputData->buffer[0] = 0xA2; // Header - Bluetooth HID report type: data/output
		outputData->buffer[1] = 0x11;
		outputData->buffer[2] = 0XC0;
		outputData->buffer[4] = 0x07;
		offset = 2;
		headerSize = 1;
	}
	outputData->buffer[4 + offset + headerSize] = rightTrigger;
	outputData->buffer[5 + offset + headerSize] = leftTrigger;
	outputData->buffer[6 + offset + headerSize] = leftStickX;
	outputData->buffer[7 + offset + headerSize] = leftStickY;
	outputData->buffer[8 + offset + headerSize] = rightStickY;

	if (byteCount == bluetoothInputByteCount)
	{
		DWORD crc = ps_crc32((BYTE*)outputData->buffer, 75, 0);
		CopyMemory((BYTE*)outputData->buffer+75, &crc, sizeof(crc));

		//uint32_t polynomial = 0x04C11DB7;
		//uint32_t crc = -1;
		//for (uint32_t byteIndex=74; byteIndex<75; --byteIndex)
		//{
		//	crc ^= outputData->buffer[byteIndex] << 24;
		//	for (uint32_t i=0; i<8; ++i)
		//	{
		//		if (crc & 0x80000000) crc = (crc<<1)^polynomial;
		//		else crc <<= 1;
		//	}
		//}
		//crc = ~crc;
		//CopyMemory((BYTE*)outputData->buffer+75, &crc, sizeof(crc));
	}

	DWORD bytesTransferred;
	bool finishedLastOutput = GetOverlappedResult(outputData->file, &outputData->overlapped, &bytesTransferred, false);
	if (finishedLastOutput)
	{
		if (outputData->file) CloseHandle(outputData->file);
		outputData->file = CreateFileW(deviceName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
		if (outputData->file != INVALID_HANDLE_VALUE)
		{
			WriteFile(outputData->file, (void*)(outputData->buffer+headerSize), outputByteCount, 0, &outputData->overlapped);
		}
	}
}

void updateJoyCon(BYTE rawData[], DWORD byteCount)
{
	for (DWORD i=0; i<byteCount; ++i)
	{
		printf("%2X ", rawData[i]);
	}
	printf("\n");
}

void updateRawInput(LPARAM lParam, OutputData* outputData)
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
			updatePS4Controller(input->data.hid.bRawData, input->data.hid.dwSizeHid, deviceName, outputData);
			//updateJoyCon(input->data.hid.bRawData, input->data.hid.dwSizeHid);
		}
	}
	free(input);
}

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	OutputData* outputData = (OutputData*)GetPropA(hwnd, "userData");
	if (msg == WM_INPUT) {
		LARGE_INTEGER start_time;
		QueryPerformanceCounter(&start_time);

		updateRawInput(lParam, outputData);

		LARGE_INTEGER end_time;
		QueryPerformanceCounter(&end_time);

		LARGE_INTEGER ticksPerSecond;
		QueryPerformanceFrequency(&ticksPerSecond);

		printf("%fms ", (double)(end_time.QuadPart-start_time.QuadPart)/(double)ticksPerSecond.QuadPart*1000);
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
	OutputData outputData = {0};

	SetPropA(hwnd, "userData", &outputData);
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

