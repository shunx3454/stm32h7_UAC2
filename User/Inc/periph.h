//
// Created by 61049 on 2024/9/28.
//

#ifndef _PERIPH_H
#define _PERIPH_H

#include "main.h"

/* Export varibales -------------------------------------- */
extern UART_HandleTypeDef g_uart1;
extern SDRAM_HandleTypeDef g_sdram1;
extern SD_HandleTypeDef g_sdio1;
extern I2C_HandleTypeDef g_i2c2;
extern DMA2D_HandleTypeDef g_dma2d;
extern TIM_HandleTypeDef g_tim6;
extern MDMA_HandleTypeDef g_mdma_sdmmc1;

void system_clock_print(void);
void sramx_init(void);
void gpio_init(void);
int sdram_init(void);
void uart_init(void);
int sdio_init(void);
void ldtc_init(uint32_t clr_fmt, uint32_t ltdc_gdram);
void i2c_init(void);
void dma2d_init(void);
void tim_init(void);

#endif //DEMO_PERIPH_H
