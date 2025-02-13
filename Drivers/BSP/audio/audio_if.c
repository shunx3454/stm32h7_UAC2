#include "audio_if.h"
#include "FreeRTOS.h"
#include "bord_config.h"
#include "queue.h"
#include "task.h"
#include <string.h>

#define I2S_QUEUE_LENGTH 16U
#define I2S_QUEUE_ITEM_SIZE 192U

enum I2S_QUEUE_STATUS {
  I2S_QUEUE_RESET,
  I2S_QUEUE_READY,
  I2S_QUEUE_POLLING,
  I2S_QUEUE_PAUSE,
  I2S_QUEUE_ERROR,
  I2S_QUEUE_BUSY
};

I2S_HandleTypeDef g_i2s1;
DMA_HandleTypeDef g_dma_i2s1;
TaskHandle_t TaskI2sLoop = NULL;
QueueHandle_t Queue_I2S = NULL;
StaticQueue_t xStaticI2sQueue;
__IO int I2sQueueState = I2S_QUEUE_RESET;
int i2s_mode = I2S_SYNC_MODE;
int buf_reserve = 0;

char I2sQueueStorageArea[I2S_QUEUE_LENGTH * I2S_QUEUE_ITEM_SIZE] __ALIGNED(4) __ATTR_SDRAM;
char i2s_buf0[I2S_QUEUE_ITEM_SIZE] __ALIGNED(4) __ATTR_SDRAM;
char i2s_buf1[I2S_QUEUE_ITEM_SIZE] __ALIGNED(4) __ATTR_SDRAM;
char i2s_buf2[I2S_QUEUE_ITEM_SIZE] __ALIGNED(4) __ATTR_SDRAM;

void thread_i2s_loop(void *param);
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

void i2s_init(I2S_InitTypeDef *i2s_config, DMA_InitTypeDef *dma_config, enum I2S_MODE I2sMode) {
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

  i2s_mode = I2sMode;

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
  if (i2s_mode == I2S_ASYNC_MODE) {
    HAL_DMA_RegisterCallback(&g_dma_i2s1, HAL_DMA_XFER_CPLT_CB_ID, i2s_dma_buf0_cplt);
    HAL_DMA_RegisterCallback(&g_dma_i2s1, HAL_DMA_XFER_M1CPLT_CB_ID, i2s_dma_buf1_cplt);
    HAL_DMA_RegisterCallback(&g_dma_i2s1, HAL_DMA_XFER_ERROR_CB_ID, i2s_dma_error);
  }
}

int i2s_start(void) {
  if (i2s_mode == I2S_SYNC_MODE)
    return 0;

  // Create queue
  if (Queue_I2S == NULL) {
    Queue_I2S = xQueueCreateStatic(I2S_QUEUE_LENGTH, I2S_QUEUE_ITEM_SIZE, I2sQueueStorageArea, &xStaticI2sQueue);
    if (Queue_I2S == NULL) {
      return -1;
    }
  }

  // Create i2s task
  if (TaskI2sLoop == NULL) {
    if (xTaskCreate(thread_i2s_loop, "i2s_loop", 512, NULL, 5, &TaskI2sLoop) != pdPASS || TaskI2sLoop == NULL) {
      return -2;
    }
  }

  // start dma double buffer
  if (HAL_I2S_Transmit_DMAEx_MultiBuffer(&g_i2s1, i2s_buf0, i2s_buf1, I2S_QUEUE_ITEM_SIZE / 2) != HAL_OK) {
    return -3;
  }
  return 0;
}

void thread_i2s_loop(void *param) {
  for (;;) {
    if (uxQueueMessagesWaiting(Queue_I2S) > 1 && (I2sQueueState == I2S_QUEUE_PAUSE)) {
      I2sQueueState = I2S_QUEUE_BUSY;
      printf("i2s resume\r\n");
      HAL_I2S_DMAResume(&g_i2s1);
    } else if (I2sQueueState == I2S_QUEUE_ERROR) {
      HAL_I2S_DMAPause(&g_i2s1);
      printf("i2s error\r\n");
      vTaskDelay(1000);
    }
    vTaskDelay(5);
  }
}

int i2s_send_sync(uint8_t *buf, uint32_t size) {
  HAL_StatusTypeDef state;
  if (i2s_mode == I2S_ASYNC_MODE) {
    return -1;
  }

  if ((state = HAL_I2S_Transmit_DMA(&g_i2s1, (const uint16_t *)buf, size / 2)) != HAL_OK) {
    printf("HAL_I2S_Transmit error: %d\r\n", state);
    return -1;
  }
  return 0;
}

