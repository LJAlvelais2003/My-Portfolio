/*
 ******************************************************************************
 * EE 329 CUSTOM PROJECT
 ******************************************************************************
 * @file           : SPI.c
 * @Created on     : November 15, 2025
 * @brief          : Enables SPI communication, programs RFM95,
 *                   - develops functions for vehicle/base station node.
 * authors         : Leo Alvelais, Anthony DeFuria, Ches Zepp, Juan Gonzalez.
 ******************************************************************************
 *SPI Wiring plan:
 *Peripheral         ---    NUCLEO
 *POWER(3.3V)-Pin1   ---    3.3V = CN8-9
 *CS-Pin11           ---    PA0=CN10-29
 *RESET-Pin10        ---    PA1=CN10-11
 *SCK-Pin12          ---    PA5=CN7-10
 *MISO-Pin6          ---    PA6=CN7-12
 *MOSI-Pin5          ---    PA7=CN7-14
 *******************************************************************************
 */
#include "SPI.h"
#include "main.h"
#include "delay.h"
#include "LPUART.h"
#include "UART3.h"
#include "can_read.h"

/*
 * Function: SPI_Config
 * ----------------------------
 *   Configures SPI1 GPIOA ports for to configure hardware for
 *   -communication between the NUCLEO and the Hope RFM95W module.
 *   NOTE: Uses manual chip select for multi-byte write.
 */

void SPI_Config(void) {

   // enable clock for GPIOA & SPI1
   RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOAEN);              // GPIOA: DAC NSS/SCK/SDO

   RCC->APB2ENR |= (RCC_APB2ENR_SPI1EN);                 // SPI1 port

   //CONFIGURE AF FOR EACH GPIOA PORT:
   //MOSI
   GPIOA->AFR[0] &= ~((0x000F << GPIO_AFRL_AFSEL7_Pos)); // clear nibble for bit 7 AF

   GPIOA->AFR[0] |= ((0x0005 << GPIO_AFRL_AFSEL7_Pos)); // set b7 AF to SPI1 (fcn 5)

   //SCK
   GPIOA->AFR[0] &= ~((0x000F << GPIO_AFRL_AFSEL5_Pos)); // clear nibble for bit 5 AF

   GPIOA->AFR[0] |= ((0x0005 << GPIO_AFRL_AFSEL5_Pos)); // set b5 AF to SPI1 (fcn 5)

   //MISO -- PA6
   GPIOA->AFR[0] &= ~((0x000F << GPIO_AFRL_AFSEL6_Pos));

   GPIOA->AFR[0] |= ((0x0005 << GPIO_AFRL_AFSEL6_Pos));

   /* USER ADD GPIO configuration of MODER/PUPDR/OTYPER/OSPEEDR registers HERE */
   // configure AFR for SPI1 function (1 of 3 SPI bits shown here)
   //CS    -> PA0
   //RESET -> PA1
   SPI_PORT->MODER &= ~( GPIO_MODER_MODE6 | GPIO_MODER_MODE5 | GPIO_MODER_MODE7
         | GPIO_MODER_MODE0 | GPIO_MODER_MODE1);

   //AFR MODE FOR PIN 6,5 AND 7
   SPI_PORT->MODER |= ( GPIO_MODER_MODE6_1 | GPIO_MODER_MODE5_1
         | GPIO_MODER_MODE7_1);

   //PULL DOWN SCK
   SPI_PORT->PUPDR &= ~(GPIO_PUPDR_PUPD5);

   SPI_PORT->PUPDR |= GPIO_PUPDR_PUPD5_0;

   //CONFIGURE MANUAL GPIO OUTPUT FOR CS AND RESET AT HIGH SPD
   SPI_PORT->MODER |= (GPIO_MODER_MODE0_0 | GPIO_MODER_MODE1_0);

   SPI_PORT->OTYPER &= ~(GPIO_OTYPER_OT0 | GPIO_OTYPER_OT1); //PUSH PULL

   SPI_PORT->PUPDR &= ~(GPIO_PUPDR_PUPD0 | GPIO_PUPDR_PUPD1); //NO PU/PD

   //Clearing bits of the speed register prior to setting due to preallocation issues.
   SPI_PORT->OSPEEDR &= ~((3 << GPIO_OSPEEDR_OSPEED0_Pos)
         | (3 << GPIO_OSPEEDR_OSPEED1_Pos));

   SPI_PORT->OSPEEDR |= ((3 << GPIO_OSPEEDR_OSPEED0_Pos)
         | (3 << GPIO_OSPEEDR_OSPEED1_Pos)); //HIGH SPD

}

