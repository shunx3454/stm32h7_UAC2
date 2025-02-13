//
// Created by ShunX on 2023/12/31.
//
#include "sd_app.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>

/** sd_dma trans flag */
#define SD_RW_TIMEOUT 0xffffff
#define TESTDATA ((uint8_t)(0x88))
#define SDIO_BLOCK_SIZE 512U

__IO static uint8_t SDTxCplt;
__IO static uint8_t SDRxCplt;
__IO static uint8_t SDError;

#if (SDIO_RW_TEST)
#define SD_TEST_BUFF_SIZE (uint32_t)(3 * 1024)
static uint8_t sd_buff[SD_TEST_BUFF_SIZE] __ATTR_SDRAM __ALIGNED(4);
#endif

#if SDIO_USE_DMA
#define SDIO_DMA_ALIGN4_BUF_SIZE (uint32_t)(1024 * 1024)
#define SDIO_DMA_ALIGN4_BUF_BLOCKS (SDIO_DMA_ALIGN4_BUF_SIZE / SDIO_BLOCK_SIZE)
#ifdef LINKSCRIP_EXSRAM
static uint8_t align4_buf[SDIO_DMA_ALIGN4_BUF_SIZE] __ALIGNED(4) __ATTR_SDRAM;
#else
static uint8_t align4_buf[SDIO_DMA_ALIGN4_BUF_SIZE] __ALIGNED(4);
#endif
#endif

int MemoryCompare(char *ptr, char val, unsigned int size) {
  while (size--) {
    if (*ptr++ != val)
      return -1;
  }
  return 0;
}

/**
 *   @brief 检测SD卡
 *   @retval 0:存在 -1：不存在
 *   @attention 检测引脚初始化时，需要下拉输入，读出高电平为检测到MSD卡
 */
int8_t SD_Detect(void) {
  return 0;
}
/**
 *  @brief 读sd卡
 *  @param pBuff: 数据地址
 *  @param Block_addr: 块地址（一般每块512B）
 *  @param Blocks_Nbr：块数量
 *  @retval 0:成功
 */
