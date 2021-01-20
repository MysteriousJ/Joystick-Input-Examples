#include <stdio.h>
#include <Windows.h>
#include <Xinput.h>
#include <Dbt.h>
#include <hidsdi.h>
#include <hidpi.h>

enum JoystickType
{
	JoystickTypeGeneric,
	JoystickTypeXbox,
	JoystickTypePS4
};

struct JoystickState
{
	static const unsigned int inputCount = 64;
	static const unsigned int maxNameLength = 128;

	bool connected;
	JoystickType type;
	float currentInputs[inputCount];
	float previousInputs[inputCount];
	wchar_t deviceName[maxNameLength];
	char productName[maxNameLength];

	// Outputs
	float lightRumble;
	float heavyRumble;
	float ledRed;
	float ledGreen;
	float ledBlue;
};

struct Joysticks
{
	unsigned int count;
	JoystickState* states;
	HWND hwnd;
};

enum XboxInputs
{
	XboxInputA,
	XboxInputB,
	XboxInputX,
	XboxInputY,
	XboxInputDpadLeft,
	XboxInputDpadRight,
	XboxInputDpadUp,
	XboxInputDpadDown,
	XboxInputLeftBumper,
	XboxInputRightBumper,
	XboxInputLeftStickButton,
	XboxInputRightStickButton,
	XboxInputBack,
	XboxInputStart,
	XboxInputLeftTrigger,
	XboxInputRightTrigger,
	XboxInputLeftStickLeft,
	XboxInputLeftStickRight,
	XboxInputLeftStickUp,
	XboxInputLeftStickDown,
	XboxInputRightStickLeft,
	XboxInputRightStickRight,
	XboxInputRightStickUp,
	XboxInputRightStickDown
};

const char* xboxInputNames[] = {
	"A",
	"B",
	"X",
	"Y",
	"Left",
	"Right",
	"Up",
	"Down",
	"LB",
	"RB",
	"LS",
	"RS",
	"Back",
	"Start",
	"Left Trigger",
	"Right Trigger",
	"Left Stick Left",
	"Left Stick Right",
	"Left Stick Up",
	"Left Stick Down",
	"Right Stick Left",
	"Right Stick Right",
	"Right Stick Up",
	"Right Stick Down",
};

enum PS4Inputs {
	PS4InputSquare,
	PS4InputX,
	PS4InputCircle,
	PS4InputTriangle,
	PS4InputL1,
	PS4InputR1,
	PS4InputL3,
	PS4InputR3,
	PS4InputOptions,
	PS4InputShare,
	PS4InputPS,
	PS4InputTouchPadButton,
	PS4InputLeftStickLeft,
	PS4InputLeftStickRight,
	PS4InputLeftStickUp,
	PS4InputLeftStickDown,
	PS4InputRightStickLeft,
	PS4InputRightStickRight,
	PS4InputRightStickUp,
	PS4InputRightStickDown,
	PS4InputL2,
	PS4InputR2,
	PS4InputDpadLeft,
	PS4InputDpadRight,
	PS4InputDpadUp,
	PS4InputDpadDown
};

const char* ps4InputNames[] = {
	"Square",
	"X",
	"Circle",
	"Triangle",
	"L1",
	"R1",
	"L3",
	"R3",
	"Options",
	"Share",
	"PlayStation Button",
	"Touch Pad Button",
	"Left Stick Left",
	"Left Stick Right",
	"Left Stick Up",
	"Left Stick Down",
	"Right Stick Left",
	"Right Stick Right",
	"Right Stick Up",
	"Right Stick Down",
	"L2",
	"R2",
	"Dpad Left",
	"Dpad Right",
	"Dpad Up",
	"Dpad Down",
};

