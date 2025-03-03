/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "apps.h"
#include "bord_config.h"
#include "periph.h"
#include "sdram.h"
#include <stdio.h>

void PeriphClockSourceConfig(void);
void SystemClock_Config(void);
void MPU_Init();

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

  /* Enable the CPU Cache */
  MPU_Init();

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* All used periph clock source configoration */
  PeriphClockSourceConfig();

  /* Config M7 SRAM D1 D2 D3 clock */
  sramx_init();

  /* os begin */
  os_run();

  while (1) {
  }
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
   */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
   */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {
  }

  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {
  }

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 8;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 |
                                RCC_CLOCKTYPE_D3PCLK1 | RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK) {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void MPU_Init() {
  HAL_MPU_Disable();

  MPU_Region_InitTypeDef MPU_Config;

  // QSPI FLASH 配置: 最强性能
  MPU_Config.BaseAddress = 0x90000000;
  MPU_Config.Number = MPU_REGION_NUMBER0;
  MPU_Config.Size = MPU_REGION_SIZE_32MB;
  MPU_Config.SubRegionDisable = 0;
  MPU_Config.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_Config.IsBufferable = MPU_ACCESS_BUFFERABLE;
  MPU_Config.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_Config.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_Config.TypeExtField = MPU_TEX_LEVEL1;
  MPU_Config.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  MPU_Config.Enable = MPU_REGION_ENABLE;
  HAL_MPU_ConfigRegion(&MPU_Config);

  // SDRAM 配置: 最强性能
  MPU_Config.BaseAddress = SDRAM_BASE_ADDR;
  MPU_Config.Number = MPU_REGION_NUMBER1;
  MPU_Config.Size = MPU_REGION_SIZE_16MB;
  MPU_Config.SubRegionDisable = 0;
  MPU_Config.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_Config.IsBufferable = MPU_ACCESS_BUFFERABLE;
  MPU_Config.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_Config.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_Config.TypeExtField = MPU_TEX_LEVEL1;
  MPU_Config.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  MPU_Config.Enable = MPU_REGION_ENABLE;
  HAL_MPU_ConfigRegion(&MPU_Config);

  // LTCD 显存配置: 直写不写分配，开启读cache读分配
  MPU_Config.BaseAddress = LTDC_GRAM_ADDR;
  MPU_Config.Number = MPU_REGION_NUMBER2;
  MPU_Config.Size = MPU_REGION_SIZE_1MB;
  MPU_Config.SubRegionDisable = 0;
  MPU_Config.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_Config.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_Config.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
  MPU_Config.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_Config.TypeExtField = MPU_TEX_LEVEL0;
  MPU_Config.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  MPU_Config.Enable = MPU_REGION_ENABLE;
  HAL_MPU_ConfigRegion(&MPU_Config);

  // D1域 SRAM1 配置: 最强性能
  MPU_Config.BaseAddress = 0x24000000;
  MPU_Config.Number = MPU_REGION_NUMBER3;
  MPU_Config.Size = MPU_REGION_SIZE_512KB;
  MPU_Config.SubRegionDisable = 0;
  MPU_Config.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_Config.IsBufferable = MPU_ACCESS_BUFFERABLE;
  MPU_Config.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_Config.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_Config.TypeExtField = MPU_TEX_LEVEL1;
  MPU_Config.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  MPU_Config.Enable = MPU_REGION_ENABLE;
  HAL_MPU_ConfigRegion(&MPU_Config);

  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}
/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
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
void assert_failed(uint8_t *file, uint32_t line) {
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
  printf("HAL assert fail:\r\n\t%s: %ld\r\n", file, line);
}
#endif /* USE_FULL_ASSERT */
