#include "periph.h"

int __io_putchar(int ch)
{
    HAL_UART_Transmit(&g_uart1, (uint8_t *)&ch, 1, 100);
    return (ch);
}