/*
 * Function: SPI_init
 * --------------------
 * Configures SPI1 for full duplex, 8-bit communication
 * build control registers CR1 & CR2 for SPI control of peripheral DAC
 */

void SPI_init(void) {

   // SPI config as specified @ STM32L4 RM0351 rev.9 p.1459

   // CR1 (reset value = 0x0000)

   SPI1->CR1 &= ~( SPI_CR1_SPE);               // disable SPI for config

   SPI1->CR1 &= ~( SPI_CR1_RXONLY);            // recv-only OFF

   SPI1->CR1 &= ~( SPI_CR1_LSBFIRST);          // data bit order MSb:LSb

   SPI1->CR1 &= ~( SPI_CR1_CPOL | SPI_CR1_CPHA); // SCLK polarity:phase = 0:0

   SPI1->CR1 |= SPI_CR1_MSTR;                // MCU is SPI controller

   SPI1->CR1 &= ~(SPI_CR1_BR_Msk);

   SPI1->CR1 |= SPI_CR1_BR_1;

   // CR2 (reset value = 0x0700 : 8b data)
   SPI1->CR2 &= ~( SPI_CR2_TXEIE | SPI_CR2_RXNEIE); // disable FIFO intrpts

   SPI1->CR2 &= ~( SPI_CR2_FRF);                // Moto frame format

   //CS PULSE IS MANUALLY GENERATED
   SPI1->CR2 |= SPI_CP_DS;                  // 8-bit data

   SPI1->CR2 |= SPI_CR2_SSOE;                // enable SS output

   // ENABLE SPI
   SPI1->CR1 |= SPI_CR1_SPE;                 // re-enable SPI for ops

   SPI1->CR2 |= SPI_CR2_FRXTH;
   //SET RS AND CS IDLE HIGH
   CS_high();

   RS_high();
}

/*
 * Function: CS_low
 * --------------------
 * Pulls the chip select ported to the RFM95 low.
 * Should be used at the start of every SPI write or read.
 */
void CS_low(void) {
   SPI_PORT->BRR |= CS_PIN;
}

/*
 * Function: CS_high
 * --------------------
 * Pulls the chip select ported to the RFM95 high.
 * Should be used at the end of every SPI write or read.
 */
void CS_high(void) {
   SPI_PORT->BSRR |= CS_PIN;
}

/*
 * Function: RS_high
 * --------------------
 * Pulls the chip select ported to the RFM95 high.
 * Should be used to set RFM95 into default settings.
 */

void RS_high(void) {
   SPI_PORT->BSRR |= RS_PIN;
}

/*
 * Function: RS_high
 * --------------------
 * Pulls the chip select ported to the RFM95 low.
 * Should be used after RS_low after a delay
 */
void RS_low(void) {
   SPI_PORT->BRR |= RS_PIN;
}

/*
 * Function:  SPI_wr_b
 * --------------------
 * Writes and receives a byte over SPI
 *
 *  n: The byte to be written.
 *
 *  returns: A byte sent back by the peripheral
 *
 *  Assumes the data bus line is never busy.
 */

uint8_t SPI_wr_b(uint8_t b) {

   while (!(SPI1->SR & SPI_SR_TXE)) {
   };

   *(__IO uint8_t*) &SPI1->DR = b;

   while (!(SPI1->SR & SPI_SR_RXNE)) {
   };

   uint8_t br = *(__IO uint8_t*) &SPI1->DR;

   return br;

}

