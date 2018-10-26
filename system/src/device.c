/*
 * device.c
 *
 *  Created on: Aug 24, 2017
 *      Author: technix
 */

#include <dreamos-rt/device.h>
#include <dreamos-rt/parameters.h>
#include <dreamos-rt/time.h>

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#include <poll.h>

#define _COMPILING_NEWLIB // We need newlib internal prototypes.
#include <unistd.h>
#include <sys/fcntl.h>

#include <errno.h>
#undef errno
extern int errno;

extern device_t **__devices_addr;
extern size_t __devices_size;

#define FILE_MAGIC 0x55aa55aa

struct file_desc
{
	uint32_t magic;
	device_t *device;
	uint32_t flags;
};

static struct file_desc files[LIMIT_HANDLES];

static inline device_t *device_named(const char *name)
{
	// Find the base name of the device node name.
	const char *basename = strrchr(name, '/');
	basename = basename ? basename + 1 : name;

	// Find the exact match for the device by name.
	size_t __device_count = __devices_size / sizeof(device_t *);
	for (size_t idx = 0; idx < __device_count; idx++)
	{
		device_t *device = __devices_addr[idx];
		if (!device->name)
			continue;
		if (!strcmp(device->name, basename))
			return device;
	}

	return NULL;
}

static inline struct file_desc *find_file(int fd)
{
	if (fd >= LIMIT_HANDLES)
	{
		errno = EMFILE;
		return NULL;
	}

	struct file_desc *file = &(files[fd]);
	if (file->magic != FILE_MAGIC)
	{
		errno = EBADF;
		return NULL;
	}

	return file;
}

static inline int find_next_fd(void)
{
	int fildes = -1; // Never allocate file numbers for standard streams, use dup2 instead.
	for (int idx = 3; idx < LIMIT_HANDLES; idx++)
	{
		if (files[idx].magic != FILE_MAGIC)
		{
			fildes = idx;
			break;
		}
	}
	if (fildes < 0)
	{
		errno = ENFILE;
		return -1;
	}

	return fildes;
}

static inline uint32_t flags_from_open(int oflags)
{
	uint32_t openness = oflags & 0x7;
	uint32_t rest = oflags & (~0x7);
	return rest | (openness + 1);
}

__attribute__((constructor)) void devices_init(void)
{
	size_t __device_count = __devices_size / sizeof(device_t *);
	for (size_t idx = 0; idx < __device_count; idx++)
	{
		device_t *device = __devices_addr[idx];
		if (!device->driver)
			continue;

		if (device->driver->load)
			device->driver->load(device);

		device->open_count = 0;
	}
}

int _open(const char *file, int oflag, ...)
{
	// Check if device exists.
	device_t *device = device_named(file);
	if (!device || !device->driver)
	{
		errno = ENOENT;
		return -1;
	}

	// Try to allocate a file number.
	int fildes = find_next_fd();
	if (fildes < 0)
		return fildes;

	// Open the device.
	int retval = -1;
	if (device->open_count > 0)
	{
		// The file have been opened. Increase the reference count only.
		retval = 0;
	}
	else
	{
		// The file have not been opened.
		if (device->driver->open)
		{
			if (oflag & O_CREAT == O_CREAT)
			{
				va_list args;
				va_start(args, oflag);
				mode_t mode = va_arg(args, mode_t);
				va_end(args);

				retval = device->driver->open(device, oflag, mode);
			}
			else
			{
				retval = device->driver->open(device, oflag);
			}
		}
		else
		{
			errno = ENOSYS;
			return -1;
		}
	}

	if (retval >= 0)
	{
		files[fildes].magic = FILE_MAGIC;
		files[fildes].device = device;
		files[fildes].flags = flags_from_open(oflag);
		device->open_count++;
	}
	else
	{
		return -1;
	}

	return fildes;
}

int _close(int fd)
{
	struct file_desc *file = find_file(fd);
	if (!file)
		return -1;

	file->device->open_count--;

	if (!file->device->open_count)
	{
		// We have closed the last instance of the file in question.

		int retval = -1;
		if (file->device->driver->close)
		{
			retval = file->device->driver->close(file->device);
		}
		else
		{
			errno = ENOSYS;
			return -1;
		}

		memset(file, 0, sizeof(*file));

		return retval;
	}
	else
	{
		memset(file, 0, sizeof(*file));

		return 0;
	}
}

