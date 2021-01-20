#include <stdio.h>
#include <Windows.h>
#include <Dbt.h>
#include <hidsdi.h>
#include <hidpi.h>

void parsePS4Controller(BYTE rawData[], DWORD dataSize)
{
	if (dataSize > 6)
	{
		const unsigned char wiredCode = 0x01;
		const unsigned char wirelessCode = 0x18;
		unsigned int offset = 0;
		if (rawData[0]==wirelessCode) {
			offset = 2;
			if (rawData[1]==0x40) {
				return;
			}
		}
		else if (rawData[0]!=wiredCode) {
			return;
		}

		for (int i=0; i<16; ++i) {
			printf("%2X-", rawData[i]);
		}
		printf("\n");

		unsigned char leftStickX   = *(rawData + offset + 1);
		unsigned char leftStickY   = *(rawData + offset + 2);
		unsigned char rightStickX  = *(rawData + offset + 3);
		unsigned char rightStickY  = *(rawData + offset + 4);
		unsigned char leftTrigger  = *(rawData + offset + 8);
		unsigned char rightTrigger = *(rawData + offset + 9);

		unsigned char buttons1 = *(rawData + offset + 5);
		unsigned char buttons2 = *(rawData + offset + 6);
		unsigned char buttons3 = *(rawData + offset + 7);

		unsigned char squareButton   = 1 & (buttons1 >> 4);
		unsigned char xButton        = 1 & (buttons1 >> 5);
		unsigned char circleButton   = 1 & (buttons1 >> 6);
		unsigned char triangleButton = 1 & (buttons1 >> 7);
		unsigned char dpad           = 0b1111 & buttons1;
		unsigned char l1Button       = 1 & (buttons2 >> 0);
		unsigned char r1Button       = 1 & (buttons2 >> 1);
		unsigned char shareButton    = 1 & (buttons2 >> 4);
		unsigned char optionsButton  = 1 & (buttons2 >> 5);
		unsigned char l3Button       = 1 & (buttons2 >> 6);
		unsigned char r3Button       = 1 & (buttons2 >> 7);
		unsigned char psButton       = 1 & (buttons3 >> 0);
		unsigned char touchPadButton = 1 & (buttons3 >> 1);

		//out->currentInputs[PS4InputSquare] = (float)squareButton;
		//out->currentInputs[PS4InputX] = (float)xButton;
		//out->currentInputs[PS4InputCircle] = (float)circleButton;
		//out->currentInputs[PS4InputTriangle] = (float)triangleButton;
		//out->currentInputs[PS4InputL1] = (float)l1Button;
		//out->currentInputs[PS4InputR1] = (float)r1Button;
		//out->currentInputs[PS4InputShare] = (float)shareButton;
		//out->currentInputs[PS4InputOptions] = (float)optionsButton;
		//out->currentInputs[PS4InputL3] = (float)l3Button;
		//out->currentInputs[PS4InputR3] = (float)r3Button;
		//out->currentInputs[PS4InputPS] = (float)psButton;
		//out->currentInputs[PS4InputTouchPadButton]  = (float)touchPadButton;
		//out->currentInputs[PS4InputLeftStickLeft]   = -(leftStickX /255.0f * 2 -1);
		//out->currentInputs[PS4InputLeftStickRight]  =  (leftStickX /255.0f * 2 -1);
		//out->currentInputs[PS4InputLeftStickUp]     = -(leftStickY /255.0f * 2 -1);
		//out->currentInputs[PS4InputLeftStickDown]   =  (leftStickY /255.0f * 2 -1);
		//out->currentInputs[PS4InputRightStickLeft]  = -(rightStickX/255.0f * 2 -1);
		//out->currentInputs[PS4InputRightStickRight] =  (rightStickX/255.0f * 2 -1);
		//out->currentInputs[PS4InputRightStickUp]    = -(rightStickY/255.0f * 2 -1);
		//out->currentInputs[PS4InputRightStickDown]  =  (rightStickY/255.0f * 2 -1);
		//out->currentInputs[PS4InputL2]  = leftTrigger /255.0f;
		//out->currentInputs[PS4InputR2] = rightTrigger/255.0f;
		//out->currentInputs[PS4InputDpadUp]    = (dpad==0 || dpad==1 || dpad==7)? 1.0f : 0.0f;
		//out->currentInputs[PS4InputDpadRight] = (dpad==1 || dpad==2 || dpad==3)? 1.0f : 0.0f; 
		//out->currentInputs[PS4InputDpadDown]  = (dpad==3 || dpad==4 || dpad==5)? 1.0f : 0.0f; 
		//out->currentInputs[PS4InputDpadLeft]  = (dpad==5 || dpad==6 || dpad==7)? 1.0f : 0.0f;
		//out->type = JoystickTypePS4;
	}
}

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

		GetRawInputDeviceInfo(input->header.hDevice, RIDI_PREPARSEDDATA, 0, &size);
		_HIDP_PREPARSED_DATA* data = (_HIDP_PREPARSED_DATA*)malloc(size);
		bool gotPreparsedData = GetRawInputDeviceInfo(input->header.hDevice, RIDI_PREPARSEDDATA, data, &size) > 0;

		WCHAR deviceName[1024] = {0};
		UINT deviceNameLength = sizeof(deviceName)/sizeof(*deviceName);
		bool gotName = GetRawInputDeviceInfoW(input->header.hDevice, RIDI_DEVICENAME, deviceName, &deviceNameLength) > 0;

		if (gotInfo && gotPreparsedData && gotName)
		{
			if (deviceInfo.hid.dwProductId == 2508) {
				parsePS4Controller(input->data.hid.bRawData, input->data.hid.dwSizeHid);
			}
			free(data);
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

void setJoystickForceFeedback(wchar_t* deviceName, float leftForceFeedback, float rightForceFeedback)
{
	HANDLE hidDevice = CreateFileW(deviceName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	//HANDLE hidDevice = CreateFileW(joysticks->states[joystickIndex].deviceName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	if (hidDevice != INVALID_HANDLE_VALUE) {
		// USB
		//unsigned char output[32] = {0};
		//output[0] = 0x05;
		//output[1] = 0xFF;
		//output[4] = (unsigned char)(joysticks->states[joystickIndex].lightRumble*255);
		//output[5] = (unsigned char)(joysticks->states[joystickIndex].heavyRumble*255);
		//output[6] = (unsigned char)(joysticks->states[joystickIndex].ledRed*255);
		//output[7] = (unsigned char)(joysticks->states[joystickIndex].ledGreen*255);
		//output[8] = (unsigned char)(joysticks->states[joystickIndex].ledBlue*255);
		// Bluetooth
		//unsigned char output[78] = {0};
		//output[0] = 0x11;
		//output[1] = 0xb0;
		//output[2] = 0x0f;
		//output[6] = (unsigned char)(joysticks->states[joystickIndex].lightRumble*255);
		//output[7] = (unsigned char)(joysticks->states[joystickIndex].heavyRumble*255);
		//output[8] = (unsigned char)(joysticks->states[joystickIndex].ledRed*255);
		//output[9] = (unsigned char)(joysticks->states[joystickIndex].ledGreen*255);
		//output[10] = (unsigned char)(joysticks->states[joystickIndex].ledBlue*255);
		OVERLAPPED ovelappedInfo = {0};
		//WriteFile(hidDevice, output, sizeof(output), 0, &ovelappedInfo);
		//HidD_SetOutputReport(hidDevice, output, sizeof(output));
		//printWindowsErrors();
		CloseHandle(hidDevice);
	}
}

int main()
{
	// Create a window, as we need a window precedure to recieve raw input
	WNDCLASSA wnd = { 0 };
	wnd.hInstance = GetModuleHandle(0);
	wnd.lpfnWndProc = WindowProcedure;
	wnd.lpszClassName = "RawInputEventWindow";
	RegisterClassA(&wnd);
	HWND hwnd = CreateWindowA(wnd.lpszClassName, 0, 0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, wnd.hInstance, 0);

	// Register devices
	RAWINPUTDEVICE rid;
	rid.usUsagePage = 0x01;
	rid.usUsage = 0;
	rid.dwFlags = RIDEV_INPUTSINK | RIDEV_PAGEONLY | RIDEV_DEVNOTIFY;
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

