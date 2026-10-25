/*-----------------------------------------
 * LPUART.c
 * AUTHORS: Leo Alvelais
 * CREATED: 10/20/2025
 * ----------------------------------------
 */
//INCLUSTIONS
#include <LPUART.h>
#include "main.h"
#include "delay.h"
#include <stdio.h>

//FUNCTIONS

/*
 * Function: LPUART_Print
 * --------------------
 * Prints a character array to the terminal emulator via LPUART
 *
 *  message: character array to be written
 */

void LPUART_Print(const char *message) {
   uint16_t iStrIdx = 0;
   while (message[iStrIdx] != 0) {
      while (!(LPUART1->ISR & USART_ISR_TXE))
         // wait for empty xmit buffer
         ;
      LPUART1->TDR = message[iStrIdx];       // send this character
      iStrIdx++;                             // advance index to next char
   }
}

/*
 * Function: LPUART_ESC_Print
 * --------------------
 * Writes escape command
 *
 *  message: escape command to be issued
 *
 */
void LPUART_ESC_Print(char message[]) {
   uint8_t iStrIdx = 0;
   //SEND ESCAPE CODE FIRST
   while (!(LPUART1->ISR & USART_ISR_TXE)) {
   }
   LPUART1->TDR = 0x1B;

   //WRITE THE REST OF THE MESSAGE CODE TO TDR
   while (message[iStrIdx] != 0) {
      while (!(LPUART1->ISR & USART_ISR_TXE))
         // wait for empty xmit buffer
         ;
      LPUART1->TDR = message[iStrIdx];       // send this character
      iStrIdx++;                             // advance index to next char
   }
}

/*
 * Function: LPUART_init
 * --------------------
 * Initializes low power UART
 * 115200 baud rate
 * 8-bit data rate.
 *
 *Configures GPIOG alternate function registers for proper UART
 *Configuration.
 *
 */

void LPUART1_init(void) {
   RCC->CCIPR &= ~(RCC_CCIPR_LPUART1SEL_Msk);
   RCC->CCIPR |= (RCC_CCIPR_LPUART1SEL_0);

   PWR->CR2 |= (PWR_CR2_IOSV);              // power avail on PG[15:2] (LPUART1)
   RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOGEN);   // enable GPIOG clock
   RCC->APB1ENR2 |= RCC_APB1ENR2_LPUART1EN; // enable LPUART clock bridge

   /* USER: configure GPIOG registers MODER/PUPDR/OTYPER/OSPEEDR then
    select AF mode and specify which function with AFR[0] and AFR[1] */

   //SET GPIOG7 AND 8 TO ALTERNATE FUNCTION MODE
   GPIOG->MODER &= ~(GPIO_MODER_MODE7 |
   GPIO_MODER_MODE8);

   GPIOG->MODER |= (GPIO_MODER_MODE7_1 |
   GPIO_MODER_MODE8_1);

   //PUSH PULL TYPE ON TX AND RX PORTS

   GPIOG->OTYPER &= ~(GPIO_OTYPER_OT7 |
   GPIO_OTYPER_OT8);

   // setting alternate function for G7 and G8
   GPIOG->AFR[0] &= ~(0x000F << GPIO_AFRL_AFSEL7_Pos); // clear PG7 nibble AF
   GPIOG->AFR[0] |= (0x0008 << GPIO_AFRL_AFSEL7_Pos); // set PG7 AF = LPUART1_TX
   GPIOG->AFR[1] &= ~(0x000F << GPIO_AFRH_AFSEL8_Pos); // clear PG8 nibble AF
   GPIOG->AFR[1] |= (0x0008 << GPIO_AFRH_AFSEL8_Pos); // set PG8 AF = LPUART1_RX

   LPUART1->CR1 &= ~(USART_CR1_M1 | USART_CR1_M0); // 8-bit data
   LPUART1->CR1 |= USART_CR1_UE;                   // enable LPUART1
   LPUART1->CR1 |= (USART_CR1_TE | USART_CR1_RE);  // enable xmit & recv
   LPUART1->CR1 |= USART_CR1_RXNEIE;        // enable LPUART1 recv interrupt
   LPUART1->ISR &= ~(USART_ISR_RXNE);       // clear Recv-Not-Empty flag
   /* USER: set baud rate register (LPUART1->BRR) */
   NVIC->ISER[2] = (1 << (LPUART1_IRQn & 0x1F));   // enable LPUART1 ISR
   //SETTING THE BAUD RATE
   LPUART1->BRR = LPUART_DIV;
   __enable_irq();                          // enable global interrupts

   //SETTING THE BAUD RATE
   LPUART1->BRR = LPUART_DIV;

}

