Requirements:
- base-devel
- ninja
- cmake
- arm-none-eabi-{gcc,gdb,newlib}
    - newlib provides gcc nano.specs to use newlib c std library
- openocd


Using VSCODE as IDE:
- install Cmake tools, C++ tools, cortex-debug
- to build:
  - see `.vscode/cmake-kits.json`: activate the baremetal kit, configure and build from cmake panel
  - this will also tell cmake to generate a compile_commands.json
  - this will generate an elf and bin in build/app/
- to debug:
  - cortex-debug already set up, use configuration