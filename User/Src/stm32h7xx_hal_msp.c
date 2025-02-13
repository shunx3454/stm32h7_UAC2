/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file         stm32h7xx_hal_msp.c
 * @brief        This file provides code for the MSP Initialization
 *               and de-Initialization codes.
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

RCC_PeriphCLKInitTypeDef g_PeriphClkInitStruct = {0};

/**
 * Initializes the Global MSP.
 */
void HAL_MspInit(void) {
  __HAL_RCC_SYSCFG_CLK_ENABLE();
}

void PeriphClockSourceConfig(void) {
  /* Initializes the PLL2, PLL3 */
  g_PeriphClkInitStruct.PLL2.PLL2M = 5;
  g_PeriphClkInitStruct.PLL2.PLL2N = 192;
  g_PeriphClkInitStruct.PLL2.PLL2P = 8;
  g_PeriphClkInitStruct.PLL2.PLL2Q = 8;
  g_PeriphClkInitStruct.PLL2.PLL2R = 2;
  g_PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
  g_PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_2;
  g_PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
  g_PeriphClkInitStruct.PLL3.PLL3M = 5;
  g_PeriphClkInitStruct.PLL3.PLL3N = 192;
  g_PeriphClkInitStruct.PLL3.PLL3P = 2;
  g_PeriphClkInitStruct.PLL3.PLL3Q = 20;
  g_PeriphClkInitStruct.PLL3.PLL3R = 12;
  g_PeriphClkInitStruct.PLL3.PLL3FRACN = 0;
  g_PeriphClkInitStruct.PLL3.PLL3RGE = RCC_PLL3VCIRANGE_2;
  g_PeriphClkInitStruct.PLL3.PLL3VCOSEL = RCC_PLL2VCOWIDE;

  /* Used periph clock config */
  /* USART1 */
  g_PeriphClkInitStruct.PeriphClockSelection |= RCC_PERIPHCLK_USART1;
  g_PeriphClkInitStruct.Usart16ClockSelection = RCC_USART16CLKSOURCE_PCLK2;
  /* SDMMC1 */
  g_PeriphClkInitStruct.PeriphClockSelection |= RCC_PERIPHCLK_SDMMC;
  g_PeriphClkInitStruct.SdmmcClockSelection = RCC_SDMMCCLKSOURCE_PLL;
  /* FMC */
  g_PeriphClkInitStruct.PeriphClockSelection |= RCC_PERIPHCLK_FMC;
  g_PeriphClkInitStruct.FmcClockSelection = RCC_FMCCLKSOURCE_D1HCLK;
  /* LTDC */
  g_PeriphClkInitStruct.PeriphClockSelection |= RCC_PERIPHCLK_LTDC;
  /* I2C2 */
  g_PeriphClkInitStruct.PeriphClockSelection |= RCC_PERIPHCLK_I2C2;
  g_PeriphClkInitStruct.I2c123ClockSelection = RCC_I2C123CLKSOURCE_D2PCLK1;
  /* DMA2D, cannot choose */
  /* I2S1/SPI1 */
  g_PeriphClkInitStruct.PeriphClockSelection |= RCC_PERIPHCLK_SPI1;
  g_PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL;
  /* USB FS */
  g_PeriphClkInitStruct.PeriphClockSelection |= RCC_PERIPHCLK_USB;
  g_PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_PLL3;
  if (HAL_RCCEx_PeriphCLKConfig(&g_PeriphClkInitStruct) != HAL_OK) {
    Error_Handler();
  }
}

void HAL_UART_MspInit(UART_HandleTypeDef *uartHandle) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if (uartHandle->Instance == USART1) {
    /* USART1 clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART1 GPIO Configuration
    PA10     ------> USART1_RX
    PA9     ------> USART1_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef *uartHandle) {
  if (uartHandle->Instance == USART1) {
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_10 | GPIO_PIN_9);
  }
}

void BSP_SD_MspInitCallback(SD_HandleTypeDef *hsd) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if (hsd->Instance == SDMMC1) {
    /* SDMMC1 clock enable */
    __HAL_RCC_SDMMC1_CLK_ENABLE();

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    /**SDMMC1 GPIO Configuration
    PC10     ------> SDMMC1_D2
    PC11     ------> SDMMC1_D3
    PC12     ------> SDMMC1_CK
    PD2     ------> SDMMC1_CMD
    PC8     ------> SDMMC1_D0
    PC9     ------> SDMMC1_D1
    */
    GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_8 | GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_SDIO1;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_SDIO1;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* SDMMC1 interrupt Init */
    HAL_NVIC_SetPriority(SDMMC1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(SDMMC1_IRQn);
  }
}

