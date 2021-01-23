#include <stdio.h>
#include <Windows.h>
#include <Dbt.h>
#include <hidsdi.h>
#include <hidpi.h>

// Notes from DS4-windows
// Do output in its own thread.

void parsePS4Controller(BYTE rawData[], DWORD dataSize)
{
	unsigned int byteCount = 32;
	if (dataSize >= byteCount)
	{
		for (unsigned int i=0; i<byteCount; ++i) {
			printf("%02X ", rawData[i]);
		}
		printf("\n");
	}
}

HANDLE device = 0;
WCHAR productString[100] = {0};
WCHAR manufacturerString[100] = {0};
WCHAR serialNumberString[100] = {0};

void updateRawInput(LPARAM lParam)
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
			if (device == 0) {
				device = CreateFileW(deviceName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
			
				UINT productStringLength = sizeof(productString)/sizeof(*productString);
				HidD_GetProductString(device, productString, productStringLength);
			
				UINT manufacturerStringLength = sizeof(manufacturerString)/sizeof(*manufacturerString);
				HidD_GetManufacturerString(device, manufacturerString, manufacturerStringLength);
			
				UINT serialNumberStringLength = sizeof(serialNumberString)/sizeof(*serialNumberString);
				HidD_GetSerialNumberString(device, serialNumberString, serialNumberStringLength);
			}
			
			//wprintf(L"%s | product:%s | manufacturer:%s | serialNumber:%s | pid:%d | vid:%d | ", deviceName, productString, manufacturerString, serialNumberString, deviceInfo.hid.dwProductId, deviceInfo.hid.dwVendorId);
			// wired size: 64
			// wireless size: 547
			parsePS4Controller(input->data.hid.bRawData, input->data.hid.dwSizeHid);

			// USB
			if (input->data.hid.dwSizeHid == 64)
			{
				unsigned char output[32] = {0};
				output[0] = 0x05;
				output[1] = 0xFF;
				output[4] = 255;
				output[5] = 0;
				DWORD written;
				WriteFile(device, output, sizeof(output), &written, 0);
			}
			// Bluetooth
			if (input->data.hid.dwSizeHid == 547)
			{
				unsigned char output[78] = {0};
				output[0] = 0x11;
				output[1] = 0XC0;
				output[3] = 0x07;
				output[6] = 255;
				output[7] = 128;
				HidD_SetOutputReport(device, output, sizeof(output));
			}

		}
	}
	free(input);
}
LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_INPUT) {
		updateRawInput(lParam);
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

int main()
{
	// Create a window, as we need a window procedure to recieve raw input
	WNDCLASSA wnd = { 0 };
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

	while (1)
	{
		MSG msg;
		while (GetMessage(&msg, NULL, 0, 0)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

