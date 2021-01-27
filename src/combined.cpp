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

struct OutputThreadData
{
	enum Type {
		type_writeFile, type_setOutputReport, type_stop
	};

	volatile DWORD byteCount;
	volatile BYTE buffer[128];
	volatile Type type;
	volatile HANDLE file;
	HANDLE thread;
	CONDITION_VARIABLE conditionVariable;
	CRITICAL_SECTION criticalSection;
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
	char manufacturerName[maxNameLength];

	// Outputs
	float lightRumble;
	float heavyRumble;
	float ledRed;
	float ledGreen;
	float ledBlue;

	OutputThreadData* outputThreadData;
};

struct Joysticks
{
	static const unsigned int maxXinputControllers = 4;
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
	"XBox-A",
	"XBox-B",
	"XBox-X",
	"XBox-Y",
	"XBox-Left",
	"XBox-Right",
	"XBox-Up",
	"XBox-Down",
	"XBox-LB",
	"XBox-RB",
	"XBox-LS",
	"XBox-RS",
	"XBox-Back",
	"XBox-Start",
	"XBox-Left Trigger",
	"XBox-Right Trigger",
	"XBox-Left Stick Left",
	"XBox-Left Stick Right",
	"XBox-Left Stick Up",
	"XBox-Left Stick Down",
	"XBox-Right Stick Left",
	"XBox-Right Stick Right",
	"XBox-Right Stick Up",
	"XBox-Right Stick Down",
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
	"DS4-Square",
	"DS4-X",
	"DS4-Circle",
	"DS4-Triangle",
	"DS4-L1",
	"DS4-R1",
	"DS4-L3",
	"DS4-R3",
	"DS4-Options",
	"DS4-Share",
	"DS4-PlayStation Button",
	"DS4-Touch Pad Button",
	"DS4-Left Stick Left",
	"DS4-Left Stick Right",
	"DS4-Left Stick Up",
	"DS4-Left Stick Down",
	"DS4-Right Stick Left",
	"DS4-Right Stick Right",
	"DS4-Right Stick Up",
	"DS4-Right Stick Down",
	"DS4-L2",
	"DS4-R2",
	"DS4-Dpad Left",
	"DS4-Dpad Right",
	"DS4-Dpad Up",
	"DS4-Dpad Down",
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
	GenericInputAxis6Positive,
	GenericInputAxis6Negative,
	GenericInputAxis7Positive,
	GenericInputAxis7Negative,
	GenericInputHatLeft,
	GenericInputHatRight,
	GenericInputHatUp,
	GenericInputHatDown,
};

const char* genericInputNames[] = {
	"Controller-Button 0",
	"Controller-Button 1",
	"Controller-Button 2",
	"Controller-Button 3",
	"Controller-Button 4",
	"Controller-Button 5",
	"Controller-Button 6",
	"Controller-Button 7",
	"Controller-Button 8",
	"Controller-Button 9",
	"Controller-Button 10",
	"Controller-Button 11",
	"Controller-Button 12",
	"Controller-Button 13",
	"Controller-Button 14",
	"Controller-Button 15",
	"Controller-Button 16",
	"Controller-Button 17",
	"Controller-Button 18",
	"Controller-Button 19",
	"Controller-Button 20",
	"Controller-Button 21",
	"Controller-Button 22",
	"Controller-Button 23",
	"Controller-Button 24",
	"Controller-Button 25",
	"Controller-Button 26",
	"Controller-Button 27",
	"Controller-Button 28",
	"Controller-Button 29",
	"Controller-Button 30",
	"Controller-Button 31",
	"Controller-Axis 0+",
	"Controller-Axis 0-",
	"Controller-Axis 1+",
	"Controller-Axis 1-",
	"Controller-Axis 2+",
	"Controller-Axis 2-",
	"Controller-Axis 3+",
	"Controller-Axis 3-",
	"Controller-Axis 4+",
	"Controller-Axis 4-",
	"Controller-Axis 5+",
	"Controller-Axis 5-",
	"Controller-Axis 6+",
	"Controller-Axis 6-",
	"Controller-Axis 7+",
	"Controller-Axis 7-",
	"Controller-Hat Left",
	"Controller-Hat Right",
	"Controller-Hat Up,"
	"Controller-Hat Down",
};

