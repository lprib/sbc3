cmake_minimum_required(VERSION 3.25)
project(sbc3 LANGUAGES C ASM)

set(CMAKE_COLOR_DIAGNOSTICS ON)

add_subdirectory(cmsis)
add_subdirectory(stm32f4xx_hal)
add_subdirectory(app)

set(FREERTOS_PORT GCC_ARM_CM4F)
set(FREERTOS_HEAP 4)
set(FREERTOS_CONFIG_FILE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/app)

add_subdirectory(freertos)