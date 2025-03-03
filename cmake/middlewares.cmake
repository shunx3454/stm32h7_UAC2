file(GLOB_RECURSE LVGL_SOURCE
    Middlewares/lvgl/src/*.c
    Middlewares/lvgl/demos/*.c
    Middlewares/lvgl/examples/porting/*.c
)

file(GLOB CHERRY_USB_SOURCE
    Middlewares/CherryUSB/port/dwc2/usb_dc_dwc2.c
    Middlewares/CherryUSB/port/dwc2/usb_glue_st.c
    Middlewares/CherryUSB/core/usbd_core.c
    Middlewares/CherryUSB/osal/usb_osal_freertos.c
    Middlewares/CherryUSB/class/cdc/usbd_cdc_acm.c
    Middlewares/CherryUSB/class/msc/usbd_msc.c
    Middlewares/CherryUSB/class/audio/usbd_audio.c
)

file(GLOB LIBFLAC_SRC 
    Middlewares/flac/src/libFLAC/bitreader.c
    Middlewares/flac/src/libFLAC/crc.c
    Middlewares/flac/src/libFLAC/format.c
    Middlewares/flac/src/libFLAC/lpc.c
    Middlewares/flac/src/libFLAC/md5.c
    Middlewares/flac/src/libFLAC/stream_decoder.c
    Middlewares/flac/src/libFLAC/memory.c
    Middlewares/flac/src/libFLAC/fixed.c
    Middlewares/flac/src/libFLAC/cpu.c
)


file(GLOB MIDDLEWARES_SRC 
    Middlewares/Fatfs/*.c
    Middlewares/FreeRTOS/*.c
    Middlewares/FreeRTOS/portable/MemMang/heap_4.c
    Middlewares/FreeRTOS/portable/GCC/ARM_CM4F/port.c
    ${LVGL_SOURCE}
    ${CHERRY_USB_SOURCE}
    ${LIBFLAC_SRC}
)

set(MIDDLEWARES_INC 
    Middlewares/Fatfs
    Middlewares/FreeRTOS/include
    Middlewares/FreeRTOS/portable/GCC/ARM_CM4F
    Middlewares/lvgl
    Middlewares/CherryUSB/core
    Middlewares/CherryUSB/common
    Middlewares/CherryUSB/class/cdc
    Middlewares/CherryUSB/class/msc
    Middlewares/CherryUSB/class/audio
    Middlewares/flac/include
    Middlewares/flac/src/libFLAC/include
    Middlewares/flac/src/libFLAC/include/private
)