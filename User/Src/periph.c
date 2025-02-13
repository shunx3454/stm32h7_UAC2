#include "periph.h"
#include "FreeRTOS.h"
#include "semphr.h"

UART_HandleTypeDef g_uart1;
SDRAM_HandleTypeDef g_sdram1;
SD_HandleTypeDef g_sdio1;
LTDC_HandleTypeDef g_ltdc;
I2C_HandleTypeDef g_i2c2;
DMA2D_HandleTypeDef g_dma2d;
TIM_HandleTypeDef g_tim6;
MDMA_HandleTypeDef g_mdma_sdmmc1;

/* ========================================= */
void BSP_TIM_MspInitCallback(TIM_HandleTypeDef *htim);
void BSP_TIM_PeriodCpltCallback(TIM_HandleTypeDef *htim);
void BSP_SD_MspInitCallback(SD_HandleTypeDef *hsd);
void BSP_SD_TxCpltCallback(SD_HandleTypeDef *hsd);
void BSP_SD_RxCpltCallback(SD_HandleTypeDef *hsd);
void BSP_SD_ErrorCallback(SD_HandleTypeDef *hsd);
void BSP_MDMA_SDMMC_CpltCallback(MDMA_HandleTypeDef *hmdma);
void BSP_MDMA_SDMMC_ErrorCallback(MDMA_HandleTypeDef *hmdma);

/* ----------------------- function ----------------------- */

void sramx_init(void) {
  __HAL_RCC_D2SRAM1_CLK_ENABLE();
  __HAL_RCC_D2SRAM2_CLK_ENABLE();
  __HAL_RCC_D2SRAM3_CLK_ENABLE();
}

void system_clock_print(void) {
  PLL1_ClocksTypeDef PLL1;
  PLL2_ClocksTypeDef PLL2;
  PLL3_ClocksTypeDef PLL3;
  HAL_RCCEx_GetPLL1ClockFreq(&PLL1);
  HAL_RCCEx_GetPLL2ClockFreq(&PLL2);
  HAL_RCCEx_GetPLL3ClockFreq(&PLL3);
  printf("\r\n########### PLL Clock Frequency ###########\r\n");
  printf("PLL1 P: %ldMHz\r\n", PLL1.PLL1_P_Frequency / 1000000U);
  printf("PLL1 Q: %ldMHz\r\n", PLL1.PLL1_Q_Frequency / 1000000U);
  printf("PLL1 R: %ldMHz\r\n", PLL1.PLL1_R_Frequency / 1000000U);
  printf("PLL2 P: %ldMHz\r\n", PLL2.PLL2_P_Frequency / 1000000U);
  printf("PLL2 Q: %ldMHz\r\n", PLL2.PLL2_Q_Frequency / 1000000U);
  printf("PLL2 R: %ldMHz\r\n", PLL2.PLL2_R_Frequency / 1000000U);
  printf("PLL3 P: %ldMHz\r\n", PLL3.PLL3_P_Frequency / 1000000U);
  printf("PLL3 Q: %ldMHz\r\n", PLL3.PLL3_Q_Frequency / 1000000U);
  printf("PLL3 R: %ldMHz\r\n", PLL3.PLL3_R_Frequency / 1000000U);
  printf("\r\n");
}

void gpio_init(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* LED pin */
  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

  /* LCD_BL pin */
  __HAL_RCC_GPIOH_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = LCD_BL_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_BL_GPIO_Port, &GPIO_InitStruct);
}

void uart_init(void) {
  g_uart1.Instance = USART1;
  g_uart1.Init.BaudRate = 115200;
  g_uart1.Init.WordLength = UART_WORDLENGTH_8B;
  g_uart1.Init.StopBits = UART_STOPBITS_1;
  g_uart1.Init.Parity = UART_PARITY_NONE;
  g_uart1.Init.Mode = UART_MODE_TX_RX;
  g_uart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  g_uart1.Init.OverSampling = UART_OVERSAMPLING_16;
  g_uart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  g_uart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  g_uart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&g_uart1) != HAL_OK) {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&g_uart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK) {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&g_uart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK) {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&g_uart1) != HAL_OK) {
    Error_Handler();
  }
}

/* FMC initialization function */
int sdram_init(void) {
  FMC_SDRAM_TimingTypeDef SdramTiming = {0};

  /** Perform the SDRAM1 memory initialization sequence
   */
  g_sdram1.Instance = FMC_SDRAM_DEVICE;
  /* g_sdram1.Init */
  g_sdram1.Init.SDBank = FMC_SDRAM_BANK1;
  g_sdram1.Init.ColumnBitsNumber = FMC_SDRAM_COLUMN_BITS_NUM_8;
  g_sdram1.Init.RowBitsNumber = FMC_SDRAM_ROW_BITS_NUM_12;
  g_sdram1.Init.MemoryDataWidth = FMC_SDRAM_MEM_BUS_WIDTH_32;
  g_sdram1.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
  g_sdram1.Init.CASLatency = FMC_SDRAM_CAS_LATENCY_3;
  g_sdram1.Init.WriteProtection = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
  g_sdram1.Init.SDClockPeriod = FMC_SDRAM_CLOCK_PERIOD_2;
  g_sdram1.Init.ReadBurst = FMC_SDRAM_RBURST_ENABLE;
  g_sdram1.Init.ReadPipeDelay = FMC_SDRAM_RPIPE_DELAY_0;
  /* SdramTiming */
  SdramTiming.LoadToActiveDelay = 2;
  SdramTiming.ExitSelfRefreshDelay = 7;
  SdramTiming.SelfRefreshTime = 4;
  SdramTiming.RowCycleDelay = 7;
  SdramTiming.WriteRecoveryTime = 3;
  SdramTiming.RPDelay = 2;
  SdramTiming.RCDDelay = 2;

  if (HAL_SDRAM_Init(&g_sdram1, &SdramTiming) != HAL_OK) {
    return -1;
  } else {
    return 0;
  }
}