int i2s_send_async(uint8_t *buf, uint32_t size) {
  int errorcode = 0;

  if (i2s_mode == I2S_SYNC_MODE) {
    return -1;
  }

  do {
    if (I2sQueueState == I2S_QUEUE_RESET) {
      errorcode = 1;
      break;
    }

    // data buf full
    if (buf_reserve + size >= I2S_QUEUE_ITEM_SIZE) {

      // fill the i2s_queue_data_backup_buf
      memcpy(i2s_buf2, buf, I2S_QUEUE_ITEM_SIZE - buf_reserve);

      // send the i2s_queue_data_backup_buf data to queue
      if (xPortIsInsideInterrupt() == pdTRUE) {
        if (xQueueSendFromISR(Queue_I2S, i2s_buf2, NULL) != pdTRUE) {
          errorcode = 2;
          break;
        }
      } else {
        if (xQueueSend(Queue_I2S, i2s_buf2, portMAX_DELAY) != pdTRUE) {
          errorcode = 2;
          break;
        }
      }

      // modify the buffer ptr and left bytes
      buf += (I2S_QUEUE_ITEM_SIZE - buf_reserve);
      size -= (I2S_QUEUE_ITEM_SIZE - buf_reserve);

      // if data enough
      if (size >= I2S_QUEUE_ITEM_SIZE) {
        // div pack
        uint16_t BufferTimes = size / I2S_QUEUE_ITEM_SIZE;

        while (BufferTimes--) {

          // send the i2s_queue_data_backup_buf data
          if (xPortIsInsideInterrupt() == pdTRUE) {
            if (xQueueSendFromISR(Queue_I2S, buf, NULL) != pdTRUE) {
              errorcode = 4;
              break;
            }
          } else {
            if (xQueueSend(Queue_I2S, buf, portMAX_DELAY) != pdTRUE) {
              errorcode = 4;
              break;
            }
          }

          // modify address
          buf += I2S_QUEUE_ITEM_SIZE;
          size -= I2S_QUEUE_ITEM_SIZE;
        }
        if (errorcode)
          break;
      } // left size cannot file buf

      // reserve the left data
      memcpy(i2s_buf2, buf, size);
      buf_reserve = size;
    } else {
      memcpy(i2s_buf2 + buf_reserve, buf, size);
      buf_reserve += size;
    }
  } while (0);

  // report the errorcode
  if (errorcode)
    printf("i2s send buf error:%d\r\n", errorcode);
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

  // receive data from queue
  if (xQueueReceiveFromISR(Queue_I2S, i2s_buf0, NULL) == pdTRUE) {
    I2sQueueState = I2S_QUEUE_BUSY;
  } else {
    memset(i2s_buf0, 0, I2S_QUEUE_ITEM_SIZE);
    if (I2sQueueState == I2S_QUEUE_POLLING) {
      HAL_I2S_DMAPause(&g_i2s1);
      printf("i2s pause\r\n");
      I2sQueueState = I2S_QUEUE_PAUSE;
    } else {
      I2sQueueState = I2S_QUEUE_POLLING;
    }
  }
  SCB_CleanDCache_by_Addr((uint32_t *)i2s_buf0, I2S_QUEUE_ITEM_SIZE);
}
void i2s_dma_buf1_cplt(DMA_HandleTypeDef *hdma) {
  // printf("i2s m1 cplt\r\n");
  
  // receive data from queue
  if (xQueueReceiveFromISR(Queue_I2S, i2s_buf1, NULL) == pdTRUE) {
    I2sQueueState = I2S_QUEUE_BUSY;
  } else {
    memset(i2s_buf1, 0, I2S_QUEUE_ITEM_SIZE);
    if (I2sQueueState == I2S_QUEUE_POLLING) {
      HAL_I2S_DMAPause(&g_i2s1);
      printf("i2s pause\r\n");
      I2sQueueState = I2S_QUEUE_PAUSE;
    } else {
      I2sQueueState = I2S_QUEUE_POLLING;
    }
  }
  SCB_CleanDCache_by_Addr((uint32_t *)i2s_buf1, I2S_QUEUE_ITEM_SIZE);
}
void i2s_dma_error(DMA_HandleTypeDef *hdma) {
  I2sQueueState = I2S_QUEUE_ERROR;
}