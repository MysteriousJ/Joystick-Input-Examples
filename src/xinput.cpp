#include <Windows.h>
#include <Xinput.h>
#include <stdio.h>

int main()
{
	while (1)
	{
		for (DWORD playerIndex=0; playerIndex < 4; ++playerIndex)
		{
			XINPUT_STATE state;
			DWORD result = XInputGetState(playerIndex, &state);
			if (result == ERROR_SUCCESS && state.dwPacketNumber)
			{
				printf("Player %d ", playerIndex);
				printf("LX:%6d ", state.Gamepad.sThumbLX);
				printf("LY:%6d ", state.Gamepad.sThumbLY);
				printf("RX:%6d ", state.Gamepad.sThumbRX);
				printf("RY:%6d ", state.Gamepad.sThumbRY);
				printf("LT:%3u ", state.Gamepad.bLeftTrigger);
				printf("RT:%3u ", state.Gamepad.bRightTrigger);
				printf("Buttons: ");
				if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP)        printf("up ");
				if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN)      printf("down ");
				if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT)      printf("left ");
				if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)     printf("right ");
				if (state.Gamepad.wButtons & XINPUT_GAMEPAD_START)          printf("start ");
				if (state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK)           printf("back ");
				if (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB)     printf("LS ");
				if (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB)    printf("RS ");
				if (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)  printf("LB ");
				if (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) printf("RB ");
				if (state.Gamepad.wButtons & XINPUT_GAMEPAD_A)              printf("A ");
				if (state.Gamepad.wButtons & XINPUT_GAMEPAD_B)              printf("B ");
				if (state.Gamepad.wButtons & XINPUT_GAMEPAD_X)              printf("X ");
				if (state.Gamepad.wButtons & XINPUT_GAMEPAD_Y)              printf("Y ");
				printf("\n");

				XINPUT_VIBRATION vibration;
				vibration.wLeftMotorSpeed  = state.Gamepad.bLeftTrigger  * 0xFFFF / 0xFF;
				vibration.wRightMotorSpeed = state.Gamepad.bRightTrigger * 0xFFFF / 0xFF;
				XInputSetState(0, &vibration);
			}
		}
		Sleep(16);
	}
}
