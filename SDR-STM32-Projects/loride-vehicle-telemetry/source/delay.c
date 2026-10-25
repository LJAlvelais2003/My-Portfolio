/*
 * delay.c
 *
 *  Created on: Oct 5, 2025
 *      Author: Leo
 */

#include "delay.h"
#include "main.h"

// --------------------------------------------------- delay.c w/o #includes ---
// configure SysTick timer for use with delay_us().
// warning: breaks HAL_delay() by disabling interrupts for shorter delay timing.
void SysTick_Init(void) {
   SysTick->CTRL |= (SysTick_CTRL_ENABLE_Msk |        // enable SysTick Timer
         SysTick_CTRL_CLKSOURCE_Msk);    // select CPU clock
   SysTick->CTRL &= ~(SysTick_CTRL_TICKINT_Msk);   // disable interrupt
}

// delay in microseconds using SysTick timer to count CPU clock cycles
// do not call with 0 : error, maximum delay.
// careful calling with small nums : results in longer delays than specified:
//    e.g. @4MHz, delay_us(1) = 10-15 us delay.
void delay_us(const uint32_t time_us) {
   // set the counts for the specified delay
   SysTick->LOAD = (uint32_t) ((time_us * (SystemCoreClock / 1000000)) - 1);
   SysTick->VAL = 0;                                      // clear timer count
   SysTick->CTRL &= ~(SysTick_CTRL_COUNTFLAG_Msk);        // clear count flag
   while (!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk))
      ; // wait for flag
}

void delay_ms(const uint32_t time_ms) {
   delay_us(time_ms * 1000);
}
