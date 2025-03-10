#include "audio_if.h"

#include <string.h>

#include "FreeRTOS.h"
#include "stream_buffer.h"
#include "task.h"

#include "bord_config.h"
#include "periph.h"

#define I2S_STREAMBUFFER_SIZE (32 * 1024UL)
#define I2S_DMA_BUFFER_MAX_SIZE (4096UL)

uint8_t i2s_static_stream_bufffer_area[I2S_STREAMBUFFER_SIZE] __ATTR_SDRAM __ALIGNED(4);
uint8_t i2s_dma_buf0[I2S_DMA_BUFFER_MAX_SIZE] __ATTR_SDRAM __ALIGNED(4);
uint8_t i2s_dma_buf1[I2S_DMA_BUFFER_MAX_SIZE] __ATTR_SDRAM __ALIGNED(4);

I2S_HandleTypeDef g_i2s1;
DMA_HandleTypeDef g_dma_i2s1;
StaticStreamBuffer_t i2s_static_stream_buffer;
StreamBufferHandle_t i2s_stream_buffer_handle;
__IO uint16_t rx_stream_not_enough = 0;
__IO uint16_t i2s_dma_trans_size = 0;

void BSP_I2S_MspInitCallback(I2S_HandleTypeDef *hi2s);
void i2s_dma_buf0_cplt(DMA_HandleTypeDef *hdma);
void i2s_dma_buf1_cplt(DMA_HandleTypeDef *hdma);
void i2s_dma_error(DMA_HandleTypeDef *hdma);
HAL_StatusTypeDef HAL_I2S_Transmit_DMAEx_MultiBuffer(I2S_HandleTypeDef *hi2s, uint8_t *pbuf0, uint8_t *pbuf1,
                                                     uint32_t Size);

void i2s_config_default(I2S_InitTypeDef *i2s_config) {
  i2s_config->Mode = I2S_MODE_MASTER_TX;
  i2s_config->Standard = I2S_STANDARD_PHILIPS;
  i2s_config->DataFormat = I2S_DATAFORMAT_16B;
  i2s_config->MCLKOutput = I2S_MCLKOUTPUT_DISABLE;
  i2s_config->AudioFreq = I2S_AUDIOFREQ_44K;
  i2s_config->CPOL = I2S_CPOL_LOW;
  i2s_config->FirstBit = I2S_FIRSTBIT_MSB;
  i2s_config->WSInversion = I2S_WS_INVERSION_DISABLE;
  i2s_config->Data24BitAlignment = I2S_DATA_24BIT_ALIGNMENT_RIGHT;
  i2s_config->MasterKeepIOState = I2S_MASTER_KEEP_IO_STATE_DISABLE;
}

void i2s_dma_config_default(DMA_InitTypeDef *dma_config) {
  dma_config->Request = DMA_REQUEST_SPI1_TX;
  dma_config->Direction = DMA_MEMORY_TO_PERIPH;
  dma_config->PeriphInc = DMA_PINC_DISABLE;
  dma_config->MemInc = DMA_MINC_ENABLE;
  dma_config->PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
  dma_config->MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
  dma_config->Mode = DMA_CIRCULAR;
  dma_config->Priority = DMA_PRIORITY_HIGH;
  dma_config->FIFOMode = DMA_FIFOMODE_ENABLE;
  dma_config->FIFOThreshold = DMA_FIFO_THRESHOLD_HALFFULL;
  dma_config->MemBurst = DMA_MBURST_SINGLE;
  dma_config->PeriphBurst = DMA_MBURST_SINGLE;
}

