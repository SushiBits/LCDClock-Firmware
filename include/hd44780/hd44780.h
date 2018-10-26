/*
 * hd44780.h
 *
 *  Created on: Jul 9, 2018
 *      Author: technix
 */

#ifndef INCLUDE_HD44780_HD44780_H_
#define INCLUDE_HD44780_HD44780_H_

#include <stdint.h>
#include <stdbool.h>

enum
{
	IOCTL_HD44780_COMMAND		= 0b00000000,
	IOCTL_HD44780_CLEAR			= 0b00000001,
	IOCTL_HD44780_HOME			= 0b00000010,
	IOCTL_HD44780_ENTRY_MODE	= 0b00000100,
	IOCTL_HD44780_ON_OFF		= 0b00001000,
	IOCTL_HD44780_SHIFT			= 0b00010000,
	IOCTL_HD44780_FUNCTION_SET	= 0b00100000,
	IOCTL_HD44780_SET_CRAM_ADDR	= 0b01000000,
	IOCTL_HD44780_SET_VRAM_ADDR	= 0b10000000,
	IOCTL_HD44780_GET_BUSY_ADDR	= 0b11111111
};

typedef enum hd44780_direction_s
{
	HD44780_DIRECTION_DECREMENT = 0,
	HD44780_DIRECTION_INCREMENT = 1
} hd44780_direction_t;

typedef struct hd44780_entry_mode_s
{
	 hd44780_direction_t direction;
	 bool shift_cursor;
} hd44780_entry_mode_t;

typedef struct hd44780_on_off_s
{
	bool display;
	bool cursor;
	bool blink;
} hd44780_on_off_t;

typedef struct hd44780_shift_s
{
	enum
	{
		HD44780_SHIFT_CURSOR,
		HD44780_SHIFT_DISPLAY
	} shift;
	hd44780_direction_t direction;
} hd44780_shift_t;

typedef struct hd44780_function_s
{
	enum
	{
		HD44780_LINES_ONE,
		HD44780_LINES_TWO
	} lines;
	enum
	{
		HD44780_FONT_5x8,
		HD44780_FONT_5x10
	} font;
} hd44780_function_t;

typedef uint8_t hd44780_addr_t;

#endif /* INCLUDE_HD44780_HD44780_H_ */
