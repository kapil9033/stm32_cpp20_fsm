cmake_minimum_required(VERSION 3.22)
project(stm32_cpp20_fsm C CXX ASM)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags for lightweight bare-metal C++
add_compile_options(
    -mcpu=cortex-m4
    -mthumb
    -mfpu=fpv4-sp-d16
    -mfloat-abi=hard
    -fdata-sections
    -ffunction-sections
    -fno-exceptions
    -fno-rtti
    -Wall
    -Wextra
)

# Project Include Directories
include_directories(
    Core/Inc
    Drivers/cxx_hal
    Drivers/devices
    Middleware/fsm
    Application
)