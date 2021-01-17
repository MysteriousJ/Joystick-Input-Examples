#include <Windows.h>
#include <stdio.h>

int main()
{
	while (1)
	{
		JOYINFOEX joy;
		MMRESULT result = joyGetPosEx(0, &joy);
		if (result == JOYERR_NOERROR)
		{
			printf("Axes: ");
			printf("0:%5d, ", joy.dwXpos);
			printf("1:%5d, ", joy.dwYpos);
			printf("2:%5d, ", joy.dwZpos);
			printf("3:%5d, ", joy.dwRpos);
			printf("4:%5d, ", joy.dwUpos);
			printf("5:%5d, ", joy.dwVpos);
			printf("hat:%5d, ", joy.dwPOV);
			printf("Buttons: ");
			for (int i=0; i<32; ++i) {
				if (joy.dwButtons & (1<<i)) printf("%d ", i);
			}
		}
		printf("\n");
		Sleep(16);
	}
	return 0;
}
