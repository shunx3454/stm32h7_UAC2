#ifndef __BSP_AUDIO_IF_H_
#define __BSP_AUDIO_IF_H_

#include "main.h"

extern I2S_HandleTypeDef g_i2s1;
extern DMA_HandleTypeDef g_dma_i2s1;
extern __IO uint16_t rx_stream_not_enough;

void i2s_config_default(I2S_InitTypeDef *i2s_config);
void i2s_dma_config_default(DMA_InitTypeDef *dma_config);
void i2s_init (I2S_InitTypeDef *i2s_config, DMA_InitTypeDef *dma_config);
int i2s_start (uint32_t i2s_dma_threshold);
int i2s_send_buf(uint8_t *buf, uint32_t size);

#endif