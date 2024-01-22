add_executable(app)
target_sources(app
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/main.c
    ${CMAKE_CURRENT_LIST_DIR}/startup_stm32f405xx.s
    ${CMAKE_CURRENT_LIST_DIR}/system_stm32f4xx.c
)

target_link_libraries(app
PRIVATE
    stm32f4xx_hal
    freertos_kernel
)
target_compile_options(app
PRIVATE
    -O3
)
set(LINKER_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/STM32F405VGTx_FLASH.ld)
set_target_properties(app PROPERTIES LINK_DEPENDS ${LINKER_SCRIPT})

target_link_options(app
PUBLIC
    -T${LINKER_SCRIPT}
    -Wl,--gc-sections
    -lc -lm
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