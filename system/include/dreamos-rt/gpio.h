/*
 * gpio.h
 *
 *  Created on: Sep 17, 2017
 *      Author: technix
 */

#ifndef SYSTEM_INCLUDE_DREAMOS_RT_GPIO_H_
#define SYSTEM_INCLUDE_DREAMOS_RT_GPIO_H_

#include <stdint.h>
#include <stdbool.h>
#include <sys/cdefs.h>

#define INPUT				0b0100
#define ANALOG				0b0000
#define INPUT_PULLUP			0b1000
#define OUTPUT				0b0011
#define ALT(afio)			0b1011
#define ALT_OD(afio)			0b1111
#define ALT_PU(afio)			ALT(afio)
#define ALT_OD_PU(afio)		ALT_OD(afio)

__BEGIN_DECLS

uint16_t getPinMode(uint8_t pin);
void pinMode(uint8_t pin, uint16_t mode);
void digitalWrite(uint8_t pin, bool value);
bool digitalRead(uint8_t pin);

__END_DECLS

#endif /* SYSTEM_INCLUDE_DREAMOS_RT_GPIO_H_ */
