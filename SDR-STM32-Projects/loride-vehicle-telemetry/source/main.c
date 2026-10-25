/*******************************************************************************
 * EE 329 CUSTOM PROJECT
 *******************************************************************************
 * @file           : main.c
 * @brief          : Enables telemetry for vehicle health monitoring.
 *                   Will allow for continuous tx(beacon mode) and rx.
 * project         : EE 329 F'25 CP Assignment: LoRide-R
 * authors         : Leo Alvelais, Anthony DeFuria, Ches Zepp, Juan Gonzalez.
 * version         : 0.5
 * date            : 25125
 * compiler        : STM32CubeIDE v.1.12.0 Build: 14980_20230301_1550 (UTC)
 * target          : NUCLEO-L4A6ZG
 * clocks          : 4 MHz MSI to AHB2
 * @attention      : (c) 2023 STMicroelectronics.  All rights reserved.
 *******************************************************************************
 * CP Plan :
 * Enable SPI1 communication between RFM95W transceiver module and NUCLEO.
 * Enable OBD-II communication between CANbus shield and vehicle CAN bus.
 * Enable UART communication between Arduino and NUCLEO.
 * Configure the operation mode and modem for RFM95W module.
 * Create functions to demonstrate simple wireless pitch and catch via LoRa.
 * Create modular functions for the vehicle and base station nodes
 * Process packet payloads into sensor indicators/quantities for display.
 *******************************************************************************
 * REVISION HISTORY
 * 0.1: Configured communication via SPI between the nucleo and rfm95,
 * programmed the op mode and modem of the rfm95, add function for tx and rx.
 * 0.2: Enabled communication via UART between CAN bus shield and nucleo.
 * 0.3: Demonstrated pitch and catch over LoRa of 32 byte packet.
 * 0.4: Demonstrated wireless transmission and reception of sensor information
 * and printed sensor information to terminal emulator.
 * ***********************
 * TODO
 * Add extra features:
 * Wireless command of car functions.
 * Enable faster communication by reconfiguring the system clock.
 *******************************************************************************

 * 45678-1-2345678-2-2345678-3-2345678-4-2345678-5-2345678-6-2345678-7-234567 */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "SPI.h"
#include "LPUART.h"
#include "delay.h"
#include "UART3.h"
#include "can_read.h"

void SystemClock_Config(void);

int main(void) {

   //INITIALIZE ALL PERIPHERALS, CLOCKS AND COM PORTS
   HAL_Init();

   SystemClock_Config();

   SysTick_Init();
   SPI_Config();
   SPI_init();

   rfm_init();
   LPUART1_init();

   //USE EITHER Vehicle_node() or base_node() depending on if the NUCLEO is
   //bound to the car or acting as the base station respectively.

   //base_node();
   //Vehicle_node();

}

void SystemClock_Config(void) {
   RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
   RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

   /** Configure the main internal regulator output voltage
    */

   if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1)
         != HAL_OK) {
      Error_Handler();
   }

   /** Initializes the RCC Oscillators according to the
    * specified parameters in the RCC_OscInitTypeDef
    * structure.
    */
   RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
   RCC_OscInitStruct.MSIState = RCC_MSI_ON;
   RCC_OscInitStruct.MSICalibrationValue = 0;
   RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
   RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
   if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
      Error_Handler();
   }
   /** Initializes the CPU, AHB and APB buses clocks
    */
   RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
         | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
   RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
   RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
   RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
   RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

   if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
      Error_Handler();
   }
}

void Error_Handler(void) {
   /* USER CODE BEGIN Error_Handler_Debug */
   /* User can add his own implementation to report
    * the HAL error return state */
   __disable_irq();
   while (1) {
   }
   /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
