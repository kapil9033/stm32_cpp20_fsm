cmake_minimum_required(VERSION 3.22)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/toolchains/arm-none-eabi.cmake)

project(stm32_cpp20_fsm C CXX ASM)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(ARM_FLAGS
    -mcpu=cortex-m4
    -mthumb
    -mfpu=fpv4-sp-d16
    -mfloat-abi=hard
)

# 1. Add ALL Include Paths (Core, HAL Drivers, CMSIS, FreeRTOS)
include_directories(
    Core/Inc
    Drivers/cxx_hal
    Drivers/devices
    Drivers/STM32F4xx_HAL_Driver/Inc
    Drivers/STM32F4xx_HAL_Driver/Inc/Legacy
    Drivers/CMSIS_Device/Include
    Drivers/CMSIS_5/CMSIS/Core/Include
    Middleware/fsm
    Application
    Middlewares/Third_Party/FreeRTOS/Source/include
    Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F
)

# 2. Gather Sources including HAL C files
file(GLOB_RECURSE SOURCES
    "Core/Src/*.cpp"
    "Core/Src/*.c"
    "Drivers/devices/*.cpp"
    "Application/*.cpp"
    "Middlewares/Third_Party/FreeRTOS/Source/*.c"
    "Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/*.c"
    "Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c"
)

set(HAL_SOURCES
    Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c
    Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cortex.c
    Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma.c
    Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash.c
    Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ex.c
    Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_gpio.c
    Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr.c
    Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr_ex.c
    Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc.c
    Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc_ex.c
    Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_spi.c
)

add_executable(${PROJECT_NAME}.elf
    ${SOURCES}
    ${HAL_SOURCES}
    Drivers/CMSIS_Device/Source/Templates/system_stm32f4xx.c
    Drivers/CMSIS_Device/Source/Templates/gcc/startup_stm32f446xx.s
)

target_compile_options(${PROJECT_NAME}.elf PRIVATE
    ${ARM_FLAGS}
    -fdata-sections
    -ffunction-sections
    -Wall
    -Wextra
    -Wpedantic
    -O2
    $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
)

target_compile_definitions(${PROJECT_NAME}.elf PRIVATE
    USE_HAL_DRIVER
    STM32F446xx
)

target_link_options(${PROJECT_NAME}.elf PRIVATE
    ${ARM_FLAGS}
    -T${CMAKE_CURRENT_SOURCE_DIR}/STM32F446RETx_FLASH.ld
    -nostartfiles
    -Wl,--gc-sections
    -Wl,-Map=${PROJECT_NAME}.map
    --specs=nano.specs
    --specs=nosys.specs
)

add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin
    COMMAND ${CMAKE_SIZE} ${PROJECT_NAME}.elf
    COMMENT "Building binary output and measuring FLASH/SRAM utilization:"
)