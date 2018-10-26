/*
 * device.h
 *
 *  Created on: Aug 24, 2017
 *      Author: technix
 */

#ifndef SYSTEM_INCLUDE_DREAMOS_RT_DEVICE_H_
#define SYSTEM_INCLUDE_DREAMOS_RT_DEVICE_H_

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/cdefs.h>
#include <sys/stat.h>

typedef struct device_s device_t;
typedef struct driver_s driver_t;

struct device_s
{
	const driver_t *driver;
	char *name;
	uint32_t open_count;
};

struct driver_s
{
	// Driver information
	const char *const name;
	void (*const load)(device_t *);

	int (*const open)(device_t *, int, ...);
	int (*const close)(device_t *);
	int (*const read)(device_t *, void *, size_t);
	int (*const write)(device_t *, const void *, size_t);
	int (*const ioctl)(device_t *, unsigned long, ...);
	int (*const poll)(device_t *, short, short *);
	int (*const fstat)(device_t *, struct stat *);
	off_t (*const lseek)(device_t *, off_t, int);
	int (*const isatty)(device_t *);
};

#endif /* SYSTEM_INCLUDE_DREAMOS_RT_DEVICE_H_ */
