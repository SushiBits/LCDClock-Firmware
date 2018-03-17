/*
 * rcc.c
 *
 *  Created on: Mar 17, 2018
 *      Author: technix
 */

#include "common.h"

void SystemInit(void)
{
	// Turn on HSE oscillator.
	RCC->CR |= RCC_CR_HSEON;
	wait(RCC->CR & RCC_CR_HSERDY);

	// Configure and turn on PLL.
	RCC->CFGR = RCC_CFGR_PLLMULL9 | RCC_CFGR_PLLSRC | RCC_CFGR_ADCPRE_DIV6 | RCC_CFGR_PPRE1_DIV2;
	RCC->CR |= RCC_CR_PLLON;
	wait(RCC->CR & RCC_CR_PLLRDY);

	// Slow down Flash access
	FLASH->ACR = FLASH_ACR_PRFTBE | (2 << FLASH_ACR_LATENCY_Pos);

	// Move to PLL clocks
	RCC->CFGR |= RCC_CFGR_SW_PLL;
	wait((RCC->CFGR & RCC_CFGR_SWS) == RCC_CFGR_SWS_PLL);
}
