#ifndef TTY_H
#define TTY_H

#include "config.h"

#ifndef TTY_SERIAL
#define TTY_SERIAL Serial
#endif

void setupTTY()
{
	TTY_SERIAL.begin(115200);
	while (WAIT_FOR_SERIAL && !TTY_SERIAL)
		delay(1);

	TTY_SERIAL.println("Serial setup complete");
}

String readTTY()
{
	String command;

	if (TTY_SERIAL.available())
	{
		delay(0);
		command = TTY_SERIAL.readStringUntil('\n');
		if (command.length() > 0)
		{
			if (command.endsWith("\r"))
				command = command.substring(0, command.length() - 1);

			TTY_SERIAL.print("Received Corename or Command: "); TTY_SERIAL.println(command.c_str());
		}
	}

	return command;
}

#endif