#if 0
int8_t SD_Read_Disk(uint8_t *pBuff, uint32_t Block_addr, uint32_t Blocks_Nbr) {
  uint32_t timeout = SD_RW_TIMEOUT;
  while (HAL_SD_GetCardState(&SD_HANDLE) != HAL_SD_CARD_TRANSFER) {
    timeout--;
    if (timeout == 0) {
      return 1;
    }
  }
  sd_rx_cplt = 0;
#if !SDIO_USE_DMA
  if (HAL_SD_ReadBlocks_IT(&SD_HANDLE, pBuff, Block_addr, Blocks_Nbr) != HAL_OK)
    return 2;
  while (sd_rx_cplt != 1 && sd_error != 1) {
  }
  if (sd_error) {
    sd_error = 0;
    printf("sd error\r\n");
    return -1;
  } else
    return 0;
#else
  if ((uint32_t)pBuff % 4) {
    { /* dma trans need align 4 byte */
      // SYS_LOG("SD Read", "NO Align pBuff");
      int8_t error = 0;
      uint16_t times = Blocks_Nbr / SDIO_DMA_ALIGN4_BUF_BLOCKS;
      uint16_t remain_blocks = Blocks_Nbr % SDIO_DMA_ALIGN4_BUF_BLOCKS;
      uint16_t trans_blocks;
      for (uint16_t i = 0; i < times + 1; i++) {
        if (i != times)
          trans_blocks = SDIO_DMA_ALIGN4_BUF_BLOCKS;
        else
          trans_blocks = remain_blocks;

        /* read to align buf : blocks*/
        if (HAL_SD_ReadBlocks_DMA(&SD_HANDLE, align4_buf, Block_addr + i * SDIO_DMA_ALIGN4_BUF_BLOCKS, trans_blocks) !=
            HAL_OK) {
          SYS_ERR("HAL_SD_ReadBlocks_DMA");
          error = 2;
          break;
        }
        /* check trans complete */
        while (sd_rx_cplt != 1 && sd_error != 1) {
        }
        if (sd_error) {
          sd_error = 0;
          SYS_ERR("sd_error");
          error = 1;
          break;
        }

        /* copy aligned buf to pbuf: bytes*/
        memcpy(pBuff + i * (SDIO_DMA_ALIGN4_BUF_SIZE), align4_buf, trans_blocks * SDIO_BLOCK_SIZE);
      }
      return error;
    }
  } else {
    if (HAL_SD_ReadBlocks_DMA(&SD_HANDLE, pBuff, Block_addr, Blocks_Nbr) != HAL_OK)
      return -2;

    /* check trans complete */
    while (sd_rx_cplt != 1 && sd_error != 1) {
    }
    if (sd_error) {
      sd_error = 0;
      SYS_ERR("sd_error");
      return -2;
    } else {
      return 0;
    }
  }
#endif
}
#else
int SD_Read_Disk(uint8_t *pBuff, uint32_t Block_addr, uint32_t Blocks_Nbr) {
  uint32_t timeout = SD_RW_TIMEOUT;
  int errorcode = 0;
  do {
    uint16_t BufferTimes = Blocks_Nbr / SDIO_DMA_ALIGN4_BUF_BLOCKS;
    uint32_t LeftBlocks = Blocks_Nbr % SDIO_DMA_ALIGN4_BUF_BLOCKS;
    
    while (BufferTimes--) {
      // wait sd card enter trans state
      while (HAL_SD_GetCardState(&SD_HANDLE) != HAL_SD_CARD_TRANSFER) {
        timeout--;
        if (timeout == 0) {
          errorcode = 1;
          break;
        }
      }
      if (errorcode)
        break;

      SDRxCplt = 0;
      SDError = 0;
      if (HAL_SD_ReadBlocks_DMA(&SD_HANDLE, align4_buf, Block_addr, SDIO_DMA_ALIGN4_BUF_BLOCKS) != HAL_OK) {
        errorcode = 2;
        break;
      }

      /* check trans complete */
      while (!SDRxCplt && !SDError) {
      }
      if (SDError) {
        errorcode = 3;
        break;
      }

      /* Trans ok */
      memcpy(pBuff, align4_buf, SDIO_DMA_ALIGN4_BUF_SIZE);

      /* Modify pointer and address */
      pBuff += SDIO_DMA_ALIGN4_BUF_SIZE;
      Block_addr += SDIO_DMA_ALIGN4_BUF_BLOCKS;
    }
    if (errorcode)
      break;

    /* Trans Left blocks */
    if (LeftBlocks) {
      // wait sd card enter trans state
      while (HAL_SD_GetCardState(&SD_HANDLE) != HAL_SD_CARD_TRANSFER) {
        timeout--;
        if (timeout == 0) {
          errorcode = 1;
          break;
        }
      }
      if (errorcode)
        break;

      SDRxCplt = 0;
      SDError = 0;
      if (HAL_SD_ReadBlocks_DMA(&SD_HANDLE, align4_buf, Block_addr, LeftBlocks) != HAL_OK) {
        errorcode = 4;
        break;
      }

      /* check trans complete */
      while (!SDRxCplt && !SDError) {
      }
      if (SDError) {
        errorcode = 5;
        break;
      }
      /* Trans ok */
      memcpy(pBuff, align4_buf, LeftBlocks * SDIO_BLOCK_SIZE);
    }
  } while (0);

  if (errorcode)
    printf("SD_Read_Disk ErrorCode: %d\r\n", errorcode);
  return errorcode;
}

#endif

/**
 *  @brief 写sd卡
 *  @param pBuff: 数据地址
 *  @param Block_addr: 块地址（一般每块512B）
 *  @param Blocks_Nbr：块数量
 *  @retval 0:成功
 */
