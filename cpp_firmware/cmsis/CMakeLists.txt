add_library(cmsis INTERFACE)
target_include_directories(cmsis
INTERFACE
    Device/ST/STM32F4xx/Include
    Include
)

# HAL does heaps of volatile |= which c++ decided to deprecate (thanks c++
# commitee)
target_compile_options(cmsis
INTERFACE
    $<$<COMPILE_LANGUAGE:CXX>:-Wno-volatile>
)