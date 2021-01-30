#include <Windows.h>
#include <stdio.h>

int main()
{
	while (1)
	{
		for (UINT joystickIndex=0; joystickIndex<16; ++joystickIndex)
		{
			JOYINFOEX joy;
			MMRESULT result = joyGetPosEx(joystickIndex, &joy);
			if (result == JOYERR_NOERROR)
			{
				printf("Joystick %d ", joystickIndex);
				printf("X:%5d ", joy.dwXpos);
				printf("Y:%5d ", joy.dwYpos);
				printf("Z:%5d ", joy.dwZpos);
				printf("R:%5d ", joy.dwRpos);
				printf("U:%5d ", joy.dwUpos);
				printf("V:%5d ", joy.dwVpos);
				printf("hat:%5d ", joy.dwPOV);
				printf("Buttons: ");
				for (int i=0; i<32; ++i) {
					if (joy.dwButtons & (1<<i)) printf("%d ", i);
				}
				printf("\n");
			}
		}
		Sleep(16);
	}
	return 0;
}
