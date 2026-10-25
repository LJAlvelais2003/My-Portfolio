/*
 ******************************************************************************
 * @file           : can_read.c
 * @brief          : contains functions that break down and process the CAN BUS
 *              line into usable values.
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
#include "can_read.h"

char g_tokens[MAX_TOKENS][TOKEN_LEN];
uint16_t g_values[MAX_TOKENS];
uint8_t g_token_count = 0;

uint8_t stream_bytes[BYTE_COUNT];
char stream_line[BYTE_COUNT * 3 + 3];

/*----------------------------------------------------------------------------
 * Function: hex_val
 * Author  : Anthony di Furia
 * Date    : 12/3/25
 * Purpose : takes a char hex val and converts that char to the int it
 *        represents
 *
 * Inputs  : char
 * Returns : int
 *--------------------------------------------------------------------------*/

static uint8_t hex_val(char c) {
   if (c >= '0' && c <= '9')
      return (uint8_t) (c - '0');
   if (c >= 'A' && c <= 'F')
      return (uint8_t) (10 + c - 'A');
   if (c >= 'a' && c <= 'f')
      return (uint8_t) (10 + c - 'a');
   return 0xFF; // invalid
}

/*----------------------------------------------------------------------------
 * Function: hex3_to_id
 * Author  : Anthony di Furia
 * Date    : 12/3/25
 * Purpose : takes string id, and converts it to the int value of that id.
 * Inputs  : string
 * Returns : int
 *--------------------------------------------------------------------------*/

static uint16_t hex3_to_id(const char in[4]) {
   uint8_t a = hex_val(in[0]);
   uint8_t b = hex_val(in[1]);
   uint8_t c = hex_val(in[2]);

   if (a == 0xFF || b == 0xFF || c == 0xFF) {
      return 0;
   }

   return (uint16_t) ((a << 8) | (b << 4) | c);
}

/*----------------------------------------------------------------------------
 * Function: hex2_to_byte
 * Author  : Anthony di Furia
 * Date    : 12/3/25
 * Purpose : takes byte value, and converts it to the int value of that byte.
 * Inputs  : string
 * Returns : int
 *--------------------------------------------------------------------------*/

static uint8_t hex2_to_byte(const char in[3]) {
   uint8_t hi = hex_val(in[0]);
   uint8_t lo = hex_val(in[1]);

   if (hi == 0xFF || lo == 0xFF) {
      return 0; // handle error however you like
   }

   return (uint8_t) ((hi << 4) | lo);
}

/*----------------------------------------------------------------------------
 * Function: split_hex_line
 * Author  : Anthony di Furia
 * Date    : 12/3/25
 * Purpose : takes the data string line, and splits the line by spaces into
 *        a global array of strings.
 * Inputs  : string
 * Returns : None
 *--------------------------------------------------------------------------*/

void split_hex_line(const char line[]) {
   uint8_t i = 0;  // index in line
   uint8_t t = 0;  // token index
   uint8_t pos = 0;  // index inside current token

   g_token_count = 0;

   while (line[i] != '\0') {
      char c = line[i];

      if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
         // end of current token
         if (pos > 0) {
            g_tokens[t][pos] = '\0';
            t++;
            pos = 0;

            if (t >= MAX_TOKENS) {
               break;
            }
         }
      } else {
         // part of a token
         if (pos < (TOKEN_LEN - 1)) {
            g_tokens[t][pos] = c;
            pos++;
         }
         // else: character dropped if token too long
      }

      i++;
   }

   // if we ended in middle of a token
   if (pos > 0 && t < MAX_TOKENS) {
      g_tokens[t][pos] = '\0';
      t++;
   }

   g_token_count = t;
}

/*----------------------------------------------------------------------------
 * Function: tokens_to_hex_values
 * Author  : Anthony di Furia
 * Date    : 12/3/25
 * Purpose : converts each index of the global string array into its integer
 *        value, then fills the global data stream array.
 * Inputs  : None
 * Returns : None
 *--------------------------------------------------------------------------*/

void tokens_to_hex_values(void) {
   uint8_t i;

   for (i = 0; i < g_token_count; i++) {
      if (i == 0) {
         // first token is 3-digit ID
         g_values[i] = hex3_to_id(g_tokens[i]);
      } else {
         // rest are 2-digit bytes
         g_values[i] = (uint16_t) hex2_to_byte(g_tokens[i]);
      }
   }
}

