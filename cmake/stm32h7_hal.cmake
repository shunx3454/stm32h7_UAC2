
set(STM32H7_HAL_DIR "${CMAKE_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver")

set(STM32H7_HAL_SRC
    stm32h7xx_hal.c
    stm32h7xx_hal_cortex.c
    stm32h7xx_hal_dma.c
    stm32h7xx_hal_dma_ex.c
    stm32h7xx_hal_flash.c
    stm32h7xx_hal_flash_ex.c
    stm32h7xx_hal_exti.c
    stm32h7xx_hal_gpio.c
    stm32h7xx_hal_hsem.c
    stm32h7xx_hal_mdma.c
    stm32h7xx_hal_rcc.c
    stm32h7xx_hal_rcc_ex.c
    stm32h7xx_hal_sdram.c
    stm32h7xx_hal_uart.c
    stm32h7xx_hal_uart_ex.c
    stm32h7xx_hal_tim.c
    stm32h7xx_hal_tim_ex.c
    stm32h7xx_hal_pwr.c
    stm32h7xx_hal_pwr_ex.c
    stm32h7xx_hal_sd.c
    stm32h7xx_hal_sd_ex.c
    stm32h7xx_hal_ltdc.c
    stm32h7xx_hal_i2c.c
    stm32h7xx_hal_i2c_ex.c
    stm32h7xx_hal_i2s.c
    stm32h7xx_hal_i2s_ex.c
    stm32h7xx_hal_dma2d.c
    stm32h7xx_hal_pcd_ex.c
    stm32h7xx_hal_pcd.c
    stm32h7xx_ll_fmc.c
    stm32h7xx_ll_sdmmc.c
    stm32h7xx_ll_usb.c
)

list(TRANSFORM STM32H7_HAL_SRC PREPEND "${STM32H7_HAL_DIR}/Src/")

set(STM32H7_HAL_INC 
    Drivers/CMSIS/Include
    Drivers/CMSIS/Device/ST/STM32H7xx/Include
    Drivers/STM32H7xx_HAL_Driver/Inc
    Drivers/STM32H7xx_HAL_Driver/Inc/Legacy
)