/*
 * time.c
 *
 *  Created on: Sep 24, 2017
 *      Author: technix
 */

#include <stm32f1xx.h>
#include <stm32f1xx_it.h>

#include <dreamos-rt/time.h>

#define _COMPILING_NEWLIB
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>
#include <unistd.h>

#include <errno.h>
#undef errno
extern int errno;

/*
 * SysTick-based application run time clock.
 */
uint32_t millis_counter = 0;

__attribute__((constructor)) void systick_init(void)
{
	SysTick_Config(SystemCoreClock / 1000);
	__DSB();
}

__attribute__((section(".datacode"))) void SysTick_IRQHandler(void)
{
	millis_counter++;
}

__attribute__((section(".datacode"), weak)) void yield(void) {}

uint32_t millis(void)
{
	return millis_counter;
}

uint32_t micros(void)
{
	uint32_t ms = millis_counter;
	uint32_t ticks;
	do
		ticks = SysTick->LOAD - SysTick->VAL;
	while (ms != millis_counter);

	return ms * 1000 + (ticks / (SystemCoreClock / 1000000));
}

unsigned sleep(unsigned seconds)
{
	int32_t ms_end = millis_counter + seconds * 1000;

	while (ms_end - (int32_t)millis_counter > 0)
		yield();

	return 0;
}

int usleep(useconds_t useconds)
{
	uint32_t ms = millis_counter;
	uint32_t ticks;
	do
		ticks = SysTick->VAL;
	while (ms != millis_counter && ticks > 20);
	ms = millis_counter;

	int32_t ms_end = ms + (useconds / 1000);
	int32_t ticks_end = ticks - (useconds % 1000) * (SystemCoreClock / 1000000);
	while (ticks_end < 0)
	{
		ms_end++;
		ticks_end += SysTick->LOAD;
	}

	while (ms_end - (int32_t)millis_counter > 0)
		yield();
	while ((int32_t)SysTick->VAL - ticks_end > 0)
		yield();

	return 0;
}

/*
 * Real time clock
 */

#ifndef LSE_VALUE
#define LSE_VALUE 32768
#endif

#define BKP_TZ  BKP->DR1
#define BKP_DST BKP->DR2

char *strptime(const char *restrict buf, const char *restrict format, struct tm *restrict timeptr);
void yield(void);

__attribute__((constructor)) void rtc_init(void)
{
	// If we don't have a clock ready, set it to the compile time of this file.
	if (RTC->CRL & RTC_CRL_CNF)
	{
		struct tm tm = {0};
		struct timeval timeval = {0};
		strptime(__DATE__ " " __TIME__, "%b %d %Y %H:%M:%S", &tm);
		timeval.tv_sec = mktime(&tm);
		settimeofday(&timeval, NULL);
	}
}

clock_t _times(struct tms *tm)
{

	clock_t ticks = millis() / (1000 / CLK_TCK);

	if (tm)
	{
		tm->tms_utime = ticks;
		tm->tms_stime = 0;
		tm->tms_cutime = 0;
		tm->tms_cstime = 0;
	}

	return ticks;
}

int _gettimeofday(struct timeval *tm, void *tz)
{
	if (RTC->CRL & RTC_CRL_CNF)
	{
		errno = ENOTSUP;
		return -1;
	}

	if (tm)
	{
		tm->tv_sec = ((uint32_t)RTC->CNTH) << 16 | ((uint32_t)RTC->CNTL);
		tm->tv_usec = (LSE_VALUE - RTC->DIVL - 1) * 1000000 / (LSE_VALUE);
	}

	if (tz)
	{
		struct timezone *z = tz;
		z->tz_minuteswest = BKP_TZ;
		z->tz_dsttime = BKP_DST;
	}

	return 0;
}

int settimeofday(const struct timeval *tm, const struct timezone *tz)
{
	if (tm)
	{
		RTC->CRL |= RTC_CRL_CNF;
		while (!(RTC->CRL & RTC_CRL_RTOFF))
			yield();

		RTC->PRLH = 0;
		RTC->PRLL = LSE_VALUE - 1;

		RTC->CNTH = tm->tv_sec >> 16;
		RTC->CNTL = tm->tv_sec & 0xffff;

		RTC->CRL &= ~RTC_CRL_CNF;
		while (!(RTC->CRL & RTC_CRL_RTOFF))
			yield();
	}

	if (tz)
	{
		BKP_TZ = tz->tz_minuteswest;
		BKP_DST = tz->tz_dsttime;
	}

	return 0;
}