int outputThread(void* param)
{
	OutputThreadData* data = (OutputThreadData*)param;
	EnterCriticalSection(&data->criticalSection);
	while (data->type != OutputThreadData::type_stop)
	{
		SleepConditionVariableCS(&data->conditionVariable, &data->criticalSection, INFINITE);
		if (data->type != OutputThreadData::type_stop)
		{
			if (data->type == OutputThreadData::type_writeFile) {
				DWORD bytesWritten;
				WriteFile(data->file, (void*)data->buffer, data->byteCount, &bytesWritten, 0);
			}
			if (data->type == OutputThreadData::type_setOutputReport) {
				HidD_SetOutputReport(data->file, (void*)data->buffer, data->byteCount);
			}
		}
	}
	LeaveCriticalSection(&data->criticalSection);
	return 0;
}

bool isXboxController(char* deviceName)
{
	return strstr(deviceName, "IG_");
}

void updatePS4Controller(JoystickState* out, BYTE rawData[], DWORD byteCount)
{
	const DWORD usbInputByteCount = 64;
	const DWORD bluetoothInputByteCount = 547;

	unsigned int offset = 0;
	if (byteCount == bluetoothInputByteCount) {
		offset = 2;
	}
	else if (byteCount != usbInputByteCount) {
		return;
	}

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

	offset = 0;
	if (byteCount == usbInputByteCount)
	{
		out->outputThreadData->type = OutputThreadData::type_writeFile;
		out->outputThreadData->byteCount = 32;
		out->outputThreadData->buffer[0] = 0x05;
		out->outputThreadData->buffer[1] = 0xFF;
	}
	if (byteCount == bluetoothInputByteCount)
	{
		out->outputThreadData->type = OutputThreadData::type_setOutputReport;
		out->outputThreadData->byteCount = 78;
		out->outputThreadData->buffer[0] = 0x11;
		out->outputThreadData->buffer[1] = 0XC0;
		out->outputThreadData->buffer[3] = 0x07;
		offset = 2;
	}
	out->outputThreadData->buffer[4 + offset] = (BYTE)(out->lightRumble*0xFF);
	out->outputThreadData->buffer[5 + offset] = (BYTE)(out->heavyRumble*0xFF);
	out->outputThreadData->buffer[6 + offset] = (BYTE)(out->ledRed     *0xFF);
	out->outputThreadData->buffer[7 + offset] = (BYTE)(out->ledGreen   *0xFF);
	out->outputThreadData->buffer[8 + offset] = (BYTE)(out->ledBlue    *0xFF);
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
		unsigned int usage = valueCaps[i].Range.UsageMin;
		if (usage >= 0x30 && usage <= 0x37) {
			int axisIndex = usage-0x30;
			out->currentInputs[GenericInputAxis0Positive+2*axisIndex] = normalizedValue;
			out->currentInputs[GenericInputAxis0Negative+2*axisIndex] = -normalizedValue;
		}
		if (usage == 0x39) {
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
			for (UINT i=Joysticks::maxXinputControllers; i<joysticks->count; ++i)
			{
				if (wcscmp(deviceName, joysticks->states[i].deviceName) == 0)
				{
					JoystickState* state = &joysticks->states[i];
					if (deviceInfo.hid.dwProductId == 2508) {
						updatePS4Controller(state, input->data.hid.bRawData, input->data.hid.dwSizeHid);
					}
					else {
						parseGenericController(state, input->data.hid.bRawData, input->data.hid.dwSizeHid, data);
					}
					free(data);
				}
			}
		}
	}
	free(input);
}

void connectHIDJoystick(Joysticks* joysticks, const WCHAR* deviceName)
{
	unsigned int joystickIndex = Joysticks::maxXinputControllers;
	while (joystickIndex < joysticks->count && wcscmp(deviceName, joysticks->states[joystickIndex].deviceName) != 0) {
		++joystickIndex;
	}
	if (joystickIndex == joysticks->count) {
		joysticks->count += 1;
		joysticks->states = (JoystickState*)realloc(joysticks->states, joysticks->count*sizeof(JoystickState));
		JoystickState newState = {0};
		wcscpy_s(newState.deviceName, deviceName);

		newState.outputThreadData = (OutputThreadData*)calloc(1, sizeof(OutputThreadData));
		InitializeConditionVariable(&newState.outputThreadData->conditionVariable);
		InitializeCriticalSection(&newState.outputThreadData->criticalSection);

		HidD_GetProductString(newState.outputThreadData->file, newState.productName, JoystickState::maxNameLength);
		HidD_GetManufacturerString(newState.outputThreadData->file, newState.manufacturerName, JoystickState::maxNameLength);

		joysticks->states[joystickIndex] = newState;
	}

	joysticks->states[joystickIndex].outputThreadData->type = OutputThreadData::type_writeFile;
	joysticks->states[joystickIndex].outputThreadData->file = CreateFileW(deviceName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	joysticks->states[joystickIndex].outputThreadData->thread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)outputThread, (LPVOID)joysticks->states[joystickIndex].outputThreadData, 0, 0);
	joysticks->states[joystickIndex].connected = true;
}

