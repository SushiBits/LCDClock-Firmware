/*
 * main.c
 *
 *  Created on: Mar 17, 2018
 *      Author: technix
 */

#include <stm32f1xx.h>
#include <dreamos-rt/gpio.h>
#include <stdio.h>
#include <stdlib.h>
#include "hd44780/hd44780.h"
#include <dreamos-rt/usart.h>

FILE *lcd;

int main(void)
{
	pinMode(0x17, OUTPUT);
	digitalWrite(0x17, 1);

	pinMode(0x16, OUTPUT);
	digitalWrite(0x16, 1);

	lcd = fopen("/dev/lcd", "rw");

	for (;;)
		__WFE();
}
