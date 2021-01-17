#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <stdio.h>

struct Joysticks
{
	unsigned int deviceCount;
	LPDIRECTINPUTDEVICE8* devices;
	LPDIRECTINPUT8 dinput;
};

BOOL CALLBACK DirectInputEnumDevicesCallback(LPCDIDEVICEINSTANCE instance, LPVOID userData)
{
	Joysticks* joysticks = (Joysticks*)userData;
	
	LPDIRECTINPUTDEVICE8 device;
	joysticks->dinput->CreateDevice(instance->guidInstance, &device, NULL);
	device->SetCooperativeLevel(GetActiveWindow(), DISCL_NONEXCLUSIVE);
	device->SetDataFormat(&c_dfDIJoystick);
	device->Acquire();

	joysticks->deviceCount += 1;
	joysticks->devices = (LPDIRECTINPUTDEVICE8*)realloc(joysticks->devices, joysticks->deviceCount * sizeof(LPDIRECTINPUTDEVICE8));
	joysticks->devices[joysticks->deviceCount - 1] = device;

	return DIENUM_CONTINUE;
}

BOOL CALLBACK EnumerateEffectsCallback(LPCDIEFFECTINFO effect, LPVOID userData)
{
	return DIENUM_STOP;
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

	LPDIRECTINPUTDEVICE8 device = joysticks.devices[0];
	device->EnumEffects(EnumerateEffectsCallback, 0, DIEFT_ALL);
	DIEFFECT effect = {sizeof(DIEFFECT)};
	effect.dwFlags = 0;
	effect.dwDuration = INFINITE;
	effect.dwGain = DI_FFNOMINALMAX;
	effect.dwTriggerButton = DIEB_NOTRIGGER;
	effect.cAxes = 2;
	DWORD axes[2] = {DIJOFS_X, DIJOFS_Y};
	effect.rgdwAxes = axes;
	DIENVELOPE envelope = {sizeof(DIENVELOPE)};
	envelope.dwAttackTime = (DWORD)(0.5 * DI_SECONDS);
	envelope.dwFadeTime = (DWORD)(0.1 * DI_SECONDS);
	effect.lpEnvelope = &envelope;
	DIPERIODIC periodic = {0};
	periodic.dwMagnitude = DI_FFNOMINALMAX;
	periodic.dwPeriod = (DWORD)(0.05 * DI_SECONDS);
	effect.cbTypeSpecificParams = sizeof(periodic);
	effect.lpvTypeSpecificParams = &periodic;
	LPDIRECTINPUTEFFECT effectInterface;
	device->CreateEffect(GUID_ConstantForce, &effect, &effectInterface, NULL);

	DIJOYSTATE previousState = {0};
	while (1)
	{
		DIJOYSTATE state;
		if (device->GetDeviceState(sizeof(state), &state) == DI_OK && memcmp(&state, &previousState, sizeof(state)) != 0)
		{
			DIDEVCAPS caps = { sizeof(DIDEVCAPS) };
			device->GetCapabilities(&caps);

			printf("Axes: ");
			printf("X:%5d ", state.lX);
			printf("Y:%5d ", state.lY);
			printf("Z:%5d ", state.lZ);
			printf("Rx:%5d ", state.lRx);
			printf("Ry:%5d ", state.lRy);
			printf("Rz:%5d ", state.lRz);

			printf("Hat:%5d", state.rgdwPOV[0]);
			
			printf("Buttons: ");
			for (unsigned int buttonIndex = 0; buttonIndex < caps.dwButtons; ++buttonIndex) {
				if (state.rgbButtons[buttonIndex]) {
					printf("%d ", buttonIndex);
				}
			}
			
			printf("\n");
			previousState = state;
		}
		Sleep(16);
	}
}
