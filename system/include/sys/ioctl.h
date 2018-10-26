/*
 * ioctl.h
 *
 *  Created on: Aug 26, 2017
 *      Author: technix
 */

#ifndef SYSTEM_INCLUDE_SYS_IOCTL_H_
#define SYSTEM_INCLUDE_SYS_IOCTL_H_

#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

int ioctl(int, unsigned long, ...);

__END_DECLS

#endif /* SYSTEM_INCLUDE_SYS_IOCTL_H_ */
