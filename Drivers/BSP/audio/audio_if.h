#ifndef __BSP_AUDIO_IF_H_
#define __BSP_AUDIO_IF_H_

#include "main.h"

extern I2S_HandleTypeDef g_i2s1;
extern DMA_HandleTypeDef g_dma_i2s1;

enum I2S_MODE { I2S_ASYNC_MODE, I2S_SYNC_MODE };

void i2s_config_default(I2S_InitTypeDef *i2s_config);
void i2s_dma_config_default(DMA_InitTypeDef *dma_config);
void i2s_init(I2S_InitTypeDef *i2s_config, DMA_InitTypeDef *dma_config, enum I2S_MODE I2sMode);
int i2s_start(void);
int i2s_send_async(uint8_t *buf, uint32_t size);
int i2s_send_sync(uint8_t *buf, uint32_t size);

#endif