enum GenericInputs {
	GenericInputButton0,
	GenericInputButton1,
	GenericInputButton2,
	GenericInputButton3,
	GenericInputButton4,
	GenericInputButton5,
	GenericInputButton6,
	GenericInputButton7,
	GenericInputButton8,
	GenericInputButton9,
	GenericInputButton10,
	GenericInputButton11,
	GenericInputButton12,
	GenericInputButton13,
	GenericInputButton14,
	GenericInputButton15,
	GenericInputButton16,
	GenericInputButton17,
	GenericInputButton18,
	GenericInputButton19,
	GenericInputButton20,
	GenericInputButton21,
	GenericInputButton22,
	GenericInputButton23,
	GenericInputButton24,
	GenericInputButton25,
	GenericInputButton26,
	GenericInputButton27,
	GenericInputButton28,
	GenericInputButton29,
	GenericInputButton30,
	GenericInputButton31,
	GenericInputAxis0Positive,
	GenericInputAxis0Negative,
	GenericInputAxis1Positive,
	GenericInputAxis1Negative,
	GenericInputAxis2Positive,
	GenericInputAxis2Negative,
	GenericInputAxis3Positive,
	GenericInputAxis3Negative,
	GenericInputAxis4Positive,
	GenericInputAxis4Negative,
	GenericInputAxis5Positive,
	GenericInputAxis5Negative,
	GenericInputHatLeft,
	GenericInputHatRight,
	GenericInputHatUp,
	GenericInputHatDown,
};

const char* genericInputNames[] = {
	"Button 0",
	"Button 1",
	"Button 2",
	"Button 3",
	"Button 4",
	"Button 5",
	"Button 6",
	"Button 7",
	"Button 8",
	"Button 9",
	"Button 10",
	"Button 11",
	"Button 12",
	"Button 13",
	"Button 14",
	"Button 15",
	"Button 16",
	"Button 17",
	"Button 18",
	"Button 19",
	"Button 20",
	"Button 21",
	"Button 22",
	"Button 23",
	"Button 24",
	"Button 25",
	"Button 26",
	"Button 27",
	"Button 28",
	"Button 29",
	"Button 30",
	"Button 31",
	"Axis 0+",
	"Axis 0-",
	"Axis 1+",
	"Axis 1-",
	"Axis 2+",
	"Axis 2-",
	"Axis 3+",
	"Axis 3-",
	"Axis 4+",
	"Axis 4-",
	"Axis 5+",
	"Axis 5-",
	"Hat Left",
	"Hat Right",
	"Hat Up,"
	"Hat Down",
};

bool isXboxController(char* deviceName)
{
	return strstr(deviceName, "IG_");
}

void parsePS4Controller(JoystickState* out, BYTE rawData[], DWORD dataSize)
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
		//for (int i=0; i<16; ++i) {
		//	printf("%2X-", rawData[i]);
		//}
		//printf("\n");

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

		out->currentInputs[PS4InputSquare] = (float)squareButton;
		out->currentInputs[PS4InputX] = (float)xButton;
		out->currentInputs[PS4InputCircle] = (float)circleButton;
		out->currentInputs[PS4InputTriangle] = (float)triangleButton;
		out->currentInputs[PS4InputL1] = (float)l1Button;
		out->currentInputs[PS4InputR1] = (float)r1Button;
		out->currentInputs[PS4InputShare] = (float)shareButton;
		out->currentInputs[PS4InputOptions] = (float)optionsButton;
		out->currentInputs[PS4InputL3] = (float)l3Button;
		out->currentInputs[PS4InputR3] = (float)r3Button;
		out->currentInputs[PS4InputPS] = (float)psButton;
		out->currentInputs[PS4InputTouchPadButton]  = (float)touchPadButton;
		out->currentInputs[PS4InputLeftStickLeft]   = -(leftStickX /255.0f * 2 -1);
		out->currentInputs[PS4InputLeftStickRight]  =  (leftStickX /255.0f * 2 -1);
		out->currentInputs[PS4InputLeftStickUp]     = -(leftStickY /255.0f * 2 -1);
		out->currentInputs[PS4InputLeftStickDown]   =  (leftStickY /255.0f * 2 -1);
		out->currentInputs[PS4InputRightStickLeft]  = -(rightStickX/255.0f * 2 -1);
		out->currentInputs[PS4InputRightStickRight] =  (rightStickX/255.0f * 2 -1);
		out->currentInputs[PS4InputRightStickUp]    = -(rightStickY/255.0f * 2 -1);
		out->currentInputs[PS4InputRightStickDown]  =  (rightStickY/255.0f * 2 -1);
		out->currentInputs[PS4InputL2]  = leftTrigger /255.0f;
		out->currentInputs[PS4InputR2] = rightTrigger/255.0f;
		out->currentInputs[PS4InputDpadUp]    = (dpad==0 || dpad==1 || dpad==7)? 1.0f : 0.0f;
		out->currentInputs[PS4InputDpadRight] = (dpad==1 || dpad==2 || dpad==3)? 1.0f : 0.0f; 
		out->currentInputs[PS4InputDpadDown]  = (dpad==3 || dpad==4 || dpad==5)? 1.0f : 0.0f; 
		out->currentInputs[PS4InputDpadLeft]  = (dpad==5 || dpad==6 || dpad==7)? 1.0f : 0.0f;
		out->type = JoystickTypePS4;
	}
}

