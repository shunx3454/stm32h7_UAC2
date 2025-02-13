cmake_minimum_required(VERSION 3.22)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME fankeH7)

# Include toolchain file
include("cmake/gcc-arm-none-eabi.cmake")
include("cmake/stm32h7_hal.cmake")
include("cmake/middlewares.cmake")

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
# add_subdirectory(cmake/stm32cubemx)

# Add some compile commonds
target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE
        -Wno-unused-parameter
        -Wno-pedantic
)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

file(GLOB USER_SRC 
    startup_stm32h750xx.s
    User/Src/*.c
    Drivers/BSP/sdram/sdram.c
    Drivers/BSP/delay/delay.c
    Drivers/BSP/sdio/sd_app.c
    Drivers/BSP/lcd/lcd.c
    Drivers/BSP/lcd/lcd_fonts.c
    Drivers/BSP/gt911/gt911.c
    Drivers/BSP/sw_i2c/sw_i2c.c
    Drivers/BSP/audio/*.c
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    ${STM32H7_HAL_SRC}
    ${MIDDLEWARES_SRC}
    ${USER_SRC}
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    ${STM32H7_HAL_INC}
    ${MIDDLEWARES_INC}
    User/Inc
    Drivers/BSP/sdram
    Drivers/BSP/delay
    Drivers/BSP/sdio
    Drivers/BSP/lcd
    Drivers/BSP/gt911
    Drivers/BSP/sw_i2c
    Drivers/BSP/audio
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
    USE_HAL_DRIVER 
	STM32H750xx
    $<$<CONFIG:Debug>:DEBUG>
)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    # Add user defined libraries
)

# Config project build time
# 获取当前时间
string(TIMESTAMP BUILD_TIME "%Y-%m-%d-%w %H:%M:%S")

# 生成头文件的路径
set(GENERATED_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/User/Inc/bord_config.h")

# 配置头文件内容
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/bord_config.h.in
    ${GENERATED_HEADER}
    @ONLY
)

# Generate HEX and BIN file
set(HEX_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.hex)
set(BIN_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin)

add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_SIZE} ${PROJECT_NAME}.elf
    COMMAND echo "Building ${HEX_FILE}"
    COMMAND echo "Building ${BIN_FILE}"
    COMMAND ${CMAKE_OBJCOPY} -Oihex   ${PROJECT_NAME}.elf ${HEX_FILE}
    COMMAND ${CMAKE_OBJCOPY} -Obinary ${PROJECT_NAME}.elf ${BIN_FILE}
)