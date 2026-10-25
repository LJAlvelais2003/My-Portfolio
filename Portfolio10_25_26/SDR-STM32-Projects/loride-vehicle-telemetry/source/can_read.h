/*
 ******************************************************************************
 * EE 329 can_read.h
 ******************************************************************************
 * @file           : can_read.h
 * @brief          : can_read setup
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
#include "main.h"

#ifndef INC_CAN_READ_H_
#define INC_CAN_READ_H_

#define MAX_TOKENS 10     // ID + LEN + 8 data bytes
#define TOKEN_LEN  4      // up to "7E8" + '\0'
#define BYTE_COUNT 10     // size of data stream

typedef struct {
   uint8_t hi;  // high byte
   uint8_t lo;  // low byte
} TwoBytes;

extern uint8_t stream_bytes[BYTE_COUNT];
extern char stream_line[BYTE_COUNT * 3 + 3];

#endif /* INC_CAN_READ_H_ */

static uint8_t hex_val(char c);
static uint16_t hex3_to_id(const char in[4]) ;
static uint8_t hex2_to_byte(const char in[3]);
void split_hex_line(const char line[]);
void tokens_to_hex_values(void);
void can_parse_line(const char line[]);
int can_read_engine_rpm(void);
int can_read_wheel_speed(void);
int can_read_coolant_temp(void);
int can_read_maf(void);
int can_read_intake(void);
TwoBytes int_to_two_bytes(int value);
void can_build_data_stream(int rpm, int speed, int coolant, int maf, int intake);
void can_build_stream_line_from_bytes(void);




