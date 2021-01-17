
##Types of devices
###HID
Standing for Human Interface Device, HID is a standardized communication protocol for USB devices a user directly interacts with, covering anything from keyboards to treadmils. An HID `usage page` is a number describing the purpose of the device, and `usages` are futher subcatagories. Nearly all game controllers use HID usage page 0x01 with usage 0x04 or 0x05, and Windows has several APIs for working with them. You can read the official usage table here: https://usb.org/sites/default/files/hut1_21_0.pdf

HID refers to both the standard protocol, physical devices (the gamepad in your hands), and virtual devices. Games interface with the virtual device while a driver handles communicating those bytes to the physical one. I'll be redundently referring to game controllers as HID devices.

###XInput
The only controllers you're likely to find that don't use HID are Mirosoft's own XBox controllers. These use a custom communication protocol, and Microsoft introduced a new API to work with them. We'll refer to the driver, the API, and the protocol as XInput.

The XInput API is one of the simplest APIs Microsoft has ever made and is extremly easy to use, but it only works with XBox controllers. This is nice for indie developers as XBox controllers are a small, consistant subset of the wide world of USB game controllers, and doing XBox support well is signigficanlty easier than supporting all possible controllers. The downside is alienating players who only have a PS4 controller, or some other controller that uses HID.

It's also possible to reverse engineer the XInput protocol and communicate with XBox controllers directly. MMozeiko has a excellent example program showing this: https://gist.github.com/mmozeiko/b8ccc54037a5eaf35432396feabbe435

XBox controllers also work on older games that only support HID. The XInput driver creates a virtual HID device and forwards inputs to it. Effectivly there are two copies of each XBox controller on your system, the XInput version and the HID version. The HID version comes with a big, annoying downside: the XInput driver combines the shoulder tiggers into one axis when converted to HID. Pressing RT makes the axis go in the negative direction, and LT makes it go in the positive direction. If you press both at the same time, it's the same as pressing neither.

So, you need XInput to get full correct input from XBox controllers, and you need HID to get input from any other controller. In order to do joystick support well, a game needs to implement support for both. You can get a list of all HID devices on your system to check which ones are XBox controllers and ignore them with the HID API, then use XInput to process them. There's unfortunately no way to associate a virtual HID device with an XInput player index.

##Inputs
The basic inputs on joysticks are
-Buttons, delivered as bitflags.
-Values for analog sticks and shoulder triggers. Delivered as signed or unsigned integers. Analog sticks have 2 axes, X and Y, that range from MIN_SHORT to MAX_SHORT, with 0 being centered. Triggers can have the same range, or go from 0 to MAX_SHORT.
- A POV hat for the d-pad. This can be implemented as bitflags or a integer representing a direction, such as 1 to the north, 2 to north-east, 3 to east, etc. and 0 for being centered.

