add_executable(app)
target_sources(app
PRIVATE
    main.cpp
    system_clock_config.cpp
    gpio.hpp
    gpio.cpp
    util.hpp
    util.cpp
    ledstrip.hpp
    ledstrip.cpp
    uart.hpp
    uart.cpp
    serial.hpp
    serial.cpp
    error.hpp
    sync.hpp

    startup_stm32f405xx.s
    system_stm32f4xx.c
)

target_compile_features(app
PRIVATE
    cxx_std_20
)
target_link_libraries(app
PRIVATE
    stm32f4xx_hal
    freertos_kernel
)
target_compile_options(app
PRIVATE
    -O3
    $<$<COMPILE_LANGUAGE:CXX>:
    -Wall
    -Wextra
    -Wcast-align
    -Wcast-qual
    -Wctor-dtor-privacy
    -Wdisabled-optimization
    -Wformat=2
    -Winit-self
    -Wlogical-op
    -Wmissing-declarations
    -Wmissing-include-dirs
    -Wnoexcept
    -Woverloaded-virtual
    -Wredundant-decls
    -Wshadow
    -Wsign-conversion
    -Wsign-promo
    -Wstrict-null-sentinel
    -Wstrict-overflow=5
    -Wswitch
    -Wundef
    >
)
set(LINKER_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/STM32F405VGTx_FLASH.ld)
set_target_properties(app PROPERTIES LINK_DEPENDS ${LINKER_SCRIPT})
target_link_options(app
PUBLIC
    -T${LINKER_SCRIPT}
    -Wl,--gc-sections
    -Wl,--no-warn-rwx-segment # ld will generate LOAD RWE section for some reason, this shuts it up
    -lc -lm
)
target_include_directories(app
PRIVATE
    .
)

add_subdirectory(hal_conf)

# custom target to do arm-objcopy to bin file
add_custom_target(app_bin
ALL
DEPENDS
    app
COMMAND
    ${ARM_OBJCOPY_EXECUTABLE} -Obinary app app.bin
)