/*
 * Function:  SPI_wr_nB
 * --------------------
 * Writes n number of bytes via SPI
 *
 *  data: bytes to be written
 *
 *  n: number of bytes to be written
 *
 *  Assumes the data bus line is never busy.
 */

void SPI_wr_nB(uint8_t *data, uint32_t n) {

   for (uint32_t W_Idx = 0; W_Idx < n; W_Idx++) {

      (void) SPI_wr_b(data[W_Idx]);
   }
}

/*
 * Function:  SPI_rd_nB
 * --------------------
 * Reads n number of bytes via SPI.
 *
 *  data: bytes to be written
 *
 *  n: number of bytes to be written
 *
 *  Assumes the data bus line is never busy.
 */
void SPI_rd_nB(uint8_t *data, uint32_t n) {

   for (uint32_t Rd_Idx = 0; Rd_Idx < n; Rd_Idx++) {

      data[Rd_Idx] = SPI_wr_b(0x00);
   }
}

/*
 * Function:  rfm_wr
 * --------------------
 * Writes a byte to rfm memory address
 *
 *  addr: address to be written to
 *
 *  b: byte to write to memory address
 *
 *  CS must be toggled low and high before and after each
 *  byte respectively.
 */
void rfm_wr(uint8_t addr, uint8_t b) {

   uint8_t frame[2] = { (0x80 | addr), b };

   CS_low();

   SPI_wr_nB(frame, 2);

   CS_high();
}

/*
 * Function:  rfm_rd
 * --------------------
 *  reads a byte from rfm memory address
 *
 * addr: address to be read
 *
 *  returns: a byte from rfm memory at specified address
 *
 *  for a single read, two byte transfers must occur,
 *  after which CS is toggled
 *  A 'dummy' byte must be sent to keep SCK high for
 *  peripheral data read.
 */

uint8_t rfm_rd(uint8_t addr) {

   CS_low();

   (void) SPI_wr_b((0x7F & addr));

   uint8_t out = SPI_wr_b(0x00);

   CS_high();

   return out;
}

/*
 * Function:  rfm_wr_nB
 * --------------------
 * Writes n number of bytes to rfm memory, only used to
 * write to the fifo buffer for this particular project.
 * Fifo buffer contains contents from received packet or
 * contents to transmit.
 *
 * addr: address to write to.
 *
 *  Wdata: Data buffer to write.
 *
 *
 *
 * CS must be toggled on and off before and after the entire
 * n-byte transfer respectively
 */

void rfm_wr_nB(uint8_t addr, uint8_t *Wdata, uint32_t len) {

   CS_low();

   SPI_wr_b((0x80 | addr));

   SPI_wr_nB(Wdata, len);

   CS_high();
}

/*
 * Function:  rfm_rd_nB
 * --------------------
 * Writes a byte to rfm memory address
 *
 * addr: address to be read from. Address increments each byte
 * transfer from memory.
 *
 * len: number of bytes to be read.
 *
 * Rdata: Data buffer for bytes to be stored into.
 */

void rfm_rd_nB(uint8_t addr, uint8_t *Rdata, uint32_t len) {

   CS_low();

   SPI_wr_b((0x7F & addr));

   SPI_rd_nB(Rdata, len);

   CS_high();
}

/*
 * Function:  Rst_pulse
 * --------------------
 * Pulses Rst pin to set rfm module into sleep mode and
 * set default settings.
 *
 * Per the rfm manual, a 100us delay must be set
 * after pulling rst low,and a 5ms delay must be
 * set after pulling rst high for proper rfm
 * functionality.
 */

void Rst_pulse(void) {

   RS_low();

   delay_us(200);

   RS_high();

   delay_ms(6);
}

/*
 * Function:  rfm_fifo_wr
 * --------------------
 * Writes n number of bytes to the fifo buffer of the rfm.
 *
 * data: Data to be written into fifo buffer.
 *
 * len: Number of bytes to be written.
 *
 * This function is to be used for transmitting data wirelessly.
 * The fifo buffer is partioned for rx and tx data, this can be changed
 * such that the entire fifo buffer is used for either rx or tx
 */
