cmake_minimum_required(VERSION 3.25)
project(sbc3 LANGUAGES C CXX ASM)

set(CMAKE_COLOR_DIAGNOSTICS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(vm)
add_subdirectory(firmware)