int sdio_init(void) {
  /* USER CODE END SDMMC1_Init 1 */
  g_sdio1.Instance = SDMMC1;
  g_sdio1.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
  g_sdio1.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  g_sdio1.Init.BusWide = SDMMC_BUS_WIDE_4B;
  g_sdio1.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
  g_sdio1.Init.ClockDiv = 3;
  g_sdio1.MspInitCallback = BSP_SD_MspInitCallback;
  if (HAL_SD_Init(&g_sdio1) == HAL_OK) {
    g_sdio1.TxCpltCallback = BSP_SD_TxCpltCallback;
    g_sdio1.RxCpltCallback = BSP_SD_RxCpltCallback;
    g_sdio1.ErrorCallback = BSP_SD_ErrorCallback;
    return 0;
  } else {
    SYS_ERR("HAL_SD_Init");
    return -1;
  }
}

void ldtc_init(uint32_t clr_fmt, uint32_t ltdc_gdram) {
  LTDC_LayerCfgTypeDef pLayerCfg = {0};

  g_ltdc.Instance = LTDC;
  g_ltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
  g_ltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
  g_ltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
  g_ltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
  g_ltdc.Init.HorizontalSync = 0;
  g_ltdc.Init.VerticalSync = 0;
  g_ltdc.Init.AccumulatedHBP = 80;
  g_ltdc.Init.AccumulatedVBP = 40;
  g_ltdc.Init.AccumulatedActiveW = 880;
  g_ltdc.Init.AccumulatedActiveH = 520;
  g_ltdc.Init.TotalWidth = 1080;
  g_ltdc.Init.TotalHeigh = 542;
  g_ltdc.Init.Backcolor.Blue = 0;
  g_ltdc.Init.Backcolor.Green = 0;
  g_ltdc.Init.Backcolor.Red = 0;
  if (HAL_LTDC_Init(&g_ltdc) != HAL_OK) {
    Error_Handler();
  }

  pLayerCfg.WindowX0 = 0;
  pLayerCfg.WindowX1 = 800;
  pLayerCfg.WindowY0 = 0;
  pLayerCfg.WindowY1 = 480;
  pLayerCfg.PixelFormat = clr_fmt;
  pLayerCfg.Alpha = 255;
  pLayerCfg.Alpha0 = 0;
  pLayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
  pLayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
  pLayerCfg.FBStartAdress = ltdc_gdram;
  pLayerCfg.ImageWidth = 800;
  pLayerCfg.ImageHeight = 480;
  pLayerCfg.Backcolor.Blue = 0;
  pLayerCfg.Backcolor.Green = 0;
  pLayerCfg.Backcolor.Red = 0;
  if (HAL_LTDC_ConfigLayer(&g_ltdc, &pLayerCfg, LTDC_LAYER_1) != HAL_OK) {
    Error_Handler();
  }
}

void i2c_init(void) {
  g_i2c2.Instance = I2C2;
  g_i2c2.Init.Timing = 0x10B01D5E;
  g_i2c2.Init.OwnAddress1 = 0;
  g_i2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  g_i2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  g_i2c2.Init.OwnAddress2 = 0;
  g_i2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  g_i2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  g_i2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&g_i2c2) != HAL_OK) {
    Error_Handler();
  }
  if (HAL_I2CEx_ConfigAnalogFilter(&g_i2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK) {
    Error_Handler();
  }
  if (HAL_I2CEx_ConfigDigitalFilter(&g_i2c2, 5) != HAL_OK) {
    Error_Handler();
  }
}

void dma2d_init(void) {
  __HAL_RCC_DMA2D_CLK_ENABLE();
}

void tim_init(void) {
  g_tim6.Instance = TIM6;
  g_tim6.Init.Prescaler = 240 - 1;
  g_tim6.Init.Period = 1000 - 1;
  g_tim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  g_tim6.Base_MspInitCallback = BSP_TIM_MspInitCallback;
  HAL_TIM_Base_Init(&g_tim6);
  // HAL_TIM_Base_Init会清除所有回调并设为HAL库默认回调， Base_MspInitCallback除外
  g_tim6.PeriodElapsedCallback = BSP_TIM_PeriodCpltCallback;
}

void usb_dc_low_level_init(uint8_t busid) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /** Enable USB Voltage detector
   */
  HAL_PWREx_EnableUSBVoltageDetector();

  __HAL_RCC_GPIOA_CLK_ENABLE();
  /**USB_OTG_FS GPIO Configuration
  PA12     ------> USB_OTG_FS_DP
  PA11     ------> USB_OTG_FS_DM
  */
  GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF10_OTG1_FS;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* Peripheral clock enable */
  __HAL_RCC_USB_OTG_FS_CLK_ENABLE();

  /* Peripheral interrupt init */
  HAL_NVIC_SetPriority(OTG_FS_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(OTG_FS_IRQn);
}

void usb_dc_low_level_deinit(uint8_t busid) {
  HAL_GPIO_DeInit(GPIOA, GPIO_PIN_12 | GPIO_PIN_11);
  __HAL_RCC_USB_OTG_FS_CLK_DISABLE();
  HAL_NVIC_DisableIRQ(OTG_FS_IRQn);
}