void HAL_SD_MspDeInit(SD_HandleTypeDef *sdHandle) {
  if (sdHandle->Instance == SDMMC1) {
    /* Peripheral clock disable */
    __HAL_RCC_SDMMC1_CLK_DISABLE();

    /**SDMMC1 GPIO Configuration
    PC10     ------> SDMMC1_D2
    PC11     ------> SDMMC1_D3
    PC12     ------> SDMMC1_CK
    PD2     ------> SDMMC1_CMD
    PC8     ------> SDMMC1_D0
    PC9     ------> SDMMC1_D1
    */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_8 | GPIO_PIN_9);

    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_2);

    /* SDMMC1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(SDMMC1_IRQn);
  }
}

static uint32_t FMC_Initialized = 0;

static void HAL_FMC_MspInit(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if (FMC_Initialized) {
    return;
  }
  FMC_Initialized = 1;

  /* Peripheral clock enable */
  __HAL_RCC_FMC_CLK_ENABLE();
  __HAL_RCC_GPIOI_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();

  /** FMC GPIO Configuration
  PI6   ------> FMC_D28
  PI5   ------> FMC_NBL3
  PI4   ------> FMC_NBL2
  PI1   ------> FMC_D25
  PI0   ------> FMC_D24
  PI7   ------> FMC_D29
  PE1   ------> FMC_NBL1
  PI2   ------> FMC_D26
  PH15   ------> FMC_D23
  PH14   ------> FMC_D22
  PE0   ------> FMC_NBL0
  PI3   ------> FMC_D27
  PG15   ------> FMC_SDNCAS
  PD0   ------> FMC_D2
  PH13   ------> FMC_D21
  PI9   ------> FMC_D30
  PD1   ------> FMC_D3
  PI10   ------> FMC_D31
  PG8   ------> FMC_SDCLK
  PF2   ------> FMC_A2
  PF1   ------> FMC_A1
  PF0   ------> FMC_A0
  PG5   ------> FMC_BA1
  PF3   ------> FMC_A3
  PG4   ------> FMC_BA0
  PF5   ------> FMC_A5
  PF4   ------> FMC_A4
  PC0   ------> FMC_SDNWE
  PH2   ------> FMC_SDCKE0
  PE10   ------> FMC_D7
  PH3   ------> FMC_SDNE0
  PF13   ------> FMC_A7
  PF14   ------> FMC_A8
  PE9   ------> FMC_D6
  PE11   ------> FMC_D8
  PH10   ------> FMC_D18
  PH11   ------> FMC_D19
  PD15   ------> FMC_D1
  PD14   ------> FMC_D0
  PF12   ------> FMC_A6
  PF15   ------> FMC_A9
  PE12   ------> FMC_D9
  PE15   ------> FMC_D12
  PH9   ------> FMC_D17
  PH12   ------> FMC_D20
  PF11   ------> FMC_SDNRAS
  PG0   ------> FMC_A10
  PE8   ------> FMC_D5
  PE13   ------> FMC_D10
  PH8   ------> FMC_D16
  PD10   ------> FMC_D15
  PD9   ------> FMC_D14
  PG1   ------> FMC_A11
  PE7   ------> FMC_D4
  PE14   ------> FMC_D11
  PD8   ------> FMC_D13
  */
  /* GPIO_InitStruct */
  GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_5 | GPIO_PIN_4 | GPIO_PIN_1 | GPIO_PIN_0 | GPIO_PIN_7 | GPIO_PIN_2 |
                        GPIO_PIN_3 | GPIO_PIN_9 | GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;

  HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

  /* GPIO_InitStruct */
  GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_0 | GPIO_PIN_10 | GPIO_PIN_9 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_15 |
                        GPIO_PIN_8 | GPIO_PIN_13 | GPIO_PIN_7 | GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;

  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /* GPIO_InitStruct */
  GPIO_InitStruct.Pin = GPIO_PIN_15 | GPIO_PIN_14 | GPIO_PIN_13 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_10 | GPIO_PIN_11 |
                        GPIO_PIN_9 | GPIO_PIN_12 | GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;

  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  /* GPIO_InitStruct */
  GPIO_InitStruct.Pin = GPIO_PIN_15 | GPIO_PIN_8 | GPIO_PIN_5 | GPIO_PIN_4 | GPIO_PIN_0 | GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;

  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /* GPIO_InitStruct */
  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_15 | GPIO_PIN_14 | GPIO_PIN_10 | GPIO_PIN_9 | GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;

  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* GPIO_InitStruct */
  GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_1 | GPIO_PIN_0 | GPIO_PIN_3 | GPIO_PIN_5 | GPIO_PIN_4 | GPIO_PIN_13 |
                        GPIO_PIN_14 | GPIO_PIN_12 | GPIO_PIN_15 | GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;

  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /* GPIO_InitStruct */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;

  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void HAL_SDRAM_MspInit(SDRAM_HandleTypeDef *sdramHandle) {
  HAL_FMC_MspInit();
}

