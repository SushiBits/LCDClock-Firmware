/*
 * hd44780.c
 *
 *  Created on: Jul 9, 2018
 *      Author: technix
 */

#include "hd44780/hd44780.h"

#include <stm32f1xx.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>
#include <unistd.h>
#include <stdint.h>
#include <stdarg.h>
#include <sys/fcntl.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <dreamos-rt/device.h>
#include <dreamos-rt/time.h>
#include <dreamos-rt/gpio.h>

struct hd44780_device
{
	device_t device;
};

#define LCD_RS 0x10
#define LCD_RW 0x12
#define LCD_EN 0x1d

#define LCD_RS_CMD 0
#define LCD_RS_DATA 1

static int hd44780_ioctl(device_t *device, unsigned long func, ...);

static int hd44780_read_register(uint8_t address, int delay_ms)
{
	GPIOA->CRL = 0x88888888;
	digitalWrite(LCD_RS, !!address);
	digitalWrite(LCD_RW, 1);
	digitalWrite(LCD_EN, 1);
	usleep(1000);
	uint8_t data = GPIOA->IDR & 0xff;
	digitalWrite(LCD_EN, 0);
	usleep(delay_ms * 1000 + 1000);
	return data;
}

static void hd44780_write_register(uint8_t address, uint8_t data, int delay_ms)
{
	GPIOA->CRL = 0x22222222;
	digitalWrite(LCD_RS, !!address);
	digitalWrite(LCD_RW, 0);
	SET_FIELD(GPIOA->ODR, 0xff, data);
	digitalWrite(LCD_EN, 1);
	usleep(500);
	digitalWrite(LCD_EN, 0);
	usleep(delay_ms * 1000 + 500);
	GPIOA->CRL = 0x88888888;
	SET_FIELD(GPIOA->ODR, 0xff, 0xff);
}

static int hd44780_open(device_t *device, int mode, ...)
{
	GPIOA->CRL = 0x88888888;
	SET_FIELD(GPIOA->ODR, 0xff, 0xff);
	pinMode(LCD_RS, OUTPUT);
	pinMode(LCD_RW, OUTPUT);
	pinMode(LCD_EN, OUTPUT);
	digitalWrite(LCD_RS, 0);
	digitalWrite(LCD_RW, 0);
	digitalWrite(LCD_EN, 0);
	usleep(15000);

	hd44780_write_register(LCD_RS_CMD, 0b00110000, 4);
	hd44780_write_register(LCD_RS_CMD, 0b00110000, 4);
	hd44780_write_register(LCD_RS_CMD, 0b00110000, 4);

	return hd44780_ioctl(device, IOCTL_HD44780_CLEAR, NULL);
}

static int hd44780_close(device_t *device)
{
	GPIOA->CRL = 0x44444444;
	SET_FIELD(GPIOA->ODR, 0xff, 0x00);
	pinMode(LCD_RS, INPUT);
	pinMode(LCD_RW, INPUT);
	pinMode(LCD_EN, INPUT);

	return 0;
}

static int hd44780_read(device_t *device, void *buf, size_t len)
{
	for (uint8_t *cp = buf; (void *)cp - buf < len; cp++)
		*cp = hd44780_read_register(LCD_RS_DATA, 0);

	return len;
}

static int hd44780_write(device_t *device, const void *buf, size_t len)
{
	for (const uint8_t *cp = buf; (const void *)cp - buf < len; cp++)
		hd44780_write_register(LCD_RS_DATA, *cp, 0);

	return len;
}

static int hd44780_ioctl(device_t *device, unsigned long func, ...)
{
	void *value;
	va_list args;
	va_start(args, func);
	value = (void *)va_arg(args, uintptr_t);
	va_end(args);

	if (func == IOCTL_HD44780_GET_BUSY_ADDR)
	{
		hd44780_addr_t *addr = value;
		*addr = hd44780_read_register(LCD_RS_CMD, 0);
	}
	else
	{
		uint8_t data = func;

		switch (func)
		{
		case IOCTL_HD44780_COMMAND:
			data = (uint8_t)value;
			break;

		case IOCTL_HD44780_CLEAR:
		case IOCTL_HD44780_HOME:
			break;

		case IOCTL_HD44780_ENTRY_MODE:
		{

		}
		break;
		}

		hd44780_write_register(LCD_RS_CMD, data, 1);
	}

	return 0;
}

static int hd44780_poll(device_t *device, short events, short *revents)
{
	return -1;
}

static int hd44780_fstat(device_t *device, struct stat *st)
{
	if (st)
	{
		st->st_dev = 0;
		st->st_ino = 0;
		st->st_mode = 0644;
		st->st_nlink = 1;
		st->st_uid = 0;
		st->st_gid = 0;
		st->st_rdev = 1;
		st->st_size = 0;
		st->st_atime = 0;
		st->st_mtime = 0;
		st->st_ctime = 0;
		st->st_blksize = 0;
		st->st_blocks = 0;
	}
	return 0;
}

static int hd44780_isatty(device_t *device)
{
	return 1;
}

static const driver_t hd44780_driver =
{
		.name = "hd44780",
		.load = NULL,
		.open = hd44780_open,
		.close = hd44780_close,
		.read = hd44780_read,
		.write = hd44780_write,
		.ioctl = hd44780_ioctl,
		.poll = hd44780_poll,
		.fstat = hd44780_fstat,
		.isatty = hd44780_isatty
};

static struct hd44780_device hd44780_device =
{
		.device = {
				.name = "lcd",
				.driver = &hd44780_driver
		}

};
__attribute__((section(".devices"))) const struct hd44780_device *LCD = &hd44780_device;
