#include <stdio.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/joystick.h>
#include <linux/input.h>

const char* buttonNames[32] = {
	"TRIGGER",
	"THUMB",
	"THUMB2",
	"TOP",
	"TOP2",
	"PINKIE",
	"BASE",
	"BASE2",
	"BASE3",
	"BASE4",
	"BASE5",
	"BASE6",
	"","","",
	"DEAD",

	"SOUTH",
	"EAST",
	"C",
	"NORTH",
	"WEST",
	"Z",
	"TL",
	"TR",
	"TL2",
	"TR2",
	"SELECT",
	"START",
	"MODE",
	"THUMBL",
	"THUMBR"
};

const char* axisNames[32] = {
	"X",
	"Y",
	"Z",
	"RX",
	"RY",
	"RZ",
	"THROTTLE",
	"RUDDER",
	"WHEEL",
	"GAS",
	"BRAKE",
	"","","","","",
	"HAT0X",
	"HAT0Y",
	"HAT1X",
	"HAT1Y",
	"HAT2X",
	"HAT2Y",
	"HAT3X",
	"HAT3Y",
	"PRESSURE",
	"DISTANCE",
	"TILT_X",
	"TILT_Y",
	"TOOL_WIDTH"
};

struct Axis
{
	int min;
	int max;
	float value;
};

struct Joystick
{
	static const unsigned int maxButtons = 32;
	static const unsigned int maxAxes = 32;

	bool connected;
	bool buttons[maxButtons];
	Axis axes[maxAxes];
	char name[128];
	int file;
	bool hasRumble;
	short rumbleEffectID;
};

Joystick openJoystick(const char* fileName)
{
	Joystick j = {0};
	int file = open(fileName, O_RDWR | O_NONBLOCK);
	if (file != -1)
	{
		ioctl(file, EVIOCGNAME(sizeof(j.name)), j.name);
		j.connected = true;
		j.file = file;

		// Setup axes
		for (unsigned int i=0; i<Joystick::maxAxes; ++i)
		{
			input_absinfo axisInfo;
			if (ioctl(file, EVIOCGABS(i), &axisInfo) != -1)
			{
				j.axes[i].min = axisInfo.minimum;
				j.axes[i].max = axisInfo.maximum;
			}
		}

		// Setup rumble
		ff_effect effect = {0};
		effect.type = FF_RUMBLE;
		effect.id = -1;
		if (ioctl(file, EVIOCSFF, &effect) != -1) {
			j.rumbleEffectID = effect.id;
			j.hasRumble = true;
		}
	}
	return j;
}

void readJoystickInput(Joystick* joystick)
{
	input_event event;
	while (read(joystick->file, &event, sizeof(event)) > 0)
	{
		if (event.type == EV_KEY && event.code >= BTN_JOYSTICK && event.code <= BTN_THUMBR) {
			joystick->buttons[event.code-0x120] = event.value;
		}
		if (event.type == EV_ABS && event.code < ABS_TOOL_WIDTH) {
			Axis* axis = &joystick->axes[event.code];
			float normalized = (event.value - axis->min) / (float)(axis->max - axis->min) * 2 - 1;
			joystick->axes[event.code].value = normalized;
		}
	}
}

void setJoystickRumble(Joystick joystick, short weakRumble, short strongRumble)
{
	if (joystick.hasRumble)
	{
		ff_effect effect = {0};
		effect.type = FF_RUMBLE;
		effect.id = joystick.rumbleEffectID;
		effect.replay.length = 5000;
		effect.replay.delay = 0;
		effect.u.rumble.weak_magnitude = weakRumble;
		effect.u.rumble.strong_magnitude = strongRumble;
		if (ioctl(joystick.file, EVIOCSFF, &effect) == -1) {
			puts("error");
		}

		input_event play = {0};
		play.type = EV_FF;
		play.code = joystick.rumbleEffectID;
		play.value = 1;
		if (write(joystick.file, &play, sizeof(play)) == -1)
		{
			puts("error writing");
		}
	}
}

int main()
{
	const unsigned int maxJoysticks = 32;
	Joystick joysticks[maxJoysticks] = {0};

	char fileName[32];
	int joystick = 0;
	for (int i=0; i<32; ++i) {
		sprintf(fileName, "/dev/input/event%d", i);
		joysticks[i] = openJoystick(fileName);
	}

	//char name[128];
	//ioctl(joystick, EVIOCGNAME(sizeof(name)), name);
	//printf("Name: %s - ", name);

	//char did[128];
	//ioctl(joystick, EVIOCGUNIQ(sizeof(did)), did);
	//printf("DID: %s - ", did);

	//char uid[128];
	//ioctl(joystick, EVIOCGUNIQ(sizeof(uid)), uid);
	//printf("UID: %s - ", uid);

	//char props[128];
	//ioctl(joystick, EVIOCGPROP(sizeof(props)), props);
	//printf("Props: %s - ", props);

	//ff_effect effect = {0};
	//effect.type = FF_RUMBLE;
	//effect.id = -1;
	//effect.replay.length = 5000;
	//effect.replay.delay = 0;
	//effect.u.rumble.weak_magnitude = 0xc000;
	//if (ioctl(joystick, EVIOCSFF, &effect) == -1) {
	//	puts("error");
	//}

	//input_event play = {0};
	//play.type = EV_FF;
	//play.code = effect.id;
	//play.value = 1;
	//if (write(joystick, &play, sizeof(play)) == -1)
	//{
	//	puts("error writing");
	//}


	while (1)
	{
		for (unsigned int i=0; i<maxJoysticks; ++i)
		{
			if (joysticks[i].connected)
			{
				readJoystickInput(&joysticks[i]);

				printf("%s - Axes: ", joysticks[i].name);
				for (char axisIndex=0; axisIndex<Joystick::maxAxes; ++axisIndex) {
					if (joysticks[i].axes[axisIndex].max-joysticks[i].axes[axisIndex].min) printf("%s:% f ", axisNames[axisIndex], joysticks[i].axes[axisIndex].value);
				}
				printf("Buttons: ");
				for (char buttonIndex=0; buttonIndex<Joystick::maxButtons; ++buttonIndex) {
					if (joysticks[i].buttons[buttonIndex]) printf("%s ", buttonNames[buttonIndex]);
				}
				printf("\n");

				short weakRumble   = fabsf(joysticks[i].axes[ABS_X].value) * 0xFFFF;
				short strongRumble = fabsf(joysticks[i].axes[ABS_Y].value) * 0xFFFF;
				setJoystickRumble(joysticks[i], weakRumble, strongRumble);
			}
		}
		fflush(stdout);
		usleep(16000);
	}
}
