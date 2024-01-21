add_library(cmsis INTERFACE)
target_include_directories(cmsis
INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F4xx/Include
    ${CMAKE_CURRENT_LIST_DIR}/Include
)