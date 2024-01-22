cmake_minimum_required(VERSION 3.25)
project(sbc3 LANGUAGES C ASM)

set(CMAKE_COLOR_DIAGNOSTICS ON)

add_subdirectory(cmsis)
add_subdirectory(stm32f4xx_hal)
add_subdirectory(app)