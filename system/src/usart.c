/*
 * usart.c
 *
 *  Created on: Aug 26, 2017
 *      Author: technix
 */

#include <dreamos-rt/usart.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <dreamos-rt/ring-buffer.h>
#include <sys/fcntl.h>
#include <sys/poll.h>
#include <dreamos-rt/device.h>
#include <dreamos-rt/time.h>
#include <stm32f1xx_it.h>
#include <stm32f1xx.h>

#include <errno.h>
#undef errno
extern int errno;

typedef struct usart_s usart_t;

struct usart_s
{
	// Static data.
	device_t device;
	USART_TypeDef *const USART;
	IRQn_Type IRQn;
	volatile uint32_t *const RCC_APBEN;
	const uint32_t RCC_APBEN_Msk;
	uint8_t RCC_CKDIV;

	// Dynamic data
	int open_mode;

	ring_buffer_t read_buffer;
	ring_buffer_t write_buffer;
};

__attribute__((section(".datacode"))) static void usart_interrupt(usart_t *usart)
{
	if ((usart->USART->CR1 & USART_CR1_RXNEIE) && (usart->USART->SR & USART_SR_RXNE))
	{
		// We have an incoming byte.

		ring_buffer_putchar(usart->read_buffer, usart->USART->DR);
	}

	if ((usart->USART->CR1 & USART_CR1_TXEIE) && (usart->USART->SR & USART_SR_TXE))
	{
		// We have space for an outgoing byte.
		int ch = ring_buffer_getchar(usart->write_buffer);
		if (ch < 0)
		{
			// We ran out of bytes to send...
			usart->USART->CR1 &= ~USART_CR1_TXEIE;
		}
		else
		{
			usart->USART->DR = ch;
		}
	}
}

static inline void usart_wait_for_write(usart_t *usart)
{
	while (ring_buffer_getlength(usart->write_buffer));
	while (!(usart->USART->SR & USART_SR_TC));
}

static int usart_open(device_t *device, int mode, ...)
{
	usart_t *usart = (usart_t *)device;

	if (!usart->read_buffer)
	{
		usart->read_buffer = ring_buffer_init(USART_READ_BUFFER_SIZE);
		if (!usart->read_buffer)
			return -1;
	}

	if (!usart->write_buffer)
	{
		usart->write_buffer = ring_buffer_init(USART_WRITE_BUFFER_SIZE);
		if (!usart->write_buffer)
		{
			ring_buffer_dealloc(usart->read_buffer);
			usart->read_buffer = NULL;
			return -1;
		}
	}

	(*usart->RCC_APBEN) |= usart->RCC_APBEN_Msk;

	SET_FIELD(usart->USART->CR1, USART_CR1_M, 0);					// 8-N-1
	SET_FIELD(usart->USART->CR2, USART_CR2_STOP, 0);
	usart->USART->BRR = SystemCoreClock / usart->RCC_CKDIV / USART_DEFAULT_BAUDRATE;	// 115200 baud

	// Enable interrupts
	NVIC_EnableIRQ(usart->IRQn);

	usart->USART->CR1 |= USART_CR1_UE;
	__DSB();
	usart->USART->CR1 |= USART_CR1_TE | USART_CR1_RE;
	__DSB();
	usart->USART->CR1 |= USART_CR1_RXNEIE;
	__DSB();

	usart->open_mode = mode;

	return 0;
}

static int usart_close(device_t *device)
{
	usart_t *usart = (usart_t *)device;

	// Gracefully shut down USART
	usart->USART->CR1 &= ~USART_CR1_RE;
	usart_wait_for_write(usart);
	usart->USART->CR1 &= ~USART_CR1_TE;
	usart->USART->CR1 &= ~USART_CR1_UE;

	// Disable interrupts.
	NVIC_DisableIRQ(usart->IRQn);

	(*usart->RCC_APBEN) &= ~usart->RCC_APBEN_Msk;

	ring_buffer_dealloc(usart->read_buffer);
	ring_buffer_dealloc(usart->write_buffer);
	usart->read_buffer = NULL;
	usart->write_buffer = NULL;

	return 0;
}

static inline int usart_getchar(usart_t *usart)
{
	while (!ring_buffer_getlength(usart->read_buffer))
		yield();
	int ch = ring_buffer_getchar(usart->read_buffer);

	if (ch < 0)
		return -EAGAIN;
	else
		return ch;
}

static inline int usart_putchar(usart_t *usart, char ch)
{
	if (usart->USART->SR & USART_SR_TXE)
	{
		usart->USART->DR = ch;
	}
	else
	{
		while (!ring_buffer_getspace(usart->write_buffer))
			yield();
		ring_buffer_putchar(usart->write_buffer, ch);
		usart->USART->CR1 |= USART_CR1_TXEIE;
	}

	return 0;
}

