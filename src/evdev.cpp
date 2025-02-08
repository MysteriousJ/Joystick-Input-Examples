#include <stdio.h>
#include <math.h>
#include <sys/ioctl.h>
#include <sys/inotify.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/joystick.h>
#include <linux/input.h>
#include <filesystem>

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

	"A",
	"B",
	"C",
	"X",
	"Y",
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

struct JoystickState
{
	static const unsigned int maxButtons = 32;
	static const unsigned int maxAxes = 32;

	bool buttons[maxButtons];
	Axis axes[maxAxes];
	char name[128];
	int file;
	bool hasRumble;
	short rumbleEffectID;
};

struct Joysticks
{
	unsigned int count;
	JoystickState* states;
};

Joysticks openJoysticks()
{
	Joysticks result = {0};
	for (auto const& entry : std::filesystem::directory_iterator{"/dev/input/by-id"})
	{
		if (entry.path().string().ends_with("event-joystick"))
		{
			int file = open(entry.path().string().c_str(), O_RDWR | O_NONBLOCK);
			if (file != -1)
			{
				JoystickState j = {0};
				j.file = file;

				// Get name
				ioctl(file, EVIOCGNAME(sizeof(j.name)), j.name);

				// Setup axes
				for (unsigned int i=0; i<JoystickState::maxAxes; ++i)
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
				if (ioctl(file, EVIOCSFF, &effect) != -1)
				{
					j.rumbleEffectID = effect.id;
					j.hasRumble = true;
				}

				++result.count;
				result.states = (JoystickState*)realloc(result.states, sizeof(JoystickState) * result.count);
				result.states[result.count-1] = j;
			}
		}
	}
	return result;
}

void closeJoysticks(Joysticks* joysticks)
{
	for (int i=0; i<joysticks->count; ++i)
	{
		close(joysticks->states[i].file);
	}
	free(joysticks->states);
	joysticks->states = 0;
	joysticks->count = 0;
}

void readJoystickInput(JoystickState* joystick)
{
	input_event event;
	while (read(joystick->file, &event, sizeof(event)) > 0)
	{
		if (event.type == EV_KEY && event.code >= BTN_JOYSTICK && event.code <= BTN_THUMBR)
		{
			joystick->buttons[event.code-0x120] = event.value;
		}
		if (event.type == EV_ABS && event.code < ABS_TOOL_WIDTH)
		{
			Axis* axis = &joystick->axes[event.code];
			float normalized = (event.value - axis->min) / (float)(axis->max - axis->min) * 2 - 1;
			joystick->axes[event.code].value = normalized;
		}
	}
}

void setJoystickRumble(JoystickState* joystick, short weakRumble, short strongRumble)
{
	if (joystick->hasRumble)
	{
		ff_effect effect = {0};
		effect.type = FF_RUMBLE;
		effect.id = joystick->rumbleEffectID;
		effect.replay.length = 5000;
		effect.replay.delay = 0;
		effect.u.rumble.weak_magnitude = weakRumble;
		effect.u.rumble.strong_magnitude = strongRumble;
		ioctl(joystick->file, EVIOCSFF, &effect);

		input_event play = {0};
		play.type = EV_FF;
		play.code = joystick->rumbleEffectID;
		play.value = 1;
		write(joystick->file, &play, sizeof(play));
	}
}

int main()
{
	Joysticks joysticks = openJoysticks();

	int deviceChangeNotify = inotify_init1(IN_NONBLOCK);
	inotify_add_watch(deviceChangeNotify, "/dev/input/by-id", IN_CREATE | IN_DELETE);

	while (1)
	{
		// Update list of joysticks
		char unneededEventData[4096];
		if (read(deviceChangeNotify, unneededEventData, sizeof(unneededEventData)) != -1)
		{
			closeJoysticks(&joysticks);
			joysticks = openJoysticks();
		}

		// Update and print inputs for each joystick
		for (unsigned int i=0; i<joysticks.count; ++i)
		{
			JoystickState* j = &joysticks.states[i];
			readJoystickInput(j);

			printf("%s - Axes: ", j->name);
			for (char axisIndex=0; axisIndex<JoystickState::maxAxes; ++axisIndex)
			{
				if (j->axes[axisIndex].max-j->axes[axisIndex].min) printf("%s:% .3f ", axisNames[axisIndex], j->axes[axisIndex].value);
			}
			printf("Buttons: ");
			for (char buttonIndex=0; buttonIndex<JoystickState::maxButtons; ++buttonIndex)
			{
				if (j->buttons[buttonIndex]) printf("%s ", buttonNames[buttonIndex]);
			}
			printf("\n");

			short weakRumble   = fabsf(j->axes[ABS_X].value) * 0xFFFF;
			short strongRumble = fabsf(j->axes[ABS_Y].value) * 0xFFFF;
			setJoystickRumble(j, weakRumble, strongRumble);
		}
		fflush(stdout);
		usleep(16000);
	}
}