#if 0
int8_t SD_Write_Disk(uint8_t *pBuff, uint32_t Block_addr, uint32_t Blocks_Nbr) {
  uint32_t timeout = SD_RW_TIMEOUT;
  while (HAL_SD_GetCardState(&SD_HANDLE) != HAL_SD_CARD_TRANSFER) {
    timeout--;
    if (timeout == 0) {
      return 1;
    }
  }
  sd_tx_cplt = 0;
#if !SDIO_USE_DMA
  if (HAL_SD_WriteBlocks_IT(&SD_HANDLE, pBuff, Block_addr, Blocks_Nbr) != HAL_OK)
    return 2;
  while (sd_tx_cplt != 1 && sd_error != 1) {
  }
  if (sd_error) {
    sd_error = 0;
    printf("sd error\r\n");
    return -1;
  } else
    return 0;
#else
  if ((uint32_t)pBuff % 4) {
    { /* dma trans need align 4 byte */
      // SYS_LOG("SD Write", "NO Align pBuff");
      int8_t error = 0;
      uint16_t times = Blocks_Nbr / SDIO_DMA_ALIGN4_BUF_BLOCKS;
      uint16_t remain_blocks = Blocks_Nbr % SDIO_DMA_ALIGN4_BUF_BLOCKS;
      uint16_t trans_blocks;
      for (uint16_t i = 0; i < times + 1; i++) {
        if (i != times)
          trans_blocks = SDIO_DMA_ALIGN4_BUF_BLOCKS;
        else
          trans_blocks = remain_blocks;

        /* copy pbuf mem to aligned buf : bytes*/
        memcpy(align4_buf, pBuff + i * (SDIO_DMA_ALIGN4_BUF_SIZE), trans_blocks * SDIO_BLOCK_SIZE);

        /* write align mem to addr: blocks*/
        if (HAL_SD_WriteBlocks_DMA(&SD_HANDLE, align4_buf, Block_addr + i * SDIO_DMA_ALIGN4_BUF_BLOCKS, trans_blocks) !=
            HAL_OK) {
          SYS_ERR("HAL_SD_WriteBlocks_DMA");
          error = 2;
          break;
        }
        /* check trans complete */
        while (sd_tx_cplt != 1 && sd_error != 1) {
        }
        if (sd_error) {
          sd_error = 0;
          SYS_ERR("sd_error");
          error = 1;
          break;
        }
      }
      return error;
    }
  } else {
    if (HAL_SD_WriteBlocks_DMA(&SD_HANDLE, pBuff, Block_addr, Blocks_Nbr) != HAL_OK)
      return 2;
    /* check trans complete */
    while (sd_tx_cplt != 1 && sd_error != 1) {
    }
    if (sd_error) {
      sd_error = 0;
      SYS_ERR("sd_error");
      return -2;
    } else {
      return 0;
    }
  }
#endif
}
#else
int SD_Write_Disk(uint8_t *pBuff, uint32_t Block_addr, uint32_t Blocks_Nbr) {
  uint32_t timeout = SD_RW_TIMEOUT;
  int errorcode = 0;
  do {
    uint16_t BufferTimes = Blocks_Nbr / SDIO_DMA_ALIGN4_BUF_BLOCKS;
    uint32_t LeftBlocks = Blocks_Nbr % SDIO_DMA_ALIGN4_BUF_BLOCKS;

    for (uint16_t i = 0; i < BufferTimes; i++) {
      /* Copy data first */
      memcpy(align4_buf, pBuff, SDIO_DMA_ALIGN4_BUF_SIZE);
      SCB_CleanDCache_by_Addr((uint32_t *)align4_buf, SDIO_DMA_ALIGN4_BUF_SIZE); // Data modified from CPU to DMA

      // wait sd card enter trans state
      while (HAL_SD_GetCardState(&SD_HANDLE) != HAL_SD_CARD_TRANSFER) {
        timeout--;
        if (timeout == 0) {
          errorcode = 1;
          break;
        }
      }
      if (errorcode)
        break;

      /* start dma */
      SDError = 0;
      SDTxCplt = 0;
      if (HAL_SD_WriteBlocks_DMA(&SD_HANDLE, align4_buf, Block_addr, SDIO_DMA_ALIGN4_BUF_BLOCKS) != HAL_OK) {
        errorcode = 2;
        break;
      }

      /* check trans complete */
      while (!SDTxCplt && !SDError) {
      }
      if (SDError) {
        errorcode = 3;
        break;
      }

      /* modify pointer and address */
      pBuff += SDIO_DMA_ALIGN4_BUF_SIZE;
      Block_addr += SDIO_DMA_ALIGN4_BUF_BLOCKS;
    }
    if (errorcode)
      break;

    /* Trans Left blocks */
    if (LeftBlocks) {
      /* Copy data first */
      memcpy(align4_buf, pBuff, LeftBlocks * SDIO_BLOCK_SIZE);
      SCB_CleanDCache_by_Addr((uint32_t *)align4_buf, LeftBlocks * SDIO_BLOCK_SIZE); // Data modified from CPU to DMA

      // wait sd card enter trans state
      while (HAL_SD_GetCardState(&SD_HANDLE) != HAL_SD_CARD_TRANSFER) {
        timeout--;
        if (timeout == 0) {
          errorcode = 1;
          break;
        }
      }
      if (errorcode)
        break;

      /* start dma */
      SDTxCplt = 0;
      SDError = 0;
      if (HAL_SD_WriteBlocks_DMA(&SD_HANDLE, align4_buf, Block_addr, LeftBlocks) != HAL_OK) {
        errorcode = 4;
        break;
      }
      /* check trans complete */
      while (!SDTxCplt && !SDError) {
      }
      if (SDError) {
        errorcode = 5;
        break;
      }
    }
  } while (0);
  if (errorcode)
    printf("SD_Write_Disk ErrorCode: %d\r\n", errorcode);
  return errorcode;
}

#endif

/** SD_Info_Print **/
void sdcard_info(SD_HandleTypeDef *sd) {
  printf("---------- SD Card Info ----------\n");
  printf("CardType:%ld\n", sd->SdCard.CardType);
  printf("CardVersion:%ld\n", sd->SdCard.CardVersion);
  printf("Class:%ld\n", sd->SdCard.Class);
  printf("BlockNbr:%ld\n", sd->SdCard.BlockNbr);
  printf("BlockSize:%ld\n", sd->SdCard.BlockSize);
  printf("LogBlockNbr:%ld\n", sd->SdCard.LogBlockNbr);
  printf("LogBlockSize:%ld\n", sd->SdCard.LogBlockSize);
  // printf("CardSpeed:%ld\n", sd->SdCard.CardSpeed);
  printf("---------- Card Info End ----------\n");
}