int _read(int fd, void *buf, size_t len)
{
	struct file_desc *file = find_file(fd);
	if (!file)
		return -1;

	if ((file->flags & FREAD) != FREAD)
	{
		errno = EACCES;
		return -1;
	}

	int retval = -1;
	if (file->device->driver->read)
	{
		retval = file->device->driver->read(file->device, buf, len);
	}
	else
	{
		errno = ENOSYS;
		return -1;
	}

	return retval;
}

int _write(int fd, const void *buf, size_t len)
{
	struct file_desc *file = find_file(fd);
	if (!file)
		return -1;

	if ((file->flags & FWRITE) != FWRITE)
	{
		errno = EACCES;
		return -1;
	}

	int retval = -1;
	if (file->device->driver->write)
	{
		retval = file->device->driver->write(file->device, buf, len);
	}
	else
	{
		errno = ENOSYS;
		return -1;
	}

	return retval;
}

int ioctl(int fd, unsigned long func, ...)
{
	struct file_desc *file = find_file(fd);
	if (!file)
		return -1;

	int retval = -1;
	if (file->device->driver->ioctl)
	{
		va_list args;
		va_start(args, func);
		uint32_t value = va_arg(args, uint32_t);
		va_end(args);

		retval = file->device->driver->ioctl(file->device, func, value);
	}
	else
	{
		errno = ENOSYS;
		return -1;
	}

	return retval;
}

int dup(int fd)
{
	int new_fd = find_next_fd();
	if (fd < 0)
		return -1;

	return dup2(fd, new_fd);
}

int dup2(int fd, int new_fd)
{
	struct file_desc *file = find_file(fd);
	if (!file)
		return -1;

	if (new_fd == fd)
		return new_fd;

	if (new_fd >= LIMIT_HANDLES)
	{
		errno = EMFILE;
		return -1;
	}

	struct file_desc *new_file = &(files[new_fd]);
	if (new_file->magic == FILE_MAGIC)
	{
		close(new_fd);
	}

	*new_file = *file;
	file->device->open_count++;

	return new_fd;
}

int _fstat(int fd, struct stat *st)
{
	struct file_desc *file = find_file(fd);
	if (!file)
		return -1;

	int retval = -1;
	if (file->device->driver->fstat)
	{
		retval = file->device->driver->fstat(file->device, st);
	}
	else
	{
		errno = ENOSYS;
		return -1;
	}

	return retval;
}

off_t _lseek(int fd, off_t offset, int whence)
{
	struct file_desc *file = find_file(fd);
	if (!file)
		return -1;

	int retval = -1;
	if (file->device->driver->lseek)
	{
		retval = file->device->driver->lseek(file->device, offset, whence);
	}
	else
	{
		errno = ENOSYS;
		return -1;
	}

	return retval;
}

int _isatty(int fd)
{
	struct file_desc *file = find_file(fd);
	if (!file)
		return -1;

	int retval = -1;
	if (file->device->driver->isatty)
	{
		retval = file->device->driver->isatty(file->device);
	}
	else
	{
		errno = ENOSYS;
		return 0;
	}

	return retval;
}

int poll(struct pollfd pollfds[], nfds_t numfds, int timeout)
{
	int count = 0;
	int32_t end_time = millis() + timeout;

	do
	{
		for (int idx = 0; idx < numfds; idx++)
		{
			struct pollfd *pfd = &(pollfds[idx]);
			struct file_desc *file = find_file(pfd->fd);

			pfd->revents = 0;
			short evmask = pfd->events | POLLERR | POLLHUP | POLLNVAL;

			if (!file)
				pfd->revents = POLLNVAL;
			else if (file->device->driver->poll)
				pfd->revents |= (file->device->driver->poll(file->device, pfd->events, &(pfd->revents)) < 0) ? POLLERR : 0;

			pfd->revents &= evmask;

			if (pfd->revents)
				count++;
		}
	} while (!count && (end_time - (int32_t)millis() > 0));

	return count;
}