void parseGenericController(JoystickState* out, BYTE rawData[], DWORD dataSize, _HIDP_PREPARSED_DATA* preparsedData)
{
	memset(out->currentInputs, 0, sizeof(out->currentInputs));

	HIDP_CAPS caps;
	HidP_GetCaps(preparsedData, &caps);

	HIDP_VALUE_CAPS* valueCaps = (HIDP_VALUE_CAPS*)malloc(caps.NumberInputValueCaps * sizeof(HIDP_VALUE_CAPS));
	HidP_GetValueCaps(HidP_Input, valueCaps, &caps.NumberInputValueCaps, preparsedData);
	for (unsigned int i = 0; i < caps.NumberInputValueCaps; ++i)
	{
		ULONG value;
		NTSTATUS status = HidP_GetUsageValue(HidP_Input, valueCaps[i].UsagePage, 0, valueCaps[i].Range.UsageMin, &value, preparsedData, (PCHAR)rawData, dataSize);
		float maxValue = (float)(1<<(valueCaps[i].BitSize))-1;
		float normalizedValue = (value / maxValue)*2-1;
		switch (valueCaps[i].Range.UsageMin) {
			case 0x30:
				out->currentInputs[GenericInputAxis0Positive] = normalizedValue;
				out->currentInputs[GenericInputAxis0Negative] = -normalizedValue;
				break;
			case 0x31:
				out->currentInputs[GenericInputAxis1Positive] = normalizedValue;
				out->currentInputs[GenericInputAxis1Negative] = -normalizedValue;
				break;
			case 0x32:
				out->currentInputs[GenericInputAxis2Positive] = normalizedValue;
				out->currentInputs[GenericInputAxis2Negative] = -normalizedValue;
				break;
			case 0x33:
				out->currentInputs[GenericInputAxis3Positive] = normalizedValue;
				out->currentInputs[GenericInputAxis3Negative] = -normalizedValue;
				break;
			case 0x34:
				out->currentInputs[GenericInputAxis4Positive] = normalizedValue;
				out->currentInputs[GenericInputAxis4Negative] = -normalizedValue;
				break;
			case 0x35:
				out->currentInputs[GenericInputAxis5Positive] = normalizedValue;
				out->currentInputs[GenericInputAxis5Negative] = -normalizedValue;
				break;
			case 0x39:
				LONG hat = value - valueCaps[i].LogicalMin;
				out->currentInputs[GenericInputHatUp]    = (hat==0 || hat==1 || hat==7)? 1.0f : 0.1f; 
				out->currentInputs[GenericInputHatRight] = (hat==1 || hat==2 || hat==3)? 1.0f : 0.1f;  
				out->currentInputs[GenericInputHatDown]  = (hat==3 || hat==4 || hat==5)? 1.0f : 0.1f;  
				out->currentInputs[GenericInputHatLeft]  = (hat==5 || hat==6 || hat==7)? 1.0f : 0.1f;  
		}
	}
	free(valueCaps);

	HIDP_BUTTON_CAPS* buttonCaps = (HIDP_BUTTON_CAPS*)malloc(caps.NumberInputButtonCaps * sizeof(HIDP_BUTTON_CAPS));
	HidP_GetButtonCaps(HidP_Input, buttonCaps, &caps.NumberInputButtonCaps, preparsedData);
	for (unsigned int i = 0; i < caps.NumberInputButtonCaps; ++i)
	{
		unsigned int buttonCount = buttonCaps->Range.UsageMax - buttonCaps->Range.UsageMin + 1;
		USAGE* usages = (USAGE*)malloc(sizeof(USAGE) * buttonCount);
		HidP_GetUsages(HidP_Input, buttonCaps[i].UsagePage, 0, usages, (PULONG)&buttonCount, preparsedData, (PCHAR)rawData, dataSize);
		for (unsigned int buttonIndex=0; buttonIndex < buttonCount; ++buttonIndex) {
			if (usages[buttonIndex] < 32) out->currentInputs[GenericInputButton0+usages[buttonIndex]] = 1.0f;
		}
		free(usages);
	}
	out->type = JoystickTypeGeneric;
}

