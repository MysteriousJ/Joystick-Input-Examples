#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <stdio.h>

struct Joysticks
{
	unsigned int deviceCount;
	IDirectInputDevice8** devices;
	IDirectInput8* dinput;
};

BOOL CALLBACK DirectInputEnumDevicesCallback(LPCDIDEVICEINSTANCE instance, LPVOID userData)
{
	Joysticks* joysticks = (Joysticks*)userData;
	
	IDirectInputDevice8* device;
	joysticks->dinput->CreateDevice(instance->guidInstance, &device, NULL);
	device->SetCooperativeLevel(GetActiveWindow(), DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
	device->SetDataFormat(&c_dfDIJoystick);
	device->Acquire();

	joysticks->deviceCount += 1;
	joysticks->devices = (IDirectInputDevice8**)realloc(joysticks->devices, joysticks->deviceCount * sizeof(IDirectInputDevice8*));
	joysticks->devices[joysticks->deviceCount - 1] = device;

	return DIENUM_CONTINUE;
}

int main()
{
	HINSTANCE hInstance = GetModuleHandle(0);
	Joysticks joysticks = {0};
	DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&joysticks.dinput, 0);
	joysticks.dinput->EnumDevices(DI8DEVCLASS_GAMECTRL, DirectInputEnumDevicesCallback, (void*)&joysticks, DIEDFL_ALLDEVICES);

	if (joysticks.deviceCount == 0) {
		printf("No joysticks connected\n");
		return 1;
	}

	while (1)
	{
		for (unsigned int joystickIndex=0; joystickIndex < joysticks.deviceCount; ++joystickIndex)
		{
			DIJOYSTATE state;
			if (joysticks.devices[joystickIndex]->GetDeviceState(sizeof(state), &state) == DI_OK)
			{
				DIDEVCAPS caps = {sizeof(DIDEVCAPS)};
				joysticks.devices[joystickIndex]->GetCapabilities(&caps);

				printf("Joystick %d ", joystickIndex);
				printf("X:%5d ", state.lX);
				printf("Y:%5d ", state.lY);
				printf("Z:%5d ", state.lZ);
				printf("Rx:%5d ", state.lRx);
				printf("Ry:%5d ", state.lRy);
				printf("Rz:%5d ", state.lRz);
				printf("Slider0:%5d ", state.rglSlider[0]);
				printf("Slider1:%5d ", state.rglSlider[1]);

				printf("Hat:%5d ", state.rgdwPOV[0]);

				printf("Buttons: ");
				for (unsigned int buttonIndex = 0; buttonIndex < caps.dwButtons; ++buttonIndex) {
					if (state.rgbButtons[buttonIndex]) {
						printf("%d ", buttonIndex);
					}
				}

				printf("\n");
			}
		}
		Sleep(16);
	}
}
