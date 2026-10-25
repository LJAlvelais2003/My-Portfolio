/*
 * delay.h
 *
 *  Created on: Oct 5, 2025
 *      Author: Leo and Anthony
 */
#include <stdint.h>

#ifndef INC_DELAY_H_
#define INC_DELAY_H_

//MY PROTOTYPES
void delay_us(uint32_t time_us);
void delay_ms(const uint32_t time_ms);
void SysTick_Init(void);

#endif /* INC_DELAY_H_ */

