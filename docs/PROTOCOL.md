# BenQ I2C

The BenQ dongle has the following buttons:
	- Exit
	- Select
	- Scroll Up
	- Scroll Down
	- Auto
	- SW1
	- SW2
	- SW3

The communication between the monitor and the dongle happens through I2C. The
dongle functions as a peripheral (or 'slave'), while the monitor is the
controller (or 'master'). The monitor supplies the dongle with 3.3V (see the GND
and VDD pins).

When the user clicks a button, the dongle sets the INT pin high, the monitor
will then issue a write to `0x28` (the address of the dongle). Depending on
which button was pressed, the dongle responds with a different sequence. When
the action is completed, the INT pin is set low again.
