cmake_minimum_required(VERSION 3.25)
project(sbc3 LANGUAGES C CXX ASM)

set(CMAKE_COLOR_DIAGNOSTICS ON)

set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_C_FLAGS_DEBUG "-O0")

set(CMAKE_C_FLAGS_RELEASE "-Os")
set(CMAKE_CXX_FLAGS_RELEASE "-Os")


# Configure freertos
set(FREERTOS_PORT "GCC_ARM_CM4F" CACHE STRING "")
set(FREERTOS_HEAP "4" CACHE STRING "")
set(FREERTOS_CONFIG_FILE_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/app" CACHE STRING "")
add_subdirectory(freertos)

add_subdirectory(cmsis)
add_subdirectory(stm32f4xx_hal)
add_subdirectory(app)