void i2s_init(I2S_InitTypeDef *i2s_config, DMA_InitTypeDef *dma_config) {
  g_i2s1.Instance = SPI1;
  g_i2s1.Init.Mode = i2s_config->Mode;
  g_i2s1.Init.Standard = i2s_config->Standard;
  g_i2s1.Init.DataFormat = i2s_config->DataFormat;
  g_i2s1.Init.MCLKOutput = i2s_config->MCLKOutput;
  g_i2s1.Init.AudioFreq = i2s_config->AudioFreq;
  g_i2s1.Init.CPOL = i2s_config->CPOL;
  g_i2s1.Init.FirstBit = i2s_config->FirstBit;
  g_i2s1.Init.WSInversion = i2s_config->WSInversion;
  g_i2s1.Init.Data24BitAlignment = i2s_config->Data24BitAlignment;
  g_i2s1.Init.MasterKeepIOState = i2s_config->MasterKeepIOState;
  g_i2s1.MspInitCallback = BSP_I2S_MspInitCallback;
  HAL_I2S_DeInit(&g_i2s1);
  if (HAL_I2S_Init(&g_i2s1) != HAL_OK) {
    Error_Handler();
  }

  /* I2S1 DMA Init */
  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* SPI1_TX Init */
  g_dma_i2s1.Instance = DMA1_Stream0;
  g_dma_i2s1.Init.Request = dma_config->Request;
  g_dma_i2s1.Init.Direction = dma_config->Direction;
  g_dma_i2s1.Init.PeriphInc = dma_config->PeriphInc;
  g_dma_i2s1.Init.MemInc = dma_config->MemInc;
  g_dma_i2s1.Init.PeriphDataAlignment = dma_config->PeriphDataAlignment;
  g_dma_i2s1.Init.MemDataAlignment = dma_config->MemDataAlignment;
  g_dma_i2s1.Init.Mode = dma_config->Mode;
  g_dma_i2s1.Init.Priority = dma_config->Priority;
  g_dma_i2s1.Init.FIFOMode = dma_config->FIFOMode;
  g_dma_i2s1.Init.FIFOThreshold = dma_config->FIFOThreshold;
  g_dma_i2s1.Init.MemBurst = dma_config->MemBurst;
  g_dma_i2s1.Init.PeriphBurst = dma_config->PeriphBurst;
  HAL_DMA_DeInit(&g_dma_i2s1);
  if (HAL_DMA_Init(&g_dma_i2s1) != HAL_OK) {
    Error_Handler();
  }

  __HAL_LINKDMA(&g_i2s1, hdmatx, g_dma_i2s1);

  /* DMA interrupt init */
  /* DMA1_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 6, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);

  /* Regist Callback */
  HAL_DMA_RegisterCallback(&g_dma_i2s1, HAL_DMA_XFER_CPLT_CB_ID, i2s_dma_buf0_cplt);
  HAL_DMA_RegisterCallback(&g_dma_i2s1, HAL_DMA_XFER_M1CPLT_CB_ID, i2s_dma_buf1_cplt);
  HAL_DMA_RegisterCallback(&g_dma_i2s1, HAL_DMA_XFER_ERROR_CB_ID, i2s_dma_error);
}

int i2s_start(uint32_t i2s_dma_threshold) {
  int errorcode = 0;
  do {
    if (i2s_dma_threshold > I2S_DMA_BUFFER_MAX_SIZE) {
      i2s_dma_trans_size = I2S_DMA_BUFFER_MAX_SIZE;
    } else {
      i2s_dma_trans_size = i2s_dma_threshold;
    }

    // Create Stream buffer
    i2s_stream_buffer_handle = xStreamBufferCreateStatic(I2S_STREAMBUFFER_SIZE, i2s_dma_trans_size,
                                                         i2s_static_stream_bufffer_area, &i2s_static_stream_buffer);
    if (i2s_stream_buffer_handle == NULL) {
      errorcode = 1;
      break;
    }

    // mem init
    memset(i2s_static_stream_bufffer_area, 0, I2S_STREAMBUFFER_SIZE);
    memset(i2s_dma_buf0, 0, I2S_DMA_BUFFER_MAX_SIZE);
    memset(i2s_dma_buf1, 0, I2S_DMA_BUFFER_MAX_SIZE);

    // start i2s dma double buffer
    if (HAL_I2S_Transmit_DMAEx_MultiBuffer(&g_i2s1, i2s_dma_buf0, i2s_dma_buf1, i2s_dma_trans_size / 2) != HAL_OK) {
      errorcode = 3;
      break;
    }

  } while (0);
  if (errorcode)
    printf("i2s_start errorcode: %d\r\n", errorcode);

  return errorcode;
}

int i2s_pause() {
  if (HAL_I2S_DMAPause(&g_i2s1) != HAL_OK) {
    return -1;
    printf("HAL_I2S_DMAPause error\r\n");
  } else {
    return 0;
  }
}

int i2s_resume() {
  if (HAL_I2S_DMAResume(&g_i2s1) != HAL_OK) {
    printf("HAL_I2S_DMAResume error\r\n");
    return -1;
  } else {
    return 0;
  }
}

int i2s_send_buf(uint8_t *buf, uint32_t size) {
  int errorcode = 0;
  do {

    if (xPortIsInsideInterrupt() == pdTRUE) {
      size_t nTx = xStreamBufferSendFromISR(i2s_stream_buffer_handle, buf, size, NULL);
      if (nTx != size) {
        printf("xStreamBufferSendFromISR error\r\n");
        errorcode = 1;
        break;
      }
    } else {
      size_t nTx = xStreamBufferSend(i2s_stream_buffer_handle, buf, size, portMAX_DELAY);
      if (nTx != size) {
        printf("xStreamBufferSend error\r\n");
        errorcode = 2;
        break;
      }
    }

  } while (0);

  // report the errorcode
  return errorcode;
}