void enumerateDevices(Joysticks* joysticks)
{
	// XInput
	for (unsigned int playerIndex=0; playerIndex<Joysticks::maxXinputControllers; ++playerIndex) {
		XINPUT_STATE state;
		joysticks->states[playerIndex].connected = (XInputGetState(playerIndex, &state) == ERROR_SUCCESS);
	}

	// Set all HID joysticks to disconnected
	for (unsigned int i=Joysticks::maxXinputControllers; i<joysticks->count; ++i) {
		if (joysticks->states[i].connected) {
			EnterCriticalSection(&joysticks->states[i].outputThreadData->criticalSection);
			joysticks->states[i].outputThreadData->type = OutputThreadData::type_stop;
			joysticks->states[i].connected = false;
			CloseHandle(joysticks->states[i].outputThreadData->file);
			CloseHandle(joysticks->states[i].outputThreadData->thread);
			LeaveCriticalSection(&joysticks->states[i].outputThreadData->criticalSection);
			WakeAllConditionVariable(&joysticks->states[i].outputThreadData->conditionVariable);
		}
	}

	// Find all connected joysticks
	unsigned int deviceCount = 0;
	GetRawInputDeviceList(0, &deviceCount, sizeof(RAWINPUTDEVICELIST));
	RAWINPUTDEVICELIST* deviceLists = (RAWINPUTDEVICELIST*)malloc(sizeof(RAWINPUTDEVICELIST) * deviceCount);
	GetRawInputDeviceList(deviceLists, &deviceCount, sizeof(RAWINPUTDEVICELIST));

	for (unsigned int i = 0; i < deviceCount; ++i)
	{
		RID_DEVICE_INFO deviceInfo;
		UINT deviceInfoSize = sizeof(deviceInfo);
		bool gotInfo = GetRawInputDeviceInfo(deviceLists[i].hDevice, RIDI_DEVICEINFO, &deviceInfo, &deviceInfoSize) > 0;

		WCHAR deviceName[1024] = {0};
		UINT deviceNameLength = sizeof(deviceName)/sizeof(*deviceName);
		bool gotName = GetRawInputDeviceInfoW(deviceLists[i].hDevice, RIDI_DEVICENAME, deviceName, &deviceNameLength) > 0;

		if (gotInfo && gotName && deviceInfo.hid.usUsagePage==0x01 && (deviceInfo.hid.usUsage==0x04 || deviceInfo.hid.usUsage==0x05))
		{
			connectHIDJoystick(joysticks, deviceName);
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
		enumerateDevices(joysticks);
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
	joysticks.count = Joysticks::maxXinputControllers;
	joysticks.states = (JoystickState*)calloc(joysticks.count, sizeof(JoystickState));
	for (UINT i=0; i<Joysticks::maxXinputControllers; ++i) joysticks.states[i].type = JoystickTypeXbox;

	enumerateDevices(&joysticks);
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

	// Signal output thread
	for (UINT i=Joysticks::maxXinputControllers; i<joysticks->count; ++i) {
		WakeConditionVariable(&joysticks->states[i].outputThreadData->conditionVariable);
	}

	// Xbox controllers
	for (unsigned int playerIndex=0; playerIndex<Joysticks::maxXinputControllers; ++playerIndex)
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

			XINPUT_VIBRATION vibration;
			vibration.wLeftMotorSpeed  = (WORD)(state->lightRumble*0xFFFF);
			vibration.wRightMotorSpeed = (WORD)(state->heavyRumble*0xFFFF);
			XInputSetState(playerIndex, &vibration);
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
	while (1)
	{
		updateJoysticks(&joysticks);
		for (unsigned int joystickIndex=0; joystickIndex<joysticks.count; ++joystickIndex)
		{
			JoystickState* state = &joysticks.states[joystickIndex];
			for (unsigned int inputIndex=0; inputIndex<state->inputCount; ++inputIndex)
			{
				if (state->currentInputs[inputIndex] > 0.5 && state->previousInputs[inputIndex] <= 0.5f)
				{
					const char* inputName = getInputName(joysticks, joystickIndex, inputIndex);
					printf("%s\n", inputName);
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
					state->ledRed = (state->currentInputs[PS4InputCircle] || state->currentInputs[PS4InputSquare])? 1.0f : 0.0f;
					state->ledGreen = state->currentInputs[PS4InputTriangle];
					state->ledBlue = (state->currentInputs[PS4InputX] || state->currentInputs[PS4InputSquare])? 1.0f : 0.0f;
				}
			}
		}
		Sleep(16);
	}
}

