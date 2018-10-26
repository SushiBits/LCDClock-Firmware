/*
 * stdio.c
 *
 *  Created on: Aug 27, 2017
 *      Author: technix
 */

#include <sys/fcntl.h>
#include <unistd.h>
#include <stdio.h>

__attribute__((constructor(1000))) void stdio_init(void)
{
	int tty0 = open("/dev/tty0", O_RDONLY);
	dup2(tty0, STDIN_FILENO);
	if (tty0 != STDIN_FILENO)
		close(tty0);

	tty0 = open("/dev/tty0", O_WRONLY);
	dup2(tty0, STDOUT_FILENO);
	dup2(tty0, STDERR_FILENO);

	if (tty0 != STDOUT_FILENO && tty0 != STDERR_FILENO)
		close(tty0);

	setvbuf(stdin, NULL, _IONBF, 0);
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
}
