cmake_minimum_required(VERSION 3.22)
cmake_policy(SET CMP0135 NEW)

project(stm32_cpp20_fsm_tests CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../Drivers/cxx_hal
    ${CMAKE_CURRENT_SOURCE_DIR}/../Drivers/devices
    ${CMAKE_CURRENT_SOURCE_DIR}/../Middleware/fsm
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks
)

add_executable(fsm_unit_tests
    test_state_machine.cpp
    ../Drivers/devices/Max7219.cpp
    ../Drivers/devices/Sg90Servo.cpp
)

target_link_libraries(fsm_unit_tests PRIVATE gtest_main gmock)

include(GoogleTest)
gtest_discover_tests(fsm_unit_tests)
