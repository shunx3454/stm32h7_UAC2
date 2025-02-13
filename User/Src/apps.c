#include <string.h>

#include "apps.h"
#include "main.h"
#include "periph.h"

#include "FreeRTOS.h"
#include "ff.h"
#include "fs_test.h"
#include "task.h"

#include "demos/flex_layout/lv_demo_flex_layout.h"
#include "demos/music/lv_demo_music.h"
#include "examples/porting/lv_port_disp.h"
#include "examples/porting/lv_port_indev.h"
#include "lvgl.h"

#include "audio_if.h"
#include "gt911.h"
#include "lcd.h"
#include "sd_app.h"
#include "sdram.h"
#include "usbd_cdc_acm.h"
#include "usbd_core.h"

#define STATIC_IDLE_STACK_SIZE 1024U
#define STATIC_TIMER_STACK_SIZE 1024U

TaskHandle_t ThreadStart;
TaskHandle_t ThreadAppMain;
TaskHandle_t ThreadLvgl;
StaticTask_t IdleTaskTCB;
StaticTask_t TimerTaskTCB;
StackType_t IdleTaskStack[STATIC_IDLE_STACK_SIZE];
StackType_t TimerTaskStack[STATIC_TIMER_STACK_SIZE];

FATFS fs;
int int_num = 0;

void audio_v2_loop(uint8_t busid);
void audio_v2_init(uint8_t busid, uintptr_t reg_base);
void cdc_acm_msc_init(uint8_t busid, uintptr_t reg_base);
void cdc_acm_init(uint8_t busid, uintptr_t reg_base);
void check_core_revision(void);
void fatfs_test(void);

void thread_start(void *param);
void thread_app_main(void *param);
void thread_lvgl_loop(void *param);

void os_run(void) {
  xTaskCreate(thread_start, "start", 2048, NULL, 1, &ThreadStart);
  vTaskStartScheduler();
}

void check_core_revision(void) {
  uint32_t cpuid = SCB->CPUID;             // 读取 CPUID 寄存器
  uint32_t revision = (cpuid >> 20) & 0xF; // 提取 revision
  uint32_t part_no = (cpuid >> 4) & 0xFFF; // 提取 part number

  printf("CPUID: 0x%08lX\n", cpuid);
  printf("Cortex-M Part Number: 0x%03lX\n", part_no);
  printf("Core Revision: r%lu\n", revision);
}

void thread_start(void *param) {
  vTaskSuspendAll();

  /* User code begin */
  tim_init(); // HAL TimeBase Inc
  HAL_TIM_Base_Start_IT(&g_tim6);
  uart_init();          // printf
  system_clock_print(); // system clock printf
  sdram_init();         // SDRAM
  SDRAM_Initialization_Sequence(&g_sdram1);
  // SDRAM_Test(SDRAM_BASE_ADDR, 1024 * 1024 * 16);
  gpio_init(); // led
  sdio_init(); // sdio
  // sd_rw_test();

  // USB FS
  // cdc_acm_init(0, USB2_OTG_FS_PERIPH_BASE);
  // cdc_acm_msc_init(0, USB2_OTG_FS_PERIPH_BASE);

  //  touch_init() --> lv_port_indev_init();
  //  ldtc_init() --> lcd_init() --> lv_port_disp_init()
  //  touch_init();

  // lv_init();
  // lv_port_disp_init();
  // lv_port_indev_init();
  // lv_demo_music();
  // lv_demo_benchmark();
  // lv_demo_flex_layout();
  // lcd_init();
  // lcd_refresh(0xffffffff);
  // lcd.bg_clr = 0xf800;
  // LCD_DisplayString(100, 100, "Fuck Ma Songnan");
  // lcd_show_charCN(100, 150, 0);
  // lcd_show_charCN(132, 150, 1);
  // lcd_show_charCN(164, 150, 2);
  // lcd_show_charCN(196, 150, 3);
  // lcd_show_charCN(228, 150, 4);
  // lcd_show_charCN(260, 150, 5);
  // lcd_show_charCN(292, 150, 6);

  xTaskCreate(thread_app_main, "app_main", 1024, NULL, 10, &ThreadAppMain);
  // xTaskCreate(thread_lvgl_loop, "lvgl_loop", 1024, NULL, 3, &ThreadLvgl);

  xTaskResumeAll();
  vTaskDelete(NULL);
}

void thread_app_main(void *param) {
  static uint8_t i2s_tx[4096] __ATTR_SDRAM;
  FIL fl;
  FRESULT res = 0;
  UINT nbr = 0;

  fatfs_mout(&fs, "/SD");
  if (fs.fs_type != 0) {
    fatfs_test();
    printf("sdio interrupt times:%d\r\n", int_num);
  }

  // config i2s
  I2S_InitTypeDef i2s_conf;
  DMA_InitTypeDef i2s_dma_conf;
  i2s_config_default(&i2s_conf);
  i2s_dma_config_default(&i2s_dma_conf);
  i2s_conf.AudioFreq = I2S_AUDIOFREQ_48K;
  i2s_init(&i2s_conf, &i2s_dma_conf, I2S_ASYNC_MODE);
  i2s_start();

  audio_v2_init(0, USB2_OTG_FS_PERIPH_BASE);

  //res = f_open(&fl, "/SD/MUSIC/光年之外.pcm", FA_READ);

  for (;;) {
    HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
    vTaskDelay(500);
    // printf("send i2s 1024 Bytes...\r\n");
    // if (!res) {
    //   res = f_read(&fl, i2s_tx, sizeof(i2s_tx), &nbr);
    //   i2s_send_async(i2s_tx, sizeof(i2s_tx));
    // }
    // vTaskDelay(10);
  }
}

void thread_lvgl_loop(void *param) {
  uint32_t times;
  for (;;) {
    times = lv_timer_handler();
    vTaskDelay(times);
  }
}

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer,
                                   configSTACK_DEPTH_TYPE *puxIdleTaskStackSize) {
  *ppxIdleTaskTCBBuffer = &IdleTaskTCB;
  *ppxIdleTaskStackBuffer = IdleTaskStack;
  *puxIdleTaskStackSize = STATIC_IDLE_STACK_SIZE;
}

void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer,
                                    configSTACK_DEPTH_TYPE *puxTimerTaskStackSize) {
  *ppxTimerTaskTCBBuffer = &TimerTaskTCB;
  *ppxTimerTaskStackBuffer = TimerTaskStack;
  *puxTimerTaskStackSize = STATIC_TIMER_STACK_SIZE;
}

void BSP_TIM_PeriodCpltCallback(TIM_HandleTypeDef *htim) {
  if (htim->Instance == TIM6) {
    HAL_IncTick();
    lv_tick_inc(1);
  }
}
