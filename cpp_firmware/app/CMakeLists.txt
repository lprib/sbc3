add_executable(app)
target_sources(app
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/main.c
    ${CMAKE_CURRENT_LIST_DIR}/pure_virtual_handler.c
    ${CMAKE_CURRENT_LIST_DIR}/startup_stm32f405xx.s
)

target_link_libraries(app stm32f4xx_hal)

set(LINKER_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/STM32F405VGTx_FLASH.ld)

target_link_options(app
PUBLIC
    -T${LINKER_SCRIPT}
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