/*----------------------------------------------------------------------------
 * Function: can_parse_line
 * Author  : Anthony di Furia
 * Date    : 12/3/25
 * Purpose : combines the split_hex_line and tokens_to_hex_values functions
 *        to do the whole operation in one go.
 * Inputs  : CAN BUS line
 * Returns : None
 *--------------------------------------------------------------------------*/

void can_parse_line(const char line[]) {
   split_hex_line(line);
   tokens_to_hex_values();
}

/*----------------------------------------------------------------------------
 * Function: can_read_engine_rpm
 * Author  : Anthony di Furia
 * Date    : 12/3/25
 * Purpose : decodes the data stream and returns the rpm as an integer
 * Inputs  : None
 * Returns : int
 *--------------------------------------------------------------------------*/

int can_read_engine_rpm(void) {
   // Need at least: ID, LEN, 04, 41, 0C, A, B
   if (g_token_count < 7) {
      return -1;
   }

   // Mode should be 0x41 (response to 0x01)
   uint8_t mode = (uint8_t) g_values[3];
   // PID should be 0x0C (engine RPM)
   uint8_t pid = (uint8_t) g_values[4];

   if (mode != 0x41 || pid != 0x0C) {
      return -1;
   }

   uint8_t A = (uint8_t) g_values[5];
   uint8_t B = (uint8_t) g_values[6];

   uint16_t raw = (uint16_t) ((A << 8) | B);  // A*256 + B
   int rpm = (int) (raw / 4);                 // OBD-II formula

   return rpm;
}

/*----------------------------------------------------------------------------
 * Function: can_read_wheel_speed
 * Author  : Anthony di Furia
 * Date    : 12/3/25
 * Purpose : decodes the data stream and returns the wheel speed  in mph
 *        as an integer
 * Inputs  : None
 * Returns : int
 *--------------------------------------------------------------------------*/

// Returns wheel speed in MPH.
// Returns -1 if the current frame is not a valid wheel speed response.
int can_read_wheel_speed(void) {
   // Need at least: ID, LEN, data_len, mode, PID, A
   if (g_token_count < 6) {
      return -1;
   }

   uint8_t mode = (uint8_t) g_values[3];  // should be 0x41
   uint8_t pid = (uint8_t) g_values[4];  // should be 0x0D

   if (mode != 0x41 || pid != 0x0D) {
      return -1;
   }

   // A = speed in mph
   uint8_t A = (uint8_t) g_values[5];
   int speed_kph = (int) A;
   int speed_mph = (int) ((float) speed_kph * 0.621371f + 0.5f);

   return speed_mph;
}

/*----------------------------------------------------------------------------
 * Function: can_read_coolant_temp
 * Author  : Anthony di Furia
 * Date    : 12/3/25
 * Purpose : decodes the data stream and returns the coolant temp in fahrenheit
 *        as an int
 * Inputs  : None
 * Returns : int
 *--------------------------------------------------------------------------*/

int can_read_coolant_temp(void) {
   // Need at least: ID, LEN, data_len, mode, PID, A
   if (g_token_count < 6) {
      return -1;
   }

   uint8_t mode = (uint8_t) g_values[3];  // should be 0x41
   uint8_t pid = (uint8_t) g_values[4];  // should be 0x05

   if (mode != 0x41 || pid != 0x05) {
      return -1;
   }

   uint8_t A = (uint8_t) g_values[5];

   // OBD-II formula: Temp (°C) = A - 40
   int temp_c = (int) A - 40;
   int temp_f = temp_c * 9 / 5 + 32;  // integer math, OK here

   return temp_f;
}

/*----------------------------------------------------------------------------
 * Function: can_read_maf
 * Author  : Anthony di Furia
 * Date    : 12/3/25
 * Purpose : decodes the data stream and returns the mass airflow amount in
 *        grams/second as an int
 * Inputs  : None
 * Returns : int
 *--------------------------------------------------------------------------*/

