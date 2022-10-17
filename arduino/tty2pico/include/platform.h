#ifndef TTY2PICO_PLATFORM_H
#define TTY2PICO_PLATFORM_H

#include "config.h"
#include "commands.h"
#include "pico/stdlib.h"
#include "hardware/vreg.h"

static queue_t cmdQ;

void setupCPU(void)
{
	if (config.enableOverclock)
	{
		// Apply an overclock to 250MHz (2x stock) and voltage tweak to stablize most RP2040 boards.
		// If it's good enough for pixel-pushing in MicroPython, it's good enough for us :P
		// https://github.com/micropython/micropython/issues/8208
		vreg_set_voltage(VREG_VOLTAGE_1_20); // Set voltage to 1.2v
		delay(10); // Allow vreg time to stabilize
		set_sys_clock_khz(250000, true); // Overclock to 250MHz
	}
}

void setupQueue(void)
{
	queue_init(&cmdQ, sizeof(CommandData), 1);
}

void addToQueue(CommandData &data)
{
	queue_try_add(&cmdQ, &data);
}

void loopQueue(void)
{
	static CommandData data;

	while (queue_try_remove(&cmdQ, &data))
		runCommand(data);
}

#endif
