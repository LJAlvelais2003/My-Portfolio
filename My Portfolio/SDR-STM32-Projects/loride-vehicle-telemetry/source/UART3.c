/*
 ******************************************************************************
 * @file           : uart3.c
 * @brief          : contains the code to enable use of the UART3 on the STM32.
 * project         : EE 329 Custom Project
 * authors         : Anthony di Furia
 * version         : 1
 * date            : 12/1/25
 * compiler        : STM32CubeIDE v.1.19.0
 * target          : NUCLEO-L4A6ZG
 * clocks          : 4 MHz MSI to AHB2
 * @attention      : (c) 2025 STMicroelectronics.  All rights reserved.
 *******************************************************************************
 * UART3 COMMUNICATION PLAN :
 * Configure USART3 on PD9 (RX) for receiving serial data from Arduino
 * 8 data bits, no parity, 1 stop bit, oversampling by 16
 * Receiver enabled, optional transmitter on PD8
 * Blocking read functions with automatic error handling
 *******************************************************************************
 * UART3 WIRING (pinout NUCLEO-L4A6ZG)
 * Arduino                                             L4A6ZG
 *
 *PD1 TX -- 10k resistor --------- node ------------PD9 (USART3_RX / D0)
 *                                  |
 *                               20k resistor
 *                                  |
 *                                 GND
 * Common GND between Arduino and Nucleo required
 * Voltage divider: 5V Arduino signal -> 3.3V STM32 logic level
 * Optional: PD8 (USART3_TX / D1) for transmit capability
 *******************************************************************************
 * SPI between Arduino Uno and Can bus shield
 *
 * Shield                                  Arduino
 * JP5 - 5V  ---------------------------- POWER - 5
 * JP5 - 3V3 ---------------------------- POWER - 4
 * JP5 - GND ---------------------------- POWER - 6
 * JP1 - 13  ---------------------------- IOH - 6
 * JP1 - 12  ---------------------------- IOH - 5
 * JP1 - 11  ---------------------------- IOH - 4
 * JP1 - 9   ---------------------------- IOH - 2
 *******************************************************************************
 * REVISION HISTORY
 * 12/1/25   Created file
 ******************************************************************************
 */

#include "UART3.h"
#include <stdio.h>      // for snprintf in LPUART_PrintInt

/*----------------------------------------------------------------------------
 * Function: UART3_init
 * Author  : Anthony di Furia
 * Date    : 12/3/25
 * Purpose : initialize the UART3, given a certain clock speed and baud rate.
 * Inputs  : clock speed, baud rate
 * Returns : None
 *--------------------------------------------------------------------------*/

void UART3_Init(uint32_t pclk1_hz, uint32_t baudrate) {
   // 1) Enable clocks for GPIOD and USART3

   // Enable GPIOD clock on AHB2
   RCC->AHB2ENR |= RCC_AHB2ENR_GPIODEN;

   // Enable USART3 clock on APB1
   RCC->APB1ENR1 |= RCC_APB1ENR1_USART3EN;   // USART3EN bit

   // 2) Configure PD9 as AF7 (USART3_RX)
   // PD9: MODER9[1:0] = 10 (alternate function)
   GPIOD->MODER &= ~(0x3U << (9U * 2U));
   GPIOD->MODER |= (0x2U << (9U * 2U));

   // Output type: push-pull
   GPIOD->OTYPER &= ~(1U << 9);

   // Speed: high speed
   GPIOD->OSPEEDR &= ~(0x3U << (9U * 2U));
   GPIOD->OSPEEDR |= (0x3U << (9U * 2U));

   // Pull-up on RX (01 = pull-up)
   GPIOD->PUPDR &= ~(0x3U << (9U * 2U));
   GPIOD->PUPDR |= (0x1U << (9U * 2U));

   // Alternate function AF7 on PD9:
   // AFRH index for pin 9 is (9-8)*4 = 4
   GPIOD->AFR[1] &= ~(0xFU << ((9U - 8U) * 4U));
   GPIOD->AFR[1] |= (0x7U << ((9U - 8U) * 4U));  // AF7: USART3_RX

   // 3) Configure USART3

   // Disable before config
   USART3->CR1 &= ~USART_CR1_UE;

   // 8 data bits, no parity, oversampling by 16
   USART3->CR1 = 0;
   USART3->CR2 = 0;
   USART3->CR3 = 0;

   // 4) Baud rate: BRR = PCLK1 / baudrate
   uint32_t usartdiv = (pclk1_hz + (baudrate / 2U)) / baudrate;
   USART3->BRR = usartdiv;

   // Enable receiver (and TX if needed)
   USART3->CR1 |= USART_CR1_RE;
   // USART3->CR1 |= USART_CR1_TE;  // enable if you wire TX

   // Enable USART3
   USART3->CR1 |= USART_CR1_UE;

}

/*----------------------------------------------------------------------------
 * Function: UART3_ReadChar
 * Author  : Anthony di Furia
 * Date    : 12/3/25
 * Purpose : reads character from the UART3 Rx pin
 * Inputs  : None
 * Returns : integer
 *--------------------------------------------------------------------------*/

int UART3_ReadChar(void) {
   while (1) {
      uint32_t isr = USART3->ISR;

      // Any error? (Overrun / Framing / Noise)
      if (isr & (USART_ISR_ORE | USART_ISR_FE | USART_ISR_NE)) {
         // Read RDR to clear RXNE / data
         volatile uint32_t tmp = USART3->RDR;
         (void) tmp;

         // Clear the error flags in ICR
         USART3->ICR = USART_ICR_ORECF | USART_ICR_FECF | USART_ICR_NCF;
         // then continue the loop and wait for a clean byte
         continue;
      }

      // Normal receive: RXNE set
      if (isr & USART_ISR_RXNE) {
         return (int) (USART3->RDR & 0xFF);
      }

      // else: no data yet, spin
   }
}

/*----------------------------------------------------------------------------
 * Function: UART3_ReadLine
 * Author  : Anthony di Furia
 * Date    : 12/3/25
 * Purpose : reads entire line received via the UART3 Rx pin
 * Inputs  : None
 * Returns : integer
 *--------------------------------------------------------------------------*/

int UART3_ReadLine(char *buffer, uint16_t maxLen) {
   if (buffer == NULL || maxLen < 2) {
      return 0;
   }

   uint16_t idx = 0;

   while (1) {
      int c = UART3_ReadChar();   // blocking

      if (c == '\n') {
         // Strip trailing '\r' if present (Arduino sends "\r\n")
         if (idx > 0 && buffer[idx - 1] == '\r') {
            idx--;
         }
         buffer[idx] = '\0';
         return (int) idx;
      }

      if (idx < (maxLen - 1)) {
         buffer[idx++] = (char) c;
      } else {
         // Buffer full, terminate and return what's there
         buffer[maxLen - 1] = '\0';
         return (int) (maxLen - 1);
      }
   }
}