int can_read_maf(void) {
   // Need at least: ID, LEN, data_len, mode, PID, A
   if (g_token_count < 7) {
      return -1;
   }

   uint8_t mode = (uint8_t) g_values[3];  // should be 0x41
   uint8_t pid = (uint8_t) g_values[4];  // should be 0x10

   if (mode != 0x41 || pid != 0x10) {
      return -1;
   }

   uint8_t A = (uint8_t) g_values[5];
   uint8_t B = (uint8_t) g_values[6];

   int raw = ((256 * A) + B);
   int maf_gps = (int) (raw / 100);

   return maf_gps;
}

/*----------------------------------------------------------------------------
 * Function: can_read_intake
 * Author  : Anthony di Furia
 * Date    : 12/3/25
 * Purpose : decodes the data stream and returns the intake air temp in
 *        fahrenheit as an int
 * Inputs  : None
 * Returns : int
 *--------------------------------------------------------------------------*/

int can_read_intake(void) {
   // Need at least: ID, LEN, data_len, mode, PID, A
   if (g_token_count < 6) {
      return -1;
   }

   uint8_t mode = (uint8_t) g_values[3];  // should be 0x41
   uint8_t pid = (uint8_t) g_values[4];  // should be 0x0F

   if (mode != 0x41 || pid != 0x0F) {
      return -1;
   }

   uint8_t A = (uint8_t) g_values[5];

   int16_t iat_C = (int16_t) A - 40;
   int iat_f = iat_C * 9 / 5 + 32;  // integer math, OK here

   return iat_f;
}

/*----------------------------------------------------------------------------
 * Function: int_to_two_bytes
 * Author  : Anthony di Furia
 * Date    : 12/3/25
 * Purpose : takes an integer, and returns a typedef struct "TwoBytes" that
 *        contains a high and low byte, as each value is two bytes.
 * Inputs  : integer
 * Returns : TwoBytes
 *--------------------------------------------------------------------------*/

TwoBytes int_to_two_bytes(int value) {
   TwoBytes out;

   if (value < 0) {
      value = 0;
   } else if (value > 65535) {
      value = 65535;
   }

   uint16_t v = (uint16_t) value;

   out.hi = (uint8_t) (v >> 8);   // upper 8 bits
   out.lo = (uint8_t) (v & 0xFF); // lower 8 bits

   return out;
}

/*----------------------------------------------------------------------------
 * Function: can_build_data_stream
 * Author  : Anthony di Furia
 * Date    : 12/3/25
 * Purpose : takes all of the values received from the car and encodes them
 *        into a global of stream_bytes
 * Inputs  : int
 * Returns : None
 *--------------------------------------------------------------------------*/

void can_build_data_stream(int rpm, int speed, int coolant, int maf, int intake) {
   TwoBytes r = int_to_two_bytes(rpm);
   TwoBytes sp = int_to_two_bytes(speed);
   TwoBytes ct = int_to_two_bytes(coolant);
   TwoBytes mf = int_to_two_bytes(maf);
   TwoBytes in = int_to_two_bytes(intake);

   stream_bytes[0] = r.hi;
   stream_bytes[1] = r.lo;

   stream_bytes[2] = sp.hi;
   stream_bytes[3] = sp.lo;

   stream_bytes[4] = ct.hi;
   stream_bytes[5] = ct.lo;

   stream_bytes[6] = mf.hi;
   stream_bytes[7] = mf.lo;

   stream_bytes[8] = in.hi;
   stream_bytes[9] = in.lo;
}

/*----------------------------------------------------------------------------
 * Function: can_build_stream_line_from_bytes
 * Author  : Anthony di Furia
 * Date    : 12/3/25
 * Purpose : For debug purposes: prints a string representation of the data
 *        stream.
 * Inputs  : None
 * Returns : None
 *--------------------------------------------------------------------------*/

// to print the string of bytes
void can_build_stream_line_from_bytes(void) {
   int pos = 0;

   for (int i = 0; i < BYTE_COUNT; i++) {
      uint8_t b = stream_bytes[i];
      uint8_t hi = (b >> 4) & 0x0F;
      uint8_t lo = b & 0x0F;

      stream_line[pos++] = (hi < 10) ? ('0' + hi) : ('A' + (hi - 10));
      stream_line[pos++] = (lo < 10) ? ('0' + lo) : ('A' + (lo - 10));
      stream_line[pos++] = ' ';
   }

   stream_line[pos++] = '\r';
   stream_line[pos++] = '\n';
   stream_line[pos] = '\0';
}