static uint32_t FMC_DeInitialized = 0;

static void HAL_FMC_MspDeInit(void) {
  if (FMC_DeInitialized) {
    return;
  }
  FMC_DeInitialized = 1;
  /* Peripheral clock enable */
  __HAL_RCC_FMC_CLK_DISABLE();

  /** FMC GPIO Configuration
  PI6   ------> FMC_D28
  PI5   ------> FMC_NBL3
  PI4   ------> FMC_NBL2
  PI1   ------> FMC_D25
  PI0   ------> FMC_D24
  PI7   ------> FMC_D29
  PE1   ------> FMC_NBL1
  PI2   ------> FMC_D26
  PH15   ------> FMC_D23
  PH14   ------> FMC_D22
  PE0   ------> FMC_NBL0
  PI3   ------> FMC_D27
  PG15   ------> FMC_SDNCAS
  PD0   ------> FMC_D2
  PH13   ------> FMC_D21
  PI9   ------> FMC_D30
  PD1   ------> FMC_D3
  PI10   ------> FMC_D31
  PG8   ------> FMC_SDCLK
  PF2   ------> FMC_A2
  PF1   ------> FMC_A1
  PF0   ------> FMC_A0
  PG5   ------> FMC_BA1
  PF3   ------> FMC_A3
  PG4   ------> FMC_BA0
  PF5   ------> FMC_A5
  PF4   ------> FMC_A4
  PC0   ------> FMC_SDNWE
  PH2   ------> FMC_SDCKE0
  PE10   ------> FMC_D7
  PH3   ------> FMC_SDNE0
  PF13   ------> FMC_A7
  PF14   ------> FMC_A8
  PE9   ------> FMC_D6
  PE11   ------> FMC_D8
  PH10   ------> FMC_D18
  PH11   ------> FMC_D19
  PD15   ------> FMC_D1
  PD14   ------> FMC_D0
  PF12   ------> FMC_A6
  PF15   ------> FMC_A9
  PE12   ------> FMC_D9
  PE15   ------> FMC_D12
  PH9   ------> FMC_D17
  PH12   ------> FMC_D20
  PF11   ------> FMC_SDNRAS
  PG0   ------> FMC_A10
  PE8   ------> FMC_D5
  PE13   ------> FMC_D10
  PH8   ------> FMC_D16
  PD10   ------> FMC_D15
  PD9   ------> FMC_D14
  PG1   ------> FMC_A11
  PE7   ------> FMC_D4
  PE14   ------> FMC_D11
  PD8   ------> FMC_D13
  */

  HAL_GPIO_DeInit(GPIOI, GPIO_PIN_6 | GPIO_PIN_5 | GPIO_PIN_4 | GPIO_PIN_1 | GPIO_PIN_0 | GPIO_PIN_7 | GPIO_PIN_2 |
                             GPIO_PIN_3 | GPIO_PIN_9 | GPIO_PIN_10);

  HAL_GPIO_DeInit(GPIOE, GPIO_PIN_1 | GPIO_PIN_0 | GPIO_PIN_10 | GPIO_PIN_9 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_15 |
                             GPIO_PIN_8 | GPIO_PIN_13 | GPIO_PIN_7 | GPIO_PIN_14);

  HAL_GPIO_DeInit(GPIOH, GPIO_PIN_15 | GPIO_PIN_14 | GPIO_PIN_13 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_10 | GPIO_PIN_11 |
                             GPIO_PIN_9 | GPIO_PIN_12 | GPIO_PIN_8);

  HAL_GPIO_DeInit(GPIOG, GPIO_PIN_15 | GPIO_PIN_8 | GPIO_PIN_5 | GPIO_PIN_4 | GPIO_PIN_0 | GPIO_PIN_1);

  HAL_GPIO_DeInit(GPIOD, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_15 | GPIO_PIN_14 | GPIO_PIN_10 | GPIO_PIN_9 | GPIO_PIN_8);

  HAL_GPIO_DeInit(GPIOF, GPIO_PIN_2 | GPIO_PIN_1 | GPIO_PIN_0 | GPIO_PIN_3 | GPIO_PIN_5 | GPIO_PIN_4 | GPIO_PIN_13 |
                             GPIO_PIN_14 | GPIO_PIN_12 | GPIO_PIN_15 | GPIO_PIN_11);

  HAL_GPIO_DeInit(GPIOC, GPIO_PIN_0);
}

void HAL_SDRAM_MspDeInit(SDRAM_HandleTypeDef *sdramHandle) {
  HAL_FMC_MspDeInit();
}

