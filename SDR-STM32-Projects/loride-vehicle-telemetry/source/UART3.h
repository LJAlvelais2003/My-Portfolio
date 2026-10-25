/*
 ******************************************************************************
 * EE 329 uart3.h
 ******************************************************************************
 * @file           : uart3.h
 * @brief          : uart3 setup
 * project         : EE 329 Custom Project
 * authors         : Anthony di Furia
 * version         : 1
 * date            : 12/1/25
 * compiler        : STM32CubeIDE v.1.19.0
 * target          : NUCLEO-L4A6ZG
 * clocks          : 4 MHz MSI to AHB2
 * @attention      : (c) 2025 STMicroelectronics.  All rights reserved.
 ******************************************************************************
 * REVISION HISTORY
 * 12/1/25   Created file
 ******************************************************************************

 */

#ifndef UART3_H
#define UART3_H

#include <stdint.h>
#include "stm32l4xx.h"   // CMSIS device header, no HAL required

// Initialize USART3 for RX (and optional TX) on PD9 (and PD8) at given baud.
// pclk1_hz must be the APB1 clock frequency (in Hz),
// e.g. 80000000 for 80 MHz.
void UART3_Init(uint32_t pclk1_hz, uint32_t baudrate);

// Blocking read of a single character from USART3.
// Returns the received byte (0–255).
int UART3_ReadChar(void);

// Blocking read of one text line from USART3.
// - buffer: where to store the line (null-terminated)
// - maxLen: size of buffer in bytes
// Returns number of chars stored (not counting '\0').
int UART3_ReadLine(char *buffer, uint16_t maxLen);

#endif // UART3_H