/** 读写测试 */
#if SDIO_RW_TEST
void sd_rw_test(void) {
  uint8_t errorcode = 0;
  uint32_t ts = 0, te = 0;
  HAL_StatusTypeDef state;

  do {
    for (uint32_t i = 0; i < SD_TEST_BUFF_SIZE; i++) {
      sd_buff[i] = TESTDATA;
    }
    SCB_CleanDCache_by_Addr((uint32_t *)sd_buff, SD_TEST_BUFF_SIZE); // Data modified from CPU to DMA

    /** 写 **/
    // SD_Write_Disk(sd_buff, 0, SD_TEST_BUFF_SIZE / 512);
    while (HAL_SD_GetCardState(&SD_HANDLE) != HAL_SD_CARD_TRANSFER) {
    }
    SDTxCplt = 0;
    SDError = 0;
    ts = HAL_GetTick();
    state = HAL_SD_WriteBlocks_DMA(&SD_HANDLE, sd_buff, 0, SD_TEST_BUFF_SIZE / SDIO_BLOCK_SIZE);
    if (state != HAL_OK) {
      printf("HAL_SD_WriteBlocks_DMA\r\n");
      errorcode = 1;
      break;
    }
    while (!SDTxCplt && !SDError) {
    }
    if (SDError) {
      errorcode = 2;
      break;
    }
    te = HAL_GetTick();
    printf("SDIO Write >>>: %ld ms / %ldkb\r\n", te - ts, SD_TEST_BUFF_SIZE / 1024U);

    /** 读**/
    for (uint32_t i = 0; i < SD_TEST_BUFF_SIZE; i++) {
      sd_buff[i] = 0x00;
    }
    SCB_CleanDCache_by_Addr((uint32_t *)sd_buff, SD_TEST_BUFF_SIZE); // Data modified from CPU to DMA

    // SD_Read_Disk(sd_buff, 0, SD_TEST_BUFF_SIZE / 512);
    while (HAL_SD_GetCardState(&SD_HANDLE) != HAL_SD_CARD_TRANSFER) {
    }
    SDRxCplt = 0;
    SDError = 0;
    ts = HAL_GetTick();
    state = HAL_SD_ReadBlocks_DMA(&SD_HANDLE, sd_buff, 0, SD_TEST_BUFF_SIZE / SDIO_BLOCK_SIZE);
    SCB_InvalidateDCache_by_Addr(sd_buff, SD_TEST_BUFF_SIZE); // Data modified from DMA to CPU
    if (state != HAL_OK) {
      printf("HAL_SD_ReadBlocks_DMA\r\n");
      errorcode = 3;
      break;
    }
    while (!SDRxCplt && !SDError) {
    }
    if (SDError) {
      errorcode = 4;
      break;
    }
    te = HAL_GetTick();
    printf("SDIO Read  >>>: %ld ms / %ldkb\r\n", te - ts, SD_TEST_BUFF_SIZE / 1024);

    /* check transform data */
    for (uint32_t i = 0; i < SD_TEST_BUFF_SIZE; i++) {
      if (sd_buff[i] != TESTDATA) {
        errorcode = 5;
        break;
      }
    }
  } while (0);

  printf("\r\nsdcard test error code: %d\r\n", errorcode);
}
#endif

/**
 * @brief 以下为中断回调函数 标志位操作
 * @param hsd
 */

#if SDIO_USE_REGISTCALLBAKS
void BSP_SD_TxCpltCallback(SD_HandleTypeDef *hsd) {
  SDTxCplt = 1;
  // printf("sd tx\r\n");
}
void BSP_SD_RxCpltCallback(SD_HandleTypeDef *hsd) {
  SDRxCplt = 1;
  SCB_InvalidateDCache_by_Addr((uint32_t *)align4_buf, SDIO_DMA_ALIGN4_BUF_SIZE); // Data modified from DMA to CPU
  // printf("sd rx\r\n");
}
void BSP_SD_ErrorCallback(SD_HandleTypeDef *hsd) {
  SDError = 1;
  // printf("sd error\r\n");
}

#else
void HAL_SD_TxCpltCallback(SD_HandleTypeDef *hsd) {
  sd_tx_cplt = 1;
  // printf("sd tx\r\n");
}
void HAL_SD_RxCpltCallback(SD_HandleTypeDef *hsd) {
  sd_rx_cplt = 1;
  // printf("sd rx\r\n");
}
void HAL_SD_ErrorCallback(SD_HandleTypeDef *hsd) {
  sd_error = 1;
  // printf("sd error\r\n");
}

#endif
