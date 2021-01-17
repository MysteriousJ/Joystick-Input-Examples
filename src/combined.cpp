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
	static const unsigned int inputCount = 32;
	static const unsigned int maxNameLength = 128;

	bool connected;
	JoystickType type;
	float currentInputs[inputCount];
	float previousInputs[inputCount];
	char deviceName[maxNameLength];
	char productName[maxNameLength];
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

bool isXboxController(char* deviceName)
{
	return strstr(deviceName, "IG_");
}

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
		RID_DEVICE_INFO deviceInfo;
		UINT deviceInfoSize = sizeof(deviceInfo);
		GetRawInputDeviceInfo(input->header.hDevice, RIDI_DEVICEINFO, &deviceInfo, &deviceInfoSize);
		GetRawInputDeviceInfo(input->header.hDevice, RIDI_PREPARSEDDATA, 0, &size);
		_HIDP_PREPARSED_DATA* data = (_HIDP_PREPARSED_DATA*)malloc(size);
		UINT bytesWritten = GetRawInputDeviceInfo(input->header.hDevice, RIDI_PREPARSEDDATA, data, &size);
		if (bytesWritten > 0)
		{
			printf("pid:%d, vid:%d, ", deviceInfo.hid.dwProductId, deviceInfo.hid.dwVendorId);
			//memset(joysticks->states[i].currentInputs, 0, sizeof(joysticks->states[i].currentInputs));
			HIDP_CAPS caps;
			HidP_GetCaps(data, &caps);

			printf("Values: ");
			HIDP_VALUE_CAPS* valueCaps = (HIDP_VALUE_CAPS*)malloc(caps.NumberInputValueCaps * sizeof(HIDP_VALUE_CAPS));
			HidP_GetValueCaps(HidP_Input, valueCaps, &caps.NumberInputValueCaps, data);
			for (unsigned int i = 0; i < caps.NumberInputValueCaps; ++i)
			{
				ULONG value;
				NTSTATUS status = HidP_GetUsageValue(HidP_Input, valueCaps[i].UsagePage, 0, valueCaps[i].Range.UsageMin, &value, data, (PCHAR)input->data.hid.bRawData, input->data.hid.dwSizeHid);
				if (status == HIDP_STATUS_SUCCESS)
				{
					float normalizedValue;
					if (valueCaps[i].LogicalMax > 0) normalizedValue = ((float)value/(float)valueCaps[i].LogicalMax)*2-1;
					else normalizedValue = (value/65535.0f)*2-1;
					switch (valueCaps[i].Range.UsageMin) {
						case 0x30: printf("X:% f ", normalizedValue); break;
						case 0x31: printf("Y:% f ", normalizedValue); break;
						case 0x32: printf("Z:% f ", normalizedValue); break;
						case 0x33: printf("RX:% f ", normalizedValue); break;
						case 0x34: printf("RY:% f ", normalizedValue); break;
						case 0x35: printf("RZ:% f ", normalizedValue); break;
						case 0x39:
							printf("hat:%5d ", value);
					}
				}
				
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
		printRawInputData(lParam);
	}
	if (msg == WM_INPUT_DEVICE_CHANGE) {
		for (unsigned int playerIndex=0; playerIndex<4; ++playerIndex) {
			XINPUT_STATE state;
			joysticks->states[playerIndex].connected = (XInputGetState(playerIndex, &state) == ERROR_SUCCESS);
		}
		UINT size = 0;
		GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER));
		RAWINPUT* input = (RAWINPUT*)malloc(size);
		UINT structsWritten = GetRawInputData((HRAWINPUT)lParam, RID_INPUT, input, &size, sizeof(RAWINPUTHEADER));
		if (structsWritten != -1) {
			char deviceName[1024] = {0};
			UINT deviceNameLength = sizeof(deviceName)/sizeof(*deviceName);
			if (GetRawInputDeviceInfoA(input->header.hDevice, RIDI_DEVICENAME, deviceName, &deviceNameLength) > 0)
			{
				for (unsigned int i=4; i<joysticks->count; ++i)
				{
					if (strcmp(joysticks->states[i].deviceName, deviceName) == 0) {
						joysticks->states[i].connected = false;
					}
				}
			}
		}
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

const char* getInputName(Joysticks joysticks, unsigned int joystickIndex, unsigned int inputIndex)
{
	return xboxInputNames[inputIndex];
}

int main()
{
	Joysticks joysticks = createJoysticks();

	while (1)
	{
		updateJoysticks(&joysticks);
		for (unsigned int joystickIndex=0; joystickIndex<joysticks.count; ++joystickIndex)
		{
			JoystickState state  = joysticks.states[joystickIndex];
			for (unsigned int inputIndex=0; inputIndex<state.inputCount; ++inputIndex)
			{
				if (state.currentInputs[inputIndex] > 0.5 && state.previousInputs[inputIndex] <= 0.5f)
				{
					const char* inputName = getInputName(joysticks, joystickIndex, inputIndex);
					printf("%s ", inputName);
				}
			}
		}
		Sleep(16);
	}
}