static int usart_read(device_t *device, void *buf, size_t len)
{
	usart_t *usart = (usart_t *)device;
	char *bp = buf;
	int count = 0;

	for (int idx = 0; idx < len; idx++)
	{
		int ch = usart_getchar(usart);
		if (ch < 0)
		{
			errno = -ch;
			break;
		}
		bp[idx] = ch;
		count++;
	}

	return count;
}

static int usart_write(device_t *device, const void *buf, size_t len)
{
	usart_t *usart = (usart_t *)device;
	const char *bp = buf;
	int count = 0;

	for (int idx = 0; idx < len; idx++)
	{
		char ch = bp[idx];
		int r = usart_putchar(usart, ch);
		if (r < 0)
		{
			errno = -r;
			break;
		}
		count++;
	}

	return count;
}

static int usart_ioctl(device_t *device, unsigned long func, ...)
{
	usart_t *usart = (usart_t *)device;

	uint32_t value;
	va_list args;
	va_start(args, func);
	value = va_arg(args, uint32_t);
	va_end(args);

	switch (func)
	{
	case IOCTL_USART_GET_BAUDRATE:
		return SystemCoreClock / usart->USART->BRR;

	case IOCTL_USART_SET_BAUDRATE:
		usart->USART->CR1 &= ~USART_CR1_RE;
		usart_wait_for_write(usart);
		usart->USART->CR1 &= ~USART_CR1_TE;
		usart->USART->CR1 &= ~USART_CR1_UE;
		__DMB();

		usart->USART->BRR = SystemCoreClock / value;
		__DMB();

		usart->USART->CR1 |= USART_CR1_UE;
		usart->USART->CR1 |= USART_CR1_TE | USART_CR1_RE;

		return 0;

	default:
		errno = EINVAL;
		return -1;
	}
}

static int usart_poll(device_t *device, short events, short *revents)
{
	usart_t *usart = (usart_t *)device;
	return -1;
}

static int usart_fstat(device_t *device, struct stat *st)
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

static int usart_isatty(device_t *device)
{
	return 1;
}

static const driver_t usart_driver =
{
		.name = "stm32f3xx-usart",
		.open = usart_open,
		.close = usart_close,
		.read = usart_read,
		.write = usart_write,
		.ioctl = usart_ioctl,
		.poll = usart_poll,
		.fstat = usart_fstat,
		.isatty = usart_isatty
};

static usart_t tty0 =
{
		{
				&usart_driver,
				"tty0",
		},
		USART1,
		USART1_IRQn,
		&(RCC->APB2ENR),
		RCC_APB2ENR_USART1EN,
		1
};
__attribute__((section(".devices"))) const usart_t *TTY0 = &tty0;
__attribute__((section(".datacode"))) void USART1_IRQHandler(void) { usart_interrupt(&tty0); }

static usart_t tty1 =
{
		{
				&usart_driver,
				"tty1",
		},
		USART2,
		USART2_IRQn,
		&(RCC->APB1ENR),
		RCC_APB1ENR_USART2EN,
		2
};
__attribute__((section(".devices"))) const usart_t *TTY1 = &tty1;
__attribute__((section(".datacode"))) void USART2_IRQHandler(void) { usart_interrupt(&tty1); }

static usart_t tty2 =
{
		{
				&usart_driver,
				"tty2",
		},
		USART3,
		USART3_IRQn,
		&(RCC->APB1ENR),
		RCC_APB1ENR_USART3EN,
		2
};
__attribute__((section(".devices"))) const usart_t *TTY2 = &tty2;
__attribute__((section(".datacode"))) void USART3_IRQHandler(void) { usart_interrupt(&tty2); }

/*
static usart_t tty3 =
{
		{
				&usart_driver,
				"tty3",
		},
		UART4,
		UART4_IRQn,
		&(RCC->APB1ENR),
		RCC_APB1ENR_UART4EN,
		2
};
__attribute__((section(".devices"))) const usart_t *TTY3 = &tty3;
__attribute__((section(".datacode"))) void UART4_IRQHandler(void) { usart_interrupt(&tty3); }

static usart_t tty4 =
{
		{
				&usart_driver,
				"tty4",
		},
		UART5,
		UART5_IRQn,
		&(RCC->APB1ENR),
		RCC_APB1ENR_UART5EN,
		2
};
__attribute__((section(".devices"))) const usart_t *TTY4 = &tty4;
__attribute__((section(".datacode"))) void UART5_IRQHandler(void) { usart_interrupt(&tty4); }
*/
