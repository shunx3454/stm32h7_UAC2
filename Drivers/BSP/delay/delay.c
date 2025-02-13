/**
 ****************************************************************************************************
 * @file        delay.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2021-10-14
 * @brief       使用SysTick的普通计数模式对延迟进行管理(支持ucosii)
 *              提供delay_init初始化函数， delay_us和delay_ms等延时函数
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 STM32F407开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 * 修改说明
 * V1.0 20211014
 * 第一次发布
 *
 ****************************************************************************************************
 */

#include "delay.h"
#include "FreeRTOS.h"

extern uint32_t SystemCoreClock;

static uint32_t g_fac_us = 0; /* us延时倍乘数 */

/**
 * @brief     延时nus
 * @param     nus: 要延时的us数
 * @note      nus取值范围: 0~8947848(最大值即2^32 / g_fac_us @g_fac_us = 168)
 * @retval    无
 */
void delay_us(uint32_t nus)
{
    uint32_t ticks;
    uint32_t told, tnow, tcnt = 0;
    uint32_t reload = SysTick->LOAD; /* LOAD的值 */

    /* edit  */
    g_fac_us = SystemCoreClock / 1000000;

    ticks = nus * g_fac_us; /* 需要的节拍数 */
    told = SysTick->VAL;    /* 刚进入时的计数器值 */
    while (1)
    {
        tnow = SysTick->VAL;
        if (tnow != told)
        {
            if (tnow < told)
            {
                tcnt += told - tnow; /* 这里注意一下SYSTICK是一个递减的计数器就可以了 */
            }
            else
            {
                tcnt += reload - tnow + told;
            }
            told = tnow;
            if (tcnt >= ticks)
            {
                break; /* 时间超过/等于要延迟的时间,则退出 */
            }
        }
    }
}

/**
 * @brief     延时nms
 * @param     nms: 要延时的ms数 (0< nms <= 65535)
 * @retval    无
 */
void delay_ms(uint16_t nms)
{
    uint32_t i;

    for (i = 0; i < nms; i++)
    {
        delay_us(1000);
    }
}

/**
  * @brief HAL库内部函数用到的延时
           HAL库的延时默认用Systick，如果我们没有开Systick的中断会导致调用这个延时后无法退出
  * @param Delay 要延时的毫秒数
  * @retval None
  */
void HAL_Delay(uint32_t Delay)
{
    delay_ms(Delay);
}