void rfm_fifo_wr(uint8_t *data, uint32_t len) {
   rfm_wr_nB(0x00, data, len);
}

/*
 * Function:  rfm_fifo_rd
 * --------------------
 * Read n number of bytes from fifo buffer
 *
 * data: Data to be written into fifo buffer.
 *
 * len: Number of bytes to be written.
 *
 */

void rfm_fifo_rd(uint8_t *data, uint8_t len) {
   rfm_rd_nB(0x00, data, len);
}

/*
 * Function:  rfm_init
 * --------------------
 * Configures the modem and operation mode of the rfm module.
 * This includes, amplifiers, modulation scheme,
 * bandwidth, carrier frequency, preamble length, spreading factor,
 * and sync word.
 *
 *
 */

void rfm_init(void) {

   Rst_pulse();
   //------OP MODE ------//
   //LONG RANGE MODE W/SLEEP
   rfm_wr(0x01, 0x80);
   delay_ms(1);
   //PUT INTO STANDBY MODE
   //HF
   //LORA REGs
   //
   rfm_wr(0x01, 0x81);

   //------CARRIER FREQUENCY------//
   //915 MHz CARRIER FREQUENCY WITH ~60 Hz STEPS
   rfm_wr(0x06, 0xE4);
   rfm_wr(0x07, 0xC0);
   rfm_wr(0x08, 0x00);

   //------PA, OSC, AND LNAg CONFIGURATIONS------//
   //+15dBm THROUGH PA BOOST
   rfm_wr(0x09, 0x8D);
   //MAX 100mA OVERCURRENT PROTECTION
   rfm_wr(0x0B, 0x2B);
   //LNA
   //MAXIMUM GAIN
   //LNA BOOST ON
   rfm_wr(0x0C, 0x23);

   //------MODEM CONFIG------//
   //BASE ADDRESSES FOR TX AND RX FIFO R FINE AS DEFAULT

   //125 kHz BANDWIDTH
   //4/6 CODING RATE
   //EXPLICIT HEADER MODE
   rfm_wr(0x1D, 0x72);
   // SF OF 7
   //CRC ON

   //rfm_wr(0x1E, 0x74);
   rfm_wr(0x1E, 0x94);

   //PREAMBLE LENGTH -- 8 SYMBOLS
   rfm_wr(0x20, 0x00);
   rfm_wr(0x21, 0x08);

   //SYNC WORD FOR NETWORK
   rfm_wr(0x39, 0x12);

   //------DIO INTERRUPTS-------//
   //ENABLE INTERRUPTS TO PROPAGATE THROUGH DIO PINS
   rfm_wr(0x11, 0x00);
   //CLEAR PENDING FLAGS8
   rfm_wr(0x12, 0xFF);

}

/*
 * Function:  rfm_tx
 * --------------------
 * Transmits n number of bytes.
 *
 * data: Data to be transmitted.
 *
 * size: Size of packet to be transmitted
 *
 */

void rfm_tx(uint8_t *data, uint32_t size) {
   //SET FIFO BUFFER STARTING ADDRESS TO 0X80
   rfm_wr(0x0D, 0x80);

   //SPECIFY THE SIZE OF THE PAYLOAD TO BE TX
   rfm_wr(0x22, size);

   //WRITE DATA TO FIFO BUFFER
   rfm_fifo_wr(data, size);

   //CLEAR PENDING INTERRUPTS
   rfm_wr(0x12, 0xFF);

   //MAP TX DONE INT TO DIO0
   rfm_wr(0x40, 0x40);

   //TURN ON TX MODE IN REGOP REG
   rfm_wr(0x01, 0x83);

   //WAIT FOR TX DONE
   while (!(rfm_rd(0x12) & TX_DONE)) {
   }

   //CLEAR TX DONE INT
   rfm_wr(0x12, TX_DONE);

   //SET BACK TO STANDBY MODE
   rfm_wr(0x01, 0x81);

}

