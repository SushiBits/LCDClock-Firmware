/*
 * poll.h
 *
 *  Created on: Aug 26, 2017
 *      Author: technix
 */

#ifndef SYSTEM_INCLUDE_SYS_POLL_H_
#define SYSTEM_INCLUDE_SYS_POLL_H_

#include <stdint.h>
#include <sys/types.h>
#include <sys/cdefs.h>

struct pollfd
{
	int fd;
	short events;
	short revents;
};

typedef uint32_t nfds_t;

enum
{
	POLLIN		= 0x0001,
	POLLPRI		= 0x0002,
	POLLOUT		= 0x0004,
	POLLERR		= 0x0008,
	POLLHUP		= 0x0010,
	POLLNVAL		= 0x0020
};

__BEGIN_DECLS
int poll(struct pollfd pollfds[], nfds_t numfds, int timeout);
__END_DECLS

#endif /* SYSTEM_INCLUDE_SYS_POLL_H_ */
