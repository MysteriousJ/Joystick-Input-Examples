#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <stdio.h>
#include <math.h>

struct Joystick
{
	static const unsigned int inputCount = 64;

	float currentInputs[inputCount];
	float previousInputs[inputCount];
	IDirectInputDevice8* device;
};

struct Input
{
	static const unsigned int keyCount = 256;

	unsigned int joystickCount;
	Joystick* joysticks;
	IDirectInput8* dinput;
	bool currentKeyboard[keyCount];
	bool previousKeyboard[keyCount];
};

enum InputActions { action_moveUp, action_moveDown, action_moveLeft, action_moveRight, action_shoot, action_jump, action_count };
const char* inputActionNames[] = {"move up", "move down", "move left", "move right", "shoot", "jump"};

struct InputBinding
{
	bool joystickBound;
	unsigned int joystickInputIndex;
	bool keyBound;
	unsigned int keyIndex;
};

struct Vec2 { float x, y; };

BOOL CALLBACK DirectInputEnumDevicesCallback(LPCDIDEVICEINSTANCE instance, LPVOID userData)
{
	Input* input = (Input*)userData;

	IDirectInputDevice8* device;
	input->dinput->CreateDevice(instance->guidInstance, &device, NULL);
	device->SetCooperativeLevel(GetActiveWindow(), DISCL_NONEXCLUSIVE);
	device->SetDataFormat(&c_dfDIJoystick);
	device->Acquire();

	input->joystickCount += 1;
	input->joysticks = (Joystick*)realloc(input->joysticks, input->joystickCount * sizeof(Joystick));
	Joystick joystick = {0};
	joystick.device = device;
	input->joysticks[input->joystickCount - 1] = joystick;

	return DIENUM_CONTINUE;
}

void updateInput(Input* input)
{
	// Update joysticks
	DIJOYSTATE state;
	if (input->joysticks[0].device->GetDeviceState(sizeof(state), &state) == DI_OK)
	{
		memcpy(input->joysticks[0].previousInputs, input->joysticks[0].currentInputs, Joystick::inputCount * sizeof(float));

		// Convert axes to [0, 1] range, with + and - directions mapped to separate inputs
		input->joysticks[0].currentInputs[0]  = fmaxf( state.lX           / (float)SHRT_MAX - 1, 0);
		input->joysticks[0].currentInputs[1]  = fmaxf(-state.lX           / (float)SHRT_MAX + 1, 0);
		input->joysticks[0].currentInputs[2]  = fmaxf( state.lY           / (float)SHRT_MAX - 1, 0);
		input->joysticks[0].currentInputs[3]  = fmaxf(-state.lY           / (float)SHRT_MAX + 1, 0);
		input->joysticks[0].currentInputs[4]  = fmaxf( state.lZ           / (float)SHRT_MAX - 1, 0);
		input->joysticks[0].currentInputs[5]  = fmaxf(-state.lZ           / (float)SHRT_MAX + 1, 0);
		input->joysticks[0].currentInputs[6]  = fmaxf( state.lRx          / (float)SHRT_MAX - 1, 0);
		input->joysticks[0].currentInputs[7]  = fmaxf(-state.lRx          / (float)SHRT_MAX + 1, 0);
		input->joysticks[0].currentInputs[8]  = fmaxf( state.lRy          / (float)SHRT_MAX - 1, 0);
		input->joysticks[0].currentInputs[9]  = fmaxf(-state.lRy          / (float)SHRT_MAX + 1, 0);
		input->joysticks[0].currentInputs[10] = fmaxf( state.lRz          / (float)SHRT_MAX - 1, 0);
		input->joysticks[0].currentInputs[11] = fmaxf(-state.lRz          / (float)SHRT_MAX + 1, 0);
		input->joysticks[0].currentInputs[12] = fmaxf( state.rglSlider[0] / (float)SHRT_MAX - 1, 0);
		input->joysticks[0].currentInputs[13] = fmaxf(-state.rglSlider[0] / (float)SHRT_MAX + 1, 0);
		input->joysticks[0].currentInputs[14] = fmaxf( state.rglSlider[1] / (float)SHRT_MAX - 1, 0);
		input->joysticks[0].currentInputs[15] = fmaxf(-state.rglSlider[1] / (float)SHRT_MAX + 1, 0);

		// Convert each hat direction to be 0 or 1
		DWORD hat = state.rgdwPOV[0];
		input->joysticks[0].currentInputs[16] = (hat==0     || hat==4500  || hat==31500)? 1.0f : 0.0f;
		input->joysticks[0].currentInputs[17] = (hat==4500  || hat==9000  || hat==13500)? 1.0f : 0.0f;
		input->joysticks[0].currentInputs[18] = (hat==13500 || hat==18000 || hat==22500)? 1.0f : 0.0f;
		input->joysticks[0].currentInputs[19] = (hat==22500 || hat==27000 || hat==31500)? 1.0f : 0.0f;

		// Convert each button to be 1 when pressed, 0 when not pressed
		for (unsigned int buttonIndex = 0; buttonIndex < 32; ++buttonIndex) {
			input->joysticks[0].currentInputs[20 + buttonIndex] = state.rgbButtons[buttonIndex]? 1.0f : 0.0f;
		}
	}

	// Update Keyboard
	memcpy(input->previousKeyboard, input->currentKeyboard, Input::keyCount * sizeof(bool));
	for (unsigned int i=8 /*skip mouse buttons*/; i<Input::keyCount; ++i) {
		input->currentKeyboard[i] = GetAsyncKeyState(i) & 0x8000 ? true : false;
	}
}

