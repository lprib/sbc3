# sbc3: small computing environment
Self contained board that can be easily programmed on target for fun little
games or GUIs. Inspired by the environment presented by TI calculators and
TI-BASIC.

# Directory structure
```
sbc3
|_ art: various images for putting on PCB and for on-screen graphics
|_ cubemx: STM32 CubeMX projects for pinmapping
|_ docs: datasheets etc
|  |_ cpu_hardware: hardware support and layout for STM32s
|  |_ hardware: various datasheets for parts used
|  \_ sofware: CPU ref manuals etc
|_ img: images for docs
|_ keyboard_v1: kicad project for keyboard
|_ main_board_v1: kicad project for main board
|_ rust_firmware: Firmware and screen driver in Rust (abandoned for now)
\_ software: (mostly c++) software and firmware
   |_ cmake: cmake scripts
   |_ firmware: firmware for main_board_v1
   |  |_ app: main application sources
   |  \_ (other dirs): supporting libraries unchanged from CubeMX/FreeRTOS upstream
   \_ vm: virtual machine
      |_ analysis: jupyter notebook for vm stat calculations
      |_ engine: main vm sources (portable)
      |_ pc_port: raylib port of VM for desktop
      |_ programs: sbc3 assembly programs
      |_ tests: vm unit tests (lol)
      |_ vscode-extension-sbcs-syntax: syntax support in VSCode for SBC3 assembly
      \_ as2.py: sbc3 vm bytecode assembler
```

## Components
### main_board_v1
#### System diagram
![System diagram](img/sbc3_system.svg)
#### PCB
![PCB](img/pcb.png)
#### Manufactured
![PCB photo](img/pcb_photo.jpg)

- PCB and assembly done by JLCPCB
- Extended parts used:
  - TLV320DAC3101IRHBR Audio DAC
  - 475900001 Micro-USB connector
  - LD1117S33TR 3V3 Linear Regulator
    - Honestly can't remember why I went for this one instead of the JLC
    preferred part AMS1117-3.3. Maybe max current
  - LD1117S18TR 1V8 Linear Regulator (for DAC)
  - STM32F405VGT6 MCU

#### Changes and ideas for v2
- [ ] Ability to disconnect 3v3 from rs485 connectors
- [ ] Isolation transformers on rs485 for long connections
- [ ] External flash or eeprom or FRAM for filesystem
- [ ] "Cartridges" with flash
- [ ] Label all the headers pin for pin to avoid looking on the schematic every
time you're connecting the SWD
- [ ] Fix BOOT0/BOOT1? Or just 0ohm unpopulated for de-bricking?
- [ ] Proper enclosure
- [ ] ESD diode protection on all inputs
- [ ] External qspi flash (w25q128jv)
- [ ] Wire OE on LED shift registers to a PWMable pin! So you can easily pwm the brightness
- [ ] 18650 batteries + holder?
- [ ] smaller form factor with parts hidden underscreen and less rs485 or more
compact layout (look for resistor/cap sizing for 0402 wattage)
- [ ] ACTUALLY WIRE UP THE BUTTONS AND ENCODER THIS TIME!
    - Of all the things to screw up on the first PCB run, I didn't expect to
    completely forget to connect the button and rotary nets
- [ ] Tiny JST connectors really are shit! Hot pluggable trs circuit?
Enable/disabable power chip? Need to get this right before making a bunch of peripheral boards.

### keyboard_v1
#### System diagram
![System diagram](img/sbc3_keyboard_system.svg)
#### PCB
![PCB](img/keyboard_pcb.png)

Not yet manufactured, since still thinking about re-doing the comms (definitely
not going to use the current JST connectors as theyre much too finicky).

### virtual machine
Also included is a stack based virtual bytecode machine (see [software/vm](./software/vm)).
The bytecode can call out to pre-defined system modules, including graphics
routines. Below is an example of the main board STM32 and a Linux desktop
running the same bytecode program.

#### STM32
![target](img/vm-on-target.gif)
#### Linux desktop
![pc](img/vm-on-pc.gif)

As the bytecode is stack based, a thin forth layer has been built for it, which
is currently assembled by a python script. See the source program for the above gifs [here](./software/vm/programs/tilemap.sbcs).