/*
 * Function: RX_Cont
 * --------------------
 * Sets the rfm into continuous receive mode
 *
 */

void RX_Cont(void) {
   //CLEAR PENDING FLAGS
   rfm_wr(0x12, 0xFF);

   //SET FIFO BASE ADDR PTR TO FIFO RX BASE
   rfm_wr(0x0D, 0x00);

   //MAP DIO0 TO RXDONE FLAG
   rfm_wr(0x40, 0x00);

   //STDBY--> CONT RX
   rfm_wr(0x01, 0x85);

}

/*
 * Function:  RX_polling
 * --------------------
 * Polls for valid packet, ie valid crc and makes sure rx complete.
 * Stores received data into buffer along.
 * Truncates packet payload if packet is longer than expected.
 *
 * data: Data buffer for received packet payload.
 *
 * max_len: The max length in bytes of the payload to be received.
 *
 */

uint8_t RX_polling(uint8_t *data, uint8_t max_len) {

   //RD FLAGS CHECK FOR CRC ERR AND RX DONE
   uint8_t flags = rfm_rd(0x12);

   if (flags & RX_DONE) {

      if (!(flags & CRC_ERR)) {

         uint8_t rx_pckt_sz = rfm_rd(0x13);

         if (rx_pckt_sz > max_len) {

            rx_pckt_sz = max_len;
         }

         //RECEIVED PACKET STARTING ADDR

         uint8_t addr = rfm_rd(0x10);

         //SET FIFO ADDR POINTER TO RC PCKT ADDR IN FIFO

         rfm_wr(0x0D, addr);

         //READ PACKET

         rfm_fifo_rd(data, rx_pckt_sz);

         //CLEAR PENDING FLAGS

         rfm_wr(0x12, (RX_DONE | CRC_ERR));

         return rx_pckt_sz;
      } else {
         rfm_wr(0x12, (RX_DONE | CRC_ERR));
      }
   }
   return 0;
}

/*
 * Function: Pitch_Demo
 * Demonstrates a 32 Byte tx over LoRa
 * --------------------

 *
 */
void Pitch_Demo(void) {

   uint8_t sz = 32;

   uint8_t msg[sz];

   while (1) {

      for (uint8_t Idx = 0; Idx < 32; Idx++) {

         msg[Idx] = Idx;

      }

      rfm_tx(msg, sz);

      delay_ms(500);
   }

}

/*
 * Function: Catch_Demo
 * --------------------
 * Demonstrates receiving a 32 byte packet over LoRa.
 */
void Catch_Demo(void) {

   RX_Cont();

   uint8_t data[32];

   while (1) {

      uint8_t rx_stat = RX_polling(data, PCK_LEN);

      if ((rx_stat != 0)) {

         CLR_Screen();

         for (uint8_t Idx = 0; Idx < rx_stat; Idx++) {

            LPUART_Wr_Dig(data[Idx]);

         }

      }

   }

}

/*
 * Function: stby_mode
 * --------------------
 * Places rfm module into standby mode.
 */
void stby_mode(void) {

   rfm_wr(0x01, 0x81);

}

/*
 * Function: base_node
 * --------------------
 * Polls for a valid vehicle node packet and translates payload
 * data into sensor measurements. Prints measurements to terminal emulator.
 *
 */
void base_node(void) {

   CLR_Screen();

   uint8_t rx_buf[32];

   static const char *Indicators[6] = { " ",

   "RPM:      RPM",

   "Speed:      MPH",

   "Coolant Temperature:      degrees Farenheit",

   "Mass Airflow:      grams / second",

   "Intake Air Temperature:      degrees Farenheit" };

   static const uint8_t data_pos[5] = {

   6,

   8,

   22,

   15,

   25 };

   uint8_t Idx = 0;

   for (Idx = 0; Idx < 6; Idx++) {

      Set_Pos(0, Idx);

      LPUART_Print(Indicators[Idx]);
   }

   RX_Cont();

   while (1) {

      uint8_t pckt_sz = RX_polling(rx_buf, PCK_LEN);

      Idx = 0;

      uint8_t cnt = 0;

      if (pckt_sz != 0) {

         uint16_t Indicator_vals[10];

         char buf[32];

         while (Idx < 10) {

            Indicator_vals[cnt] = (rx_buf[Idx] << 8) | rx_buf[Idx + 1];

            Set_Pos(data_pos[cnt], cnt + 1);

            LPUART_Print("     ");

            u16_2_str(Indicator_vals[cnt], buf);

            Set_Pos(data_pos[cnt], cnt + 1);

            LPUART_Print(buf);

            Idx += 2;

            cnt++;

         }

      }

   }

}