HAL_StatusTypeDef HAL_I2S_Transmit_DMAEx_MultiBuffer(I2S_HandleTypeDef *hi2s, uint8_t *pbuf0, uint8_t *pbuf1,
                                                     uint32_t Size) {
  HAL_StatusTypeDef errorcode = HAL_OK;

  if ((pbuf0 == NULL) || (pbuf1 == NULL) || (Size == 0UL)) {
    return HAL_ERROR;
  }

  if (hi2s->State != HAL_I2S_STATE_READY) {
    return HAL_BUSY;
  }

  /* Process Locked */
  __HAL_LOCK(hi2s);

  /* Set state and reset error code */
  hi2s->State = HAL_I2S_STATE_BUSY_TX;
  hi2s->ErrorCode = HAL_I2S_ERROR_NONE;
  hi2s->pTxBuffPtr = (const uint16_t *)pbuf0;
  hi2s->TxXferSize = Size;
  hi2s->TxXferCount = Size;

  /* Init field not used in handle to zero */
  hi2s->pRxBuffPtr = NULL;
  hi2s->RxXferSize = (uint16_t)0UL;
  hi2s->RxXferCount = (uint16_t)0UL;

  /* Set the I2S Tx DMA M0/M1/Error transfer complete callback */
  if (hi2s->hdmatx->XferM1CpltCallback == NULL || hi2s->hdmatx->XferCpltCallback == NULL ||
      hi2s->hdmatx->XferErrorCallback == NULL) {
    SET_BIT(hi2s->ErrorCode, HAL_I2S_ERROR_INVALID_CALLBACK);
    hi2s->State = HAL_I2S_STATE_READY;

    __HAL_UNLOCK(hi2s);
    errorcode = HAL_ERROR;
    return errorcode;
  }

  /* Enable the Tx DMA Stream/Channel */
  if (HAL_OK != HAL_DMAEx_MultiBufferStart_IT(hi2s->hdmatx, (uint32_t)pbuf0, (uint32_t)&hi2s->Instance->TXDR,
                                              (uint32_t)pbuf1, hi2s->TxXferCount)) {
    /* Update I2S error code */
    SET_BIT(hi2s->ErrorCode, HAL_I2S_ERROR_DMA);
    hi2s->State = HAL_I2S_STATE_READY;

    __HAL_UNLOCK(hi2s);
    errorcode = HAL_ERROR;
    return errorcode;
  }

  /* Check if the I2S Tx request is already enabled */
  if (HAL_IS_BIT_CLR(hi2s->Instance->CFG1, SPI_CFG1_TXDMAEN)) {
    /* Enable Tx DMA Request */
    SET_BIT(hi2s->Instance->CFG1, SPI_CFG1_TXDMAEN);
  }

  /* Check if the I2S is already enabled */
  if (HAL_IS_BIT_CLR(hi2s->Instance->CR1, SPI_CR1_SPE)) {
    /* Enable I2S peripheral */
    __HAL_I2S_ENABLE(hi2s);
  }

  /* Start the transfer */
  SET_BIT(hi2s->Instance->CR1, SPI_CR1_CSTART);

  __HAL_UNLOCK(hi2s);
  return errorcode;
}

void i2s_dma_buf0_cplt(DMA_HandleTypeDef *hdma) {
  // printf("i2s m0 cplt\r\n");
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  if (xStreamBufferBytesAvailable(i2s_stream_buffer_handle) >= i2s_dma_trans_size) {
    xStreamBufferReceiveFromISR(i2s_stream_buffer_handle, i2s_dma_buf0, i2s_dma_trans_size, &xHigherPriorityTaskWoken);
    SCB_CleanDCache_by_Addr((uint32_t *)i2s_dma_buf0, i2s_dma_trans_size);
  } else {
    // memset (i2s_dma_buf0, 0, i2s_dma_trans_size);
    rx_stream_not_enough = 1;
  }

  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
void i2s_dma_buf1_cplt(DMA_HandleTypeDef *hdma) {
  // printf("i2s m1 cplt\r\n");
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  if (xStreamBufferBytesAvailable(i2s_stream_buffer_handle) >= i2s_dma_trans_size) {
    xStreamBufferReceiveFromISR(i2s_stream_buffer_handle, i2s_dma_buf1, i2s_dma_trans_size, &xHigherPriorityTaskWoken);
    SCB_CleanDCache_by_Addr((uint32_t *)i2s_dma_buf1, i2s_dma_trans_size);
  } else {
    // memset (i2s_dma_buf1, 0, i2s_dma_trans_size);
    rx_stream_not_enough = 2;
  }

  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
void i2s_dma_error(DMA_HandleTypeDef *hdma) {}