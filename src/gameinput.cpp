#include "GameInput.h"
#include <stdio.h>


int main()
{
	IGameInput* input;
	GameInputCreate(&input);
	IGameInputReading* reading;
	bool buttons[64];
	GameInputSwitchPosition switches[64];
	float axes[64];
	while (1)
	{
		while (SUCCEEDED(input->GetCurrentReading(GameInputKindController, 0, &reading)))
		{
			IGameInputDevice* device;
			reading->GetDevice(&device);
			const GameInputDeviceInfo* deviceInfo = device->GetDeviceInfo();
			if (deviceInfo->displayName) {
				printf("Joystick - %s, ", deviceInfo->displayName->data);
			}
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
			device->Release();
			reading->Release();
		}
	}
}