/*
 * Function: Vehicle_node
 * --------------------
 * Turns vehicle based NUCLEO into a beacon.
 * Continuously streams CAN bus
 */
void Vehicle_node(void) {

   UART3_Init(2000000UL, 115200UL);   // USART3 init

   LPUART_ESC_Print("[2J"); // clear screen
   LPUART_ESC_Print("[H");  // move to 1,1
   LPUART_ESC_Print("[?25l"); // disable cursor on LPUART

   char line[64];

   int rpm = 0;
   int speed = 0;
   int coolant_temp = 0;
   int maf = 0;
   int intake_air = 0;

   int tx_delay = 0;

   while (1) {

      //read line from the UART
      int len = UART3_ReadLine(line, sizeof(line));

      //update CAN line here
      can_parse_line(line);

      int test_rpm = can_read_engine_rpm();
      if (test_rpm >= 0) {
         rpm = test_rpm;
      }

      int test_speed = can_read_wheel_speed();
      if (test_speed >= 0) {
         speed = test_speed;
      }
      int test_coolant_temp = can_read_coolant_temp();
      if (test_coolant_temp >= 0) {
         coolant_temp = test_coolant_temp;
      }
      int test_maf = can_read_maf();
      if (test_maf >= 0) {
         maf = test_maf;
      }
      int test_intake = can_read_intake();
      if (test_intake >= 0) {
         intake_air = test_intake;
      }

      // prepare data to send
      can_build_data_stream(rpm, speed, coolant_temp, maf, intake_air);
      can_build_stream_line_from_bytes();

      // send data

      if (tx_delay == 0) {
         rfm_tx(stream_bytes, BYTE_COUNT);
      }

      tx_delay++;

      if (tx_delay > 5) {
         tx_delay = 0;
      }

      // refresh UART display, but not too quickly
      LPUART_ESC_Print("[H");  // move to 1,1
      LPUART_Print(line);     // print current OBD2 line
      LPUART_ESC_Print("[K"); // erase from cursor to end of line
      LPUART_Print("\n\r");

      LPUART_Print("RPM: ");
      LPUART_PrintInt(rpm);   // print rpm number
      LPUART_ESC_Print("[K"); // erase from cursor to end of line
      LPUART_Print("\n\r");   // next line

      LPUART_Print("Speed (mph): ");
      LPUART_PrintInt(speed);
      LPUART_ESC_Print("[K"); // erase from cursor to end of screen
      LPUART_Print("\n\r");

      LPUART_Print("Coolant Temp (F): ");
      LPUART_PrintInt(coolant_temp);
      LPUART_ESC_Print("[K"); // erase from cursor to end of line
      LPUART_Print("\n\r");

      LPUART_Print("Mass Airflow Sensor (grams/s): ");
      LPUART_PrintInt(maf);
      LPUART_ESC_Print("[K"); // erase from cursor to end of line
      LPUART_Print("\n\r");

      LPUART_Print("Intake Air Temperature (F): ");
      LPUART_PrintInt(intake_air);
      LPUART_ESC_Print("[K"); // erase from cursor to end of line
      LPUART_Print("\n\r");

      LPUART_Print("\n\r");

      LPUART_Print("Data Stream: ");
      LPUART_Print(stream_line);
      LPUART_ESC_Print("[K");

      LPUART_Print("\n\r");

   }
}