Joysticks may support non-standard inputs such as touchpads and accelerometers. For HID devices, these are delivered along with standardized data, but are specific to the device and must be reverse-engineered. [Here's some reverse-engineering of a Playstation 4 controller](http://eleccelerator.com/wiki/index.php?title=DualShock_4).

##Outputs
Older joysticks supported HID force feedback, but not the controllers currently popular for games; XBox and PS4 controllers can't do rumble through HID. XInput provides a function to cause rumble on XBox controllers. You can set rumble and LED color on PS4 controllers by directly writing reverse-engineered data through RawInput.

##APIs
We'll do a brief overview of each API. The `src` folder contains small example programs to illustrate implementation details. You can run them in Visual Studio by opening `Joystick Input Examples.sln` in the `vs` folder, or by running `build.bat` with a Visual Studio developer command line and launching the `.exe`s.

###Multimedia
Multimedia Joystick is the simplest HID API for Windows. The `Ex` version of the functions and structs support up to 16 controllers at a time, each with up to 32 buttons, 6 axes, and a hat.

Getting the joystick state is one function call to `joyGetPosEx`, which takes a device index and a struct to fill with data. Documentation for this function states the device index starts at zero, but on my Windows 10 machine it starts at 1 (https://docs.microsoft.com/en-us/windows/win32/api/joystickapi/nf-joystickapi-joygetposex).

Multimedia's functionality is quite limited. It doesn't give you any handles to hardware, so you can't tell what's an Xbox controller to use XInput instead. It also doesn't have any force feedback support.

###DirectInput
DirectInput is lower level and gives a bit more control. First create a DirectInput object and use it to enumerate HID devices.

MSDN provides a function to check if a device is an XBox controller while enumerating HID devices. It searches through all devices on the system to see if they have "IG_" in the device name. If it does, it compares the product and vendor IDs to the direct input device passed into the function, and if they match, returns that the device is an xbox controller.

###RawInput
The first step with RawInput is registering to recieve input `WM_INPUT` events. Unlike the other APIs, RawInput uses an event queue to get inputs. This has a distinct advantage of being able to get inputs that may have been missed between frames, such as a button being quickly pressed and released (though most games don't worry about this, and players are unlikely to notice).

To register for events, fill out one or more `RAWINPUTDEVICE` structs and pass an array of them to RegisterRawInputDevices(). The structs take the HID usage page and usages of devices for which to receive events, bit flags, and a handle to the window whose window procedure will receive the events. You can use your game window's existing procedure, or create a non-visible window to encapsulate joystick code. The `rawinput` and `combined` examples use a non-visible window to get events in a terminal-based program.

The documentation for all this on [Microsoft's website](https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/_hid/) is pretty sparse and hard to understand. Open up hidpi.h for much better explanations.

###XInput
XInput is simple API similar to multimedia joystick. It only works with XBox controllers, but makes getting controller state and setting rumble nice and easy.

XInputGetState() has been known to cause a several milisecond hang when trying to access nonexistant devices, for example, asking for player 2 input when only one controller is plugged in. You'll probably want to query which controller indices are available once, and only get regular updates from devices you know are connected. See the [Detecting Device Changes](#Detecting Device Changes) section for more details.

### Libraries
The most popular joystick libraries are SDL's Joystick and Game Controller interfaces. Joystick provides generic button, axis, and hat data. Game Controller referes to inputs in terms of an xbox controller, giving uniform semantics to all kinds of controllers (see [Controller Database section](#Controller Database)).

## Button configuration
Ideally, players should have full control over how their controller inputs map to game actions with an intuitive interface. Some players will want to play your retro-style game with a super nintendo controller going through a random SNES-to-USB converter they found on ebay. Other players will have physical disabbilities and need to use unusual button mappings or custom-built controllers. Button config is crutial for player experience, but presents numerous complications for developers.

The gold-standard for button config is press-to-set. The player selects a game action, then presses the corresponding button. Even better is if the next action is automatically selected, so the player can quickly map all the buttons in a row. Directional inputs can also be configurable by separating an analog stick into -x, +x, -y, and +y. So to do full button config you need to iterate through all inputs on the controller: check if a button is pressed, if an axis passed a threshold, or if the hat is not centered. The `Combined` example puts all inputs into an array of floats, each in the range `[0,1]` (buttons and cardinal hat directions are 0 or 1 while analog inputs can be inbetween). This makes button config code easy as the inputs are generic.

The first problem with this kind of button config is that axes are not always centered at rest. Shoulder triggers on XBox controllers are 0 at rest, 255 when fully pressed. If these were mapped to HID just like any other axis, it would appear to a game that axis is always being pushed in the negative direction. This is Microsoft's rationale for mapping both triggers to the same axis for HID. The correct inplementation would have been to map them to separate axes that are 0 at rest, and forget about the negative range, but we're stuck with this decision. If you want to support as many controllers as possible, you have to deal with this anyway: PS4 and Gamecube controllers report the shoudler triggers as negative at rest.

PS4 controllers have another problem: R2 and L2 report both a button and an axis. This could lead to the being mistakenly mapped to two actions, or the button being mapped where an analog value is wanted. The `Combined` example throws out the button to avoid this problem, as well as mapping shoulder triggers to a `[0,1]` range.

So what of generic HID devices? How do we correctly map inputs when they could be anything?

### Controller Database
SDL's Game Controller interface brings the idea of expressing every controller one could possibly plug into a PC in terms of an XBox controller. They crowd-sourced a database that maps hardware IDs to button configurations, eliminating the headaches of generic input. You can then apply your game's own mapping, and effectivly only worry about xbox controllers while support many more than that. SDL includes the database in its source code and is ready to go if you use the Game Controller interface. You can also download the database and parse it yourself if you're not using SDL: https://github.com/gabomdq/SDL_GameControllerDB

The one problem with the database approch is that it doesn't work with USB converters that support multiple types of controllers. The device only has one hardware ID for all the controllers. For example, I have a converter which supports PS2, Gamecube, and origonal XBox controllers: https://www.mayflash.com/Products/UNIVERSAL/PC035.html
Somebody at some point made an entry in the SDL database for this device, and mapped it for gamecube controllers. I use it for a PS2 controller, so the mapping is totally wrong and renders some buttons unusable. Still, the database approch will work fantastically well for nearly all of your players.

### Callibration

### Cutting our losses
Really we can still get decent button config without a database or callibration. Let's imagine a player configuring their Gamecube controller in a press-to-set interface. The want to map the Shoot action to the right shoulder trigger, so they select the action and press down R. The input was negative both this frame and last, so we ignore the initial state. It isn't until the input becomes positive that we create the mapping. After Shoot is Jump, and when the player releases R they accidently map Jump to a negative axis input. They curse, redo that mapping, and move on. So while not perfect, it does support an uncommon controller and lets the user create any mappings they want.

##Detecting Device Changes
Players may not have all their controllers connected when they launch your game. 