#include "GameInput.h"
#include <stdio.h>

struct Joysticks
{
	uint32_t deviceCount;
	IGameInputDevice** devices;
};

void CALLBACK deviceChangeCallback(GameInputCallbackToken callbackToken, void* context, IGameInputDevice* device, uint64_t timestamp, GameInputDeviceStatus currentStatus, GameInputDeviceStatus previousStatus) 
{ 
	Joysticks* joysticks = (Joysticks*)context;
	if (currentStatus & GameInputDeviceConnected)
	{
		for (uint32_t i = 0; i < joysticks->deviceCount; ++i)
		{
			if (joysticks->devices[i] == device) return;
		}
		++joysticks->deviceCount;
		joysticks->devices = (IGameInputDevice**)realloc(joysticks->devices, joysticks->deviceCount * sizeof(IGameInputDevice*));
		joysticks->devices[joysticks->deviceCount-1] = device;
	}
} 

int main()
{
	Joysticks joysticks = {0};
	IGameInput* input;
	GameInputCreate(&input);
	IGameInputDispatcher* dispatcher;
	input->CreateDispatcher(&dispatcher);
	GameInputCallbackToken callbackId;
	input->RegisterDeviceCallback(0, GameInputKindController, GameInputDeviceAnyStatus, GameInputBlockingEnumeration, &joysticks, deviceChangeCallback, &callbackId);

	bool buttons[64];
	GameInputSwitchPosition switches[64];
	float axes[64];

	while (1)
	{
		dispatcher->Dispatch(0);
		for (uint32_t i = 0; i < joysticks.deviceCount; ++i)
		{
			IGameInputReading* reading;
			if (SUCCEEDED(input->GetCurrentReading(GameInputKindController, joysticks.devices[i], &reading)))
			{
				IGameInputDevice* device;
				reading->GetDevice(&device);
				const GameInputDeviceInfo* deviceInfo = device->GetDeviceInfo();

				printf("Joystick %d, ", i);
				reading->GetControllerAxisState(ARRAYSIZE(axes), axes);
				reading->GetControllerSwitchState(ARRAYSIZE(switches), switches);
				reading->GetControllerButtonState(ARRAYSIZE(buttons), buttons);
				printf("Axes - ");
				for (uint32_t i = 0; i < reading->GetControllerAxisCount(); ++i) {
					printf("%d:%f ", i, axes[i]);
				}
				printf("Switches - ");
				for (uint32_t i = 0; i < reading->GetControllerSwitchCount(); ++i) {
					printf("%d:%d ", i, switches[i]);
				}
				printf("Buttons - ");
				for (uint32_t i = 0; i < reading->GetControllerButtonCount(); ++i) {
					if (buttons[i]) printf("%d ", i);
				}
				puts("");
				fflush(stdout);
				device->Release();
				reading->Release();
			}
		}
		Sleep(16);
	}
}
