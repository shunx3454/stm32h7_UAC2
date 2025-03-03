#ifndef __BORD_CONFIG_H_
#define __BORD_CONFIG_H_

/* OS support */
#define USE_FREERTOS 1

/* memory */
#define LINKSCRIP_EXSRAM
#ifdef LINKSCRIP_EXSRAM
#define __ATTR_EXSRAM __attribute__((section(".exsram")))
#define __ATTR_SDRAM __attribute__((section(".sdram")))
#define __ATTR_SRAM1 __attribute__((section(".ram_d1")))
#define __ATTR_SRAM2 __attribute__((section(".ram_d2")))
#endif

/* SDRAM */
#define SDRAM_BASE_ADDR (0xC0000000)
#define SDRAM_SIZE      (0x1000000)
#define SDRAM_BIT_WIDTH (32U)

/* GCC */
#ifndef __ALIGNED
#define __ALIGNED(x) __attribute__((aligned(x)))
#endif

/* SDIO Config */
#define SDIO_RW_TEST 1
#define SDIO_USE_DMA 1
#define SDIO_USE_REGISTCALLBAKS 1

/* LCD */
#define LCD_SCREEN_ROT 3 // 0:0deg,1:90deg,2:180deg,3:270deg
#define LCD_BLK_GPIO_Port GPIOB
#define LCD_BLK_Pin GPIO_PIN_15
#define LCD_BLK_GPIO_CLK_ENABLE __HAL_RCC_GPIOB_CLK_ENABLE
#define LCD_USE_DMA 1

/* LTDC */
#define LTDC_GRAM_ADDR (0xC0000000 + 0xf00000)  // 最后1M

/* TOUCH GT911 */
#define TOUCH_GT911_LOG 0
#define TOUCH_GT911_ROT 3 // 0:0deg,1:90deg,2:180deg,3:270deg

/* I2S_DMA */
#define I2S_DMA_QUEUE_SIZE (4096U)
#define I2S_DMA_QUEUE_LENGTH (8U)
#define DECODER_INBUF_SIZE (8 * 1024U)
#define DECODER_OUTBUF_SIZE (4 * 1024U)
#define DECODER_INPUT_THRESHOLD (4096U)

/* Build Time */
#define BUILD_TIME "2025-03-03-1 20:21:26"

#endif // __BORD_CONFIG_H_