/*
 * Function: LPUART_init
 * --------------------
 * Sets cursor to specific position in terminal emulator
 *
 * X_pos: x coordinate of cursor placement
 * Y_pos: y coordinate of cursor placement
 *
 *
 * X_pos or Y_pos can be no greater than 999
 *
 */

void Set_Pos(uint8_t X_Pos, uint8_t Y_Pos) {
   uint8_t Dig[3] = { 0, 0, 0 };
   uint8_t Dig_Idx = 0;

   Dig[0] = Y_Pos / 100;
   Dig[1] = (Y_Pos / 10) % 10;
   Dig[2] = Y_Pos % 10;

   //FOR SETTING Y POSITION
   LPUART_ESC_Print("[");
   //SEND DESIRED LINE TO IDR
   while (Dig_Idx < 3) {
      while (!(LPUART1->ISR & USART_ISR_TXE))
         ;
      LPUART1->TDR = (0x30 | Dig[Dig_Idx]);
      Dig_Idx++;
   }
   LPUART_Print(";");

   //SEND DESIRED COLUMN TO IDR
   Dig_Idx = 0;
   Dig[0] = X_Pos / 100;
   Dig[1] = (X_Pos / 10) % 10;
   Dig[2] = X_Pos % 10;

   while (Dig_Idx < 3) {
      while (!(LPUART1->ISR & USART_ISR_TXE))
         ;
      LPUART1->TDR = (0x30 | Dig[Dig_Idx]);
      Dig_Idx++;
   }
   LPUART_Print("H");
}

/*
 * Function: CLR_Screen
 * --------------------
 * Clears the terminal emulator screen and places the cursor
 * in the default position(top left corner).
 *
 */

void CLR_Screen(void) {
   LPUART_ESC_Print("[2J");
   LPUART_ESC_Print("[H");
}

/*
 * Function: LPUART_Wr_Dig
 * --------------------
 * Writes a 3 digit number to the UART terminal
 *
 * Dig_2_Wr: The digit to write to the terminal.
 *
 */
void LPUART_Wr_Dig(uint16_t Dig_2_Wr) {
   uint8_t Dig_Idx = 0;
   uint16_t Dig_Array[4] = { 0, 0, 0, 0 };
   Dig_Array[0] = (Dig_2_Wr / 1000);
   Dig_Array[1] = (Dig_2_Wr / 100) % 10;
   Dig_Array[2] = (Dig_2_Wr / 10) % 10;
   Dig_Array[3] = Dig_2_Wr % 10;

   while (Dig_Idx < 4) {

      while (!(LPUART1->ISR & USART_ISR_TXE)) {
      } // wait for empty xmit buffer
      LPUART1->TDR = (0X30 | Dig_Array[Dig_Idx]);
      Dig_Idx++;
   }
}

/*----------------------------------------------------------------------------
 * Function: LPUART_PrintInt
 * Author : Anthony di Furia
 * Date : 12/1/25
 * Purpose : prints integer to LPUART terminal
 * Inputs : int
 * Returns : None
 *--------------------------------------------------------------------------*/

void LPUART_PrintInt(int value) {
   char buf[32];
// convert int to string
   snprintf(buf, sizeof(buf), "%d", value);
// now send as string
   LPUART_Print(buf);
}

/*
 * Function: u16_2_str
 * --------------------
 * converts a 16 bit number to a string to print for printing numbers
 *  to the UART terminal.
 *
 * value: unsigned 16 bit number to be converted to string
 * buf: character buffer to store characters to print.
 *
 */
void u16_2_str(uint16_t value, char *buf) {
   char tmp[6];
   int pos = 0;

   if (value == 0) {
      buf[0] = '0';
      buf[1] = '\0';
      return;
   }

   while (value > 0) {
      tmp[pos++] = (value % 10) + '0';
      value /= 10;
   }

   for (int i = 0; i < pos; i++) {
      buf[i] = tmp[pos - 1 - i];
   }

   buf[pos] = '\0';
}

