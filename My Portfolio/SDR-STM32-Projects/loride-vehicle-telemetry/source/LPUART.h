/*-----------------------------------------
 * LPUART.h
 * AUTHORS: Leo Alvelais and Derek Huie
 * CREATED: 10/20/2025
 * ----------------------------------------
 * GPIOG PINS:
 * PG7 - TX
 * PG8 - RX
 */

#ifndef INC_LPUART_H_
#define INC_LPUART_H_

//MY INCLUSIONS
#include "main.h"
#endif /* INC_LPUART_H_ */

//VARIABLES FOR EXPORT
volatile extern uint8_t up;
volatile extern uint8_t down;
volatile extern uint8_t left;
volatile extern uint8_t right;

//MY PROTOTYPES
void LPUART1_IRQHandler(void);
void LPUART1_init(void);
void LPUART_ESC_Print(char message[]);
void LPUART_Print(const char *message);
void Set_Pos(uint8_t X_Pos, uint8_t Y_pos);
void Make_Fireworks(uint8_t x_pos, uint8_t y_pos);
void Wr_to_Pos(char message[], uint8_t X_pos, uint8_t Y_pos);
void CLR_Screen(void);
void LPUART_Wr_Dig(uint16_t Dig_2_Wr);
void u16_2_str(uint16_t value, char *buf);
void LPUART_PrintInt(int value);

//MY DEFINITIONS
#define LPUART_DIV  (8889)
//#define RED_TEXT ()

//PROTOTYPES
void LPUART_Print(const char *message);
void LPUART1_init(void);
void LPUART1_IRQHandler(void);