void printWindowsErrors()
{
	LPTSTR errorText = NULL;

	FormatMessage(
		// use system message tables to retrieve error text
		FORMAT_MESSAGE_FROM_SYSTEM
		// allocate buffer on local heap for error text
		|FORMAT_MESSAGE_ALLOCATE_BUFFER
		// Important! will fail otherwise, since we're not 
		// (and CANNOT) pass insertion parameters
		|FORMAT_MESSAGE_IGNORE_INSERTS,  
		NULL,    // unused with FORMAT_MESSAGE_FROM_SYSTEM
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&errorText,  // output 
		0, // minimum size for output buffer
		NULL);   // arguments - see note 

	if ( NULL != errorText )
	{
		wprintf(L"error:%s ", errorText);

		// release memory allocated by FormatMessage()
		LocalFree(errorText);
		errorText = NULL;
	}
}

void updateRawInput(Joysticks* joysticks, LPARAM lParam)
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
			unsigned int joystickIndex = 4;
			while (joystickIndex<joysticks->count && wcscmp(deviceName, joysticks->states[joystickIndex].deviceName) != 0) {
				++joystickIndex;
			}
			if (joystickIndex == joysticks->count) {
				joysticks->count += 1;
				joysticks->states = (JoystickState*)realloc(joysticks->states, joysticks->count*sizeof(JoystickState));
				JoystickState newState = {0};
				wcscpy_s(newState.deviceName, deviceName);
				joysticks->states[joystickIndex] = newState;
			}

			JoystickState* state = &joysticks->states[joystickIndex];
			if (deviceInfo.hid.dwProductId == 2508) {
				parsePS4Controller(state, input->data.hid.bRawData, input->data.hid.dwSizeHid);
				unsigned char output[547] = {0};
				output[0] = 0x05;
				output[1] = 0xFF;
				output[4] = (unsigned char)(state->lightRumble*255);
				output[5] = (unsigned char)(state->heavyRumble*255);
				//HANDLE hidDevice = CreateFileW(deviceName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
				//HidD_SetOutputReport(hidDevice, output, sizeof(output));
				//printWindowsErrors();
				//WriteFile(hidDevice, output, sizeof(output), 0, 0);
			}
			else {
				parseGenericController(state, input->data.hid.bRawData, input->data.hid.dwSizeHid, data);
			}
			free(data);
		}
	}
	free(input);
}

