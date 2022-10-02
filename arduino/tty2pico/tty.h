#ifndef TTY_H
#define TTY_H

#include "config.h"

void setupTTY()
{
	Serial.begin(TTY_BAUDRATE);
#if defined(WAIT_FOR_SERIAL) && WAIT_FOR_SERIAL == 1
	while (!Serial) delay(10);
#endif
	Serial.println("Serial setup complete");
}

String readTTY()
{
	static String command;

	if (Serial.available())
	{
		command = Serial.readStringUntil('\n');
		if (command.length() > 0)
		{
			Serial.print("Received Corename or Command: "); Serial.println(command.c_str());
		}
		else command = "";
	}
	else command = "";

	return command;
}

#endif