bool bindInput(InputBinding* binding, Input input)
{
	for (unsigned int i=0; i<Input::keyCount; ++i) {
		if (input.currentKeyboard[i] && !input.previousKeyboard[i]) {
			binding->keyIndex = i;
			binding->keyBound = true;
			return true;
		}
	}
	for (unsigned int i=0; i<Joystick::inputCount; ++i) {
		if (input.joysticks[0].currentInputs[i] > 0.5f && input.joysticks[0].previousInputs[i] < 0.5f) {
			binding->joystickInputIndex = i;
			binding->joystickBound = true;
			return true;
		}
	}
	return false;
}

bool isInputActive(InputBinding binding, Input input)
{
	return (binding.keyBound && input.currentKeyboard[binding.keyIndex])
		|| (binding.joystickBound && input.joysticks[0].currentInputs[binding.joystickInputIndex] > 0.5f);
}

Vec2 get2dAnalogInput(InputBinding up, InputBinding down, InputBinding left, InputBinding right, Input input)
{
	// Ignore joystick input if any of the keys are pressed.
	// Mixing the two can cause drifting from small values in analog sticks.
	float upValue    = up.keyBound    && input.currentKeyboard[up.keyIndex]? 1.0f : 0.0f;
	float downValue  = down.keyBound  && input.currentKeyboard[down.keyIndex]? 1.0f : 0.0f;
	float leftValue  = left.keyBound  && input.currentKeyboard[left.keyIndex]? 1.0f : 0.0f;
	float rightValue = right.keyBound && input.currentKeyboard[right.keyIndex]? 1.0f : 0.0f;
	if (upValue==0 && downValue==0 && leftValue==0 && rightValue==0)
	{
		if (up.joystickBound) upValue    = input.joysticks[0].currentInputs[up.joystickInputIndex];
		if (down.joystickBound) downValue  = input.joysticks[0].currentInputs[down.joystickInputIndex];
		if (left.joystickBound) leftValue  = input.joysticks[0].currentInputs[left.joystickInputIndex];
		if (right.joystickBound) rightValue = input.joysticks[0].currentInputs[right.joystickInputIndex];	
	}
	Vec2 result = {rightValue-leftValue, upValue-downValue};
	return result;
}

int main()
{
	HINSTANCE hInstance = GetModuleHandle(0);
	Input input = {0};
	DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&input.dinput, 0);
	input.dinput->EnumDevices(DI8DEVCLASS_GAMECTRL, DirectInputEnumDevicesCallback, (void*)&input, DIEDFL_ALLDEVICES);

	// Initialize current inputs so previous inputs will be accurate on first frame
	updateInput(&input);

	if (input.joystickCount == 0) {
		printf("No joysticks connected\n");
		return 1;
	}

	puts("Configure inputs for keyboard and joystick 0. Press Esc to undo");
	InputBinding bindings[action_count] = {0};
	unsigned int editBindingIndex = 0;

	while (1)
	{
		updateInput(&input);
		if (editBindingIndex < action_count)
		{
			printf("\rEdit binding for %s        ", inputActionNames[editBindingIndex]);

			if (input.currentKeyboard[VK_ESCAPE]) {
				if (!input.previousKeyboard[VK_ESCAPE] && editBindingIndex > 0) {
					--editBindingIndex;
				}
			}
			else if (bindInput(&bindings[editBindingIndex], input)) {
				++editBindingIndex;
			}
		}
		else
		{
			if (input.currentKeyboard[VK_ESCAPE]) {
				editBindingIndex = 0;
			}

			printf("\rPlay game: ");
			Vec2 movement = get2dAnalogInput(bindings[action_moveUp], bindings[action_moveDown], bindings[action_moveLeft], bindings[action_moveRight], input);
			printf("movement: x% 1.2f y% 1.2f ", movement.x, movement.y);
			if (isInputActive(bindings[action_shoot], input)) printf("BLAM ");
			if (isInputActive(bindings[action_jump], input)) printf("boing!");
			printf("                            ");
		}

		Sleep(16);
	}
}
