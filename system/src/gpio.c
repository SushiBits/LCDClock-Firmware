/*
 * gpio.c
 *
 *  Created on: Sep 17, 2017
 *      Author: technix
 */

#include <dreamos-rt/gpio.h>
#include <stm32f1xx.h>

__attribute__((constructor(1000))) void GPIO_Initialize(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN | RCC_APB2ENR_IOPDEN;
	__DSB();
}

static inline GPIO_TypeDef *GPIO_GetPin(uint8_t pin, uint8_t *pin_id)
{
	static GPIO_TypeDef *const GPIO[] =
	{
			GPIOA,
			GPIOB,
			GPIOC,
			GPIOD
	};

	uint8_t gpio = (pin & 0xf0) >> 4;
	if (gpio > 5)
		return NULL;

	if (pin_id)
		*pin_id = pin & 0x0f;
	return GPIO[gpio];
}

uint16_t getPinMode(uint8_t pin)
{
	uint8_t pinid = 0;
	GPIO_TypeDef *GPIO = GPIO_GetPin(pin, &pinid);
	if (!GPIO)
		return 0;

	uint32_t CR = (pinid & 0x8) ? (GPIO->CRH) : (GPIO->CRL);
	return CR >> ((pinid & 0x7) * 4) & 0b1111;
}

void pinMode(uint8_t pin, uint16_t mode)
{
	uint8_t pinid = 0;
	GPIO_TypeDef *GPIO = GPIO_GetPin(pin, &pinid);
	if (!GPIO)
		return;

	if (mode == INPUT_PULLUP)
		GPIO->ODR |= 1 << pinid;

	volatile uint32_t *CR = (pinid & 0x8) ? &(GPIO->CRH) : &(GPIO->CRL);
	SET_FIELD(*CR, 0b1111 << ((pinid & 0x7) * 4), mode << ((pinid & 0x7) * 4));
	__DSB();
}

void digitalWrite(uint8_t pin, bool value)
{
	uint8_t pinid = 0;
	GPIO_TypeDef *GPIO = GPIO_GetPin(pin, &pinid);
	if (!GPIO)
		return;

	SET_FIELD(GPIO->ODR, 0x1 << pinid, (value ? 0x1 : 0x0) << pinid);
	__DSB();
}

bool digitalRead(uint8_t pin)
{
	uint8_t pinid = 0;
	GPIO_TypeDef *GPIO = GPIO_GetPin(pin, &pinid);
	if (!GPIO)
		return false;

	return ((GPIO->IDR >> pinid) & 0x1) ? true : false;
}
