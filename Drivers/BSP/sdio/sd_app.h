
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SD_APP_H
#define __SD_APP_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "periph.h"
#include "bord_config.h"

#define SD_RW_TEST 1
#define SD_HANDLE g_sdio1

typedef enum
{
	SD_RW_OK = 0x00000000U,
	SD_W_ERROR = 0x00000001U,
	SD_R_ERROR = 0x00000002U,
	SD_RW_TIMEOUT = 0x00000003U
} SD_RW_DISK_STATE;

int8_t SD_Detect(void);
void sd_rw_test(void);
void sdcard_info(SD_HandleTypeDef *sd);
int SD_Read_Disk(uint8_t *pBuff, uint32_t Block_addr, uint32_t Blocks_Nbr);
int SD_Write_Disk(uint8_t *pBuff, uint32_t Block_addr, uint32_t Blocks_Nbr);

#if SDIO_USE_REGISTCALLBAKS
	void BSP_SD_TxCpltCallback(SD_HandleTypeDef *hsd);
	void BSP_SD_RxCpltCallback(SD_HandleTypeDef *hsd);
	void BSP_SD_ErrorCallback(SD_HandleTypeDef *hsd);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __SD_APP_H */
