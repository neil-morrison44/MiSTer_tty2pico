#ifndef TTY2PICO_UTILS_H
#define TTY2PICO_UTILS_H

#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof(arr[0]))

void trimTrailing(char *str)
{
	int index = -1, i = 0;

	while(str[i] != '\0')
	{
		if(str[i] != ' ' && str[i] != '\t' && str[i] != '\r' && str[i] != '\n')
			index = i;

		i++;
	}

	// Move the NULL terminator to last non-white space character
	str[index + 1] = '\0';
}

#endif