void HAL_LTDC_MspInit(LTDC_HandleTypeDef *ltdcHandle) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if (ltdcHandle->Instance == LTDC) {
    /* LTDC clock enable */
    __HAL_RCC_LTDC_CLK_ENABLE();

    __HAL_RCC_GPIOI_CLK_ENABLE();
    __HAL_RCC_GPIOJ_CLK_ENABLE();
    __HAL_RCC_GPIOK_CLK_ENABLE();

    GPIO_InitStruct.Pin =
        GPIO_PIN_5 | GPIO_PIN_4 | GPIO_PIN_6 | GPIO_PIN_3 | GPIO_PIN_7 | GPIO_PIN_2 | GPIO_PIN_0 | GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
    HAL_GPIO_Init(GPIOK, &GPIO_InitStruct);

    // GPIO_InitStruct.Pin = GPIO_PIN_15 | GPIO_PIN_14(B2) | GPIO_PIN_12(B0) | GPIO_PIN_13(B1) | GPIO_PIN_11 |
    // GPIO_PIN_10 |
    //                       GPIO_PIN_9 | GPIO_PIN_8(G1) | GPIO_PIN_7(G0) | GPIO_PIN_6 | GPIO_PIN_1(R2) | GPIO_PIN_5 |
    //                       GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_0(R1);
    GPIO_InitStruct.Pin = GPIO_PIN_15 | GPIO_PIN_11 | GPIO_PIN_10 | GPIO_PIN_9 | GPIO_PIN_6 | GPIO_PIN_5 | GPIO_PIN_2 |
                          GPIO_PIN_3 | GPIO_PIN_4;

    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
    HAL_GPIO_Init(GPIOJ, &GPIO_InitStruct);

    // GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15(R0);
    GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
    HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);
  }
}

void HAL_LTDC_MspDeInit(LTDC_HandleTypeDef *ltdcHandle) {
  if (ltdcHandle->Instance == LTDC) {
    /* Peripheral clock disable */
    __HAL_RCC_LTDC_CLK_DISABLE();

    HAL_GPIO_DeInit(GPIOK, GPIO_PIN_5 | GPIO_PIN_4 | GPIO_PIN_6 | GPIO_PIN_3 | GPIO_PIN_7 | GPIO_PIN_2 | GPIO_PIN_0 |
                               GPIO_PIN_1);

    HAL_GPIO_DeInit(GPIOJ, GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_9 | GPIO_PIN_10 |
                               GPIO_PIN_11 | GPIO_PIN_15);

    HAL_GPIO_DeInit(GPIOI, GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14);
  }
}

void HAL_I2C_MspInit(I2C_HandleTypeDef *i2cHandle) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if (i2cHandle->Instance == I2C2) {
    __HAL_RCC_GPIOH_CLK_ENABLE();
    /**I2C2 GPIO Configuration
    PH4     ------> I2C2_SCL
    PH5     ------> I2C2_SDA
    */
    GPIO_InitStruct.Pin = TOUCH_IIC_CLK_Pin | TOUCH_IIC_SDA_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
    HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

    /* I2C2 clock enable */
    __HAL_RCC_I2C2_CLK_ENABLE();
  }
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef *i2cHandle) {

  if (i2cHandle->Instance == I2C2) {
    /* Peripheral clock disable */
    __HAL_RCC_I2C2_CLK_DISABLE();

    /**I2C2 GPIO Configuration
    PH4     ------> I2C2_SCL
    PH5     ------> I2C2_SDA
    */
    HAL_GPIO_DeInit(TOUCH_IIC_CLK_GPIO_Port, TOUCH_IIC_CLK_Pin);
    HAL_GPIO_DeInit(TOUCH_IIC_SDA_GPIO_Port, TOUCH_IIC_SDA_Pin);
  }
}

void HAL_DMA2D_MspInit(DMA2D_HandleTypeDef *dma2dHandle) {
  if (dma2dHandle->Instance == DMA2D) {
    /* DMA2D clock enable */
    __HAL_RCC_DMA2D_CLK_ENABLE();
  }
}

void HAL_DMA2D_MspDeInit(DMA2D_HandleTypeDef *dma2dHandle) {
  if (dma2dHandle->Instance == DMA2D) {
    /* Peripheral clock disable */
    __HAL_RCC_DMA2D_CLK_DISABLE();
  }
}

void BSP_TIM_MspInitCallback(TIM_HandleTypeDef *htim) {
  if (htim->Instance == TIM6) {
    __HAL_RCC_TIM6_CLK_ENABLE();
    HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
  }
}

void BSP_I2S_MspInitCallback(I2S_HandleTypeDef *hi2s) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if (hi2s->Instance == SPI1) {
    /* I2S1 clock enable */
    __HAL_RCC_SPI1_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**I2S1 GPIO Configuration
    PB5     ------> I2S1_SDO
    PA15 (JTDI)     ------> I2S1_WS
    PB3 (JTDO/TRACESWO)     ------> I2S1_CK
    */
    GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  }
}