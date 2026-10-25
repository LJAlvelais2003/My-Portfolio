/*
 * SPI.h
 *
 *  Created on: Nov 15, 2025
 *      Author: alvel
 */

#ifndef INC_SPI_H_
#define INC_SPI_H_

//MY INCLUSIONS
#include "main.h"

//MY DEFINITIONS
#define SPI_PORT (GPIOA)
#define SPI_CP_DS (0x7 << 8) //8 bit data size
#define CS_PIN (GPIO_PIN_0)
#define RS_PIN (GPIO_PIN_1)
#define TX_DONE (1U << 3)
#define RX_DONE (1U << 6)
#define CRC_ERR (1U << 5)
#define PCK_LEN (32)

#endif /* INC_SPI_H_ */

//MY PROTOTYPES
void SPI_Config(void);
void SPI_init(void);
uint8_t SPI_wr_b(uint8_t b);
void CS_low(void);
void CS_high(void);
void RS_low(void);
void RS_high(void);
void SPI_w_nB(uint8_t *data, uint32_t n);
void SPI_rd_nB(uint8_t *data, uint32_t n);
void rfm_wr(uint8_t addr, uint8_t b);
uint8_t rfm_rd(uint8_t addr);
void rfm_wr_nB(uint8_t addr, uint8_t *Wdata, uint32_t len);
void rfm_rd_nB(uint8_t addr, uint8_t *Rdata, uint32_t len);
void rfm_init(void);
void rfm_tx(uint8_t *data, uint32_t size);
void RX_Cont(void);
uint8_t RX_polling(uint8_t *data, uint8_t max_len);
void Pitch_Demo(void);
void Catch_Demo(void);
uint8_t RFM95_Basic_Bringup(void);
void base_node(void);