void enumerateDevices(Joysticks* joysticks)
{
	unsigned int deviceCount = 0;
	GetRawInputDeviceList(0, &deviceCount, sizeof(RAWINPUTDEVICELIST));
	RAWINPUTDEVICELIST* deviceLists = (RAWINPUTDEVICELIST*)malloc(sizeof(RAWINPUTDEVICELIST) * deviceCount);
	GetRawInputDeviceList(deviceLists, &deviceCount, sizeof(RAWINPUTDEVICELIST));
	for (unsigned int i = 0; i < deviceCount; ++i) {
		RID_DEVICE_INFO deviceInfo;
		UINT deviceInfoSize = sizeof(deviceInfo);
		if (GetRawInputDeviceInfo(deviceLists[i].hDevice, RIDI_DEVICEINFO, &deviceInfo, &deviceInfoSize) > 0)
		{
			if (deviceInfo.hid.usUsagePage == 1 && (deviceInfo.hid.usUsage==4 || deviceInfo.hid.usUsage==5))
			{
				WCHAR deviceName[1024] = {0};
				UINT deviceNameLength = sizeof(deviceName)/sizeof(*deviceName);
				if (GetRawInputDeviceInfoW(deviceLists[i].hDevice, RIDI_DEVICENAME, deviceName, &deviceNameLength) > 0)
				{
					HANDLE device = CreateFileW(deviceName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

					WCHAR productString[100] = {0};
					UINT productStringLength = sizeof(productString)/sizeof(*productString);
					HidD_GetProductString(device, productString, productStringLength);

					WCHAR manufacturerString[100] = {0};
					UINT manufacturerStringLength = sizeof(manufacturerString)/sizeof(*manufacturerString);
					HidD_GetManufacturerString(device, manufacturerString, manufacturerStringLength);

					WCHAR serialNumberString[100] = {0};
					UINT serialNumberStringLength = sizeof(serialNumberString)/sizeof(*serialNumberString);
					HidD_GetSerialNumberString(device, serialNumberString, serialNumberStringLength);

					wprintf(L"%s | product:%s | manufacturer:%s | serialNumber:%s\n", deviceName, productString, manufacturerString, serialNumberString);
				}
			}
		}
	}
}

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	Joysticks* joysticks = (Joysticks*)GetPropA(hwnd, "userData");
	if (msg == WM_INPUT) {
		updateRawInput(joysticks, lParam);
	}
	if (msg == WM_INPUT_DEVICE_CHANGE) {
		//for (unsigned int playerIndex=0; playerIndex<4; ++playerIndex) {
		//	XINPUT_STATE state;
		//	joysticks->states[playerIndex].connected = (XInputGetState(playerIndex, &state) == ERROR_SUCCESS);
		//}
		//UINT size = 0;
		//GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER));
		//RAWINPUT* input = (RAWINPUT*)malloc(size);
		//UINT structsWritten = GetRawInputData((HRAWINPUT)lParam, RID_INPUT, input, &size, sizeof(RAWINPUTHEADER));
		//if (structsWritten != -1) {
		//	char deviceName[1024] = {0};
		//	UINT deviceNameLength = sizeof(deviceName)/sizeof(*deviceName);
		//	if (GetRawInputDeviceInfoA(input->header.hDevice, RIDI_DEVICENAME, deviceName, &deviceNameLength) > 0)
		//	{
		//		for (unsigned int i=4; i<joysticks->count; ++i)
		//		{
		//			if (strcmp(joysticks->states[i].deviceName, deviceName) == 0) {
		//				joysticks->states[i].connected = false;
		//			}
		//		}
		//	}
		//}
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

Joysticks createJoysticks()
{
	Joysticks joysticks = {0};

	// Create a window, as we need a window precedure to recieve raw input
	WNDCLASSA wnd = { 0 };
	wnd.hInstance = GetModuleHandle(0);
	wnd.lpfnWndProc = WindowProcedure;
	wnd.lpszClassName = "RawInputEventWindow";
	RegisterClassA(&wnd);
	joysticks.hwnd = CreateWindowA(wnd.lpszClassName, 0, 0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, wnd.hInstance, 0);

	// Register devices
	RAWINPUTDEVICE rid;
	rid.usUsagePage = 0x01;
	rid.usUsage = 0;
	rid.dwFlags = RIDEV_INPUTSINK | RIDEV_PAGEONLY | RIDEV_DEVNOTIFY;
	rid.hwndTarget = joysticks.hwnd;
	RegisterRawInputDevices(&rid, 1, sizeof(RAWINPUTDEVICE));

	// Allocate xbox controllers
	joysticks.count = 4;
	joysticks.states = (JoystickState*)calloc(joysticks.count, sizeof(JoystickState));
	for (unsigned int playerIndex=0; playerIndex<4; ++playerIndex) {
		joysticks.states[playerIndex].connected = true;
		joysticks.states[playerIndex].type = JoystickTypeXbox;
	}

	return joysticks;
}

void updateJoysticks(Joysticks* joysticks)
{
	for (unsigned int i=0; i<joysticks->count; ++i) {
		memcpy(joysticks->states[i].previousInputs, joysticks->states[i].currentInputs, sizeof(joysticks->states[i].previousInputs));
	}

	// HID controllers
	SetPropA(joysticks->hwnd, "userData", joysticks);
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// Xbox controllers
	for (unsigned int playerIndex=0; playerIndex<4; ++playerIndex)
	{
		JoystickState* state = &joysticks->states[playerIndex];
		XINPUT_STATE xinput;
		if (state->connected && XInputGetState(playerIndex, &xinput) == ERROR_SUCCESS) {
			state->currentInputs[XboxInputA]                   = (xinput.Gamepad.wButtons & XINPUT_GAMEPAD_A)? 1.0f : 0.0f;
			state->currentInputs[XboxInputB]                   = (xinput.Gamepad.wButtons & XINPUT_GAMEPAD_B)? 1.0f : 0.0f;
			state->currentInputs[XboxInputX]                   = (xinput.Gamepad.wButtons & XINPUT_GAMEPAD_X)? 1.0f : 0.0f;
			state->currentInputs[XboxInputY]                   = (xinput.Gamepad.wButtons & XINPUT_GAMEPAD_Y)? 1.0f : 0.0f;
			state->currentInputs[XboxInputDpadLeft]            = (xinput.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT)? 1.0f : 0.0f;
			state->currentInputs[XboxInputDpadRight]           = (xinput.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)? 1.0f : 0.0f;
			state->currentInputs[XboxInputDpadUp]              = (xinput.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP)? 1.0f : 0.0f;
			state->currentInputs[XboxInputDpadDown]            = (xinput.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN)? 1.0f : 0.0f;
			state->currentInputs[XboxInputLeftBumper]          = (xinput.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)? 1.0f : 0.0f;
			state->currentInputs[XboxInputRightBumper]         = (xinput.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)? 1.0f : 0.0f;
			state->currentInputs[XboxInputLeftStickButton]     = (xinput.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB)? 1.0f : 0.0f;
			state->currentInputs[XboxInputRightStickButton]    = (xinput.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB)? 1.0f : 0.0f;
			state->currentInputs[XboxInputBack]                = (xinput.Gamepad.wButtons & XINPUT_GAMEPAD_BACK)? 1.0f : 0.0f;
			state->currentInputs[XboxInputStart]               = (xinput.Gamepad.wButtons & XINPUT_GAMEPAD_START)? 1.0f : 0.0f;
			state->currentInputs[XboxInputLeftTrigger]         = xinput.Gamepad.bLeftTrigger  / 255.0f;
			state->currentInputs[XboxInputRightTrigger]        = xinput.Gamepad.bRightTrigger / 255.0f;
			state->currentInputs[XboxInputLeftStickLeft]       =-xinput.Gamepad.sThumbLX / 32767.0f;
			state->currentInputs[XboxInputLeftStickRight]      = xinput.Gamepad.sThumbLX / 32767.0f;
			state->currentInputs[XboxInputLeftStickUp]         = xinput.Gamepad.sThumbLY / 32767.0f;
			state->currentInputs[XboxInputLeftStickDown]       =-xinput.Gamepad.sThumbLY / 32767.0f;
			state->currentInputs[XboxInputRightStickLeft]      =-xinput.Gamepad.sThumbRX / 32767.0f;
			state->currentInputs[XboxInputRightStickRight]     = xinput.Gamepad.sThumbRX / 32767.0f;
			state->currentInputs[XboxInputRightStickUp]        = xinput.Gamepad.sThumbRY / 32767.0f;
			state->currentInputs[XboxInputRightStickDown]      =-xinput.Gamepad.sThumbRY / 32767.0f;
		}
		else {
			state->connected = false;
		}
	}

	// Clamp negative inputs to 0
	for (unsigned int joystickIndex=0; joystickIndex<joysticks->count; ++joystickIndex) {
		for (unsigned int inputIndex=0; inputIndex<JoystickState::inputCount; ++inputIndex) {
			float* input = &joysticks->states[joystickIndex].currentInputs[inputIndex];
			if (*input < 0) *input = 0;
		}
	}
}

void setJoystickForceFeedback(Joysticks* joysticks, unsigned int joystickIndex, float leftForceFeedback, float rightForceFeedback)
{
	if (joystickIndex <= 3) {
		XINPUT_VIBRATION vibration;
		vibration.wLeftMotorSpeed  = (WORD)(leftForceFeedback*0xFFFF);
		vibration.wRightMotorSpeed = (WORD)(rightForceFeedback*0xFFFF);
		XInputSetState(joystickIndex, &vibration);
	}
	if (joysticks->states[joystickIndex].type == JoystickTypePS4) {
		HANDLE hidDevice = CreateFileW(joysticks->states[joystickIndex].deviceName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
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
			unsigned char output[78] = {0};
			output[0] = 0x11;
			output[1] = 0x80;
			output[2] = 0x0f;
			output[6] = (unsigned char)(joysticks->states[joystickIndex].lightRumble*255);
			output[7] = (unsigned char)(joysticks->states[joystickIndex].heavyRumble*255);
			output[8] = (unsigned char)(joysticks->states[joystickIndex].ledRed*255);
			output[9] = (unsigned char)(joysticks->states[joystickIndex].ledGreen*255);
			output[10] = (unsigned char)(joysticks->states[joystickIndex].ledBlue*255);
			OVERLAPPED ovelappedInfo = {0};
			WriteFile(hidDevice, output, sizeof(output), 0, &ovelappedInfo);
			//HidD_SetOutputReport(hidDevice, output, sizeof(output));
			//printWindowsErrors();
			CloseHandle(hidDevice);
		}
	}
}

const char* getInputName(Joysticks joysticks, unsigned int joystickIndex, unsigned int inputIndex)
{
	switch (joysticks.states[joystickIndex].type)
	{
		case JoystickTypeGeneric: return genericInputNames[inputIndex];
		case JoystickTypePS4: return ps4InputNames[inputIndex];
		case JoystickTypeXbox: return xboxInputNames[inputIndex];
		default: return 0;
	}
}

int main()
{
	Joysticks joysticks = createJoysticks();
	unsigned int frameNumber = 0;
	while (1)
	{
		//printf("frame%d ", frameNumber);
		updateJoysticks(&joysticks);
		for (unsigned int joystickIndex=0; joystickIndex<joysticks.count; ++joystickIndex)
		{
			JoystickState* state = &joysticks.states[joystickIndex];
			for (unsigned int inputIndex=0; inputIndex<state->inputCount; ++inputIndex)
			{
				if (state->currentInputs[inputIndex] > 0.5 && state->previousInputs[inputIndex] <= 0.5f)
				{
					const char* inputName = getInputName(joysticks, joystickIndex, inputIndex);
					printf("%s ", inputName);
				}

				float rumbleLeft = 0;
				float rumbleRight = 0;
				if (state->type == JoystickTypeXbox) {
					state->lightRumble = state->currentInputs[XboxInputLeftTrigger];
					state->heavyRumble = state->currentInputs[XboxInputRightTrigger];
				}
				else if (state->type == JoystickTypePS4) {
					state->heavyRumble = state->currentInputs[PS4InputL2];
					state->lightRumble = state->currentInputs[PS4InputR2];
					state->ledGreen = state->currentInputs[PS4InputTriangle];
				}
				setJoystickForceFeedback(&joysticks, joystickIndex, rumbleLeft, rumbleRight);
			}
		}
		++frameNumber;
		Sleep(16);
	}
}

