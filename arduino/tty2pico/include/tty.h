#ifndef TTY_H
#define TTY_H

#include "config.h"

#ifndef TTY_SERIAL
#define TTY_SERIAL Serial
#endif

void setupTTY()
{
	TTY_SERIAL.begin(TTY_BAUDRATE);
#if defined(WAIT_FOR_SERIAL) && WAIT_FOR_SERIAL == 1
	while (!TTY_SERIAL) delay(10);
#endif
	TTY_SERIAL.println("Serial setup complete");
}

String readTTY()
{
	static String command;

	if (TTY_SERIAL.available())
	{
		command = TTY_SERIAL.readStringUntil('\n');
		if (command.endsWith("\r"))
			command = command.substring(0, command.length() - 1);
		if (command.length() > 0)
		{
			TTY_SERIAL.print("Received Corename or Command: "); TTY_SERIAL.println(command.c_str());
		}
		else command = "";
	}
	else command = "";

	return command;
}

#endif
