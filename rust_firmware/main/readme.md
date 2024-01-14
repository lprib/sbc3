# Main board firmware binary

## build
`cargo build --release`

Alternatively there is a vscode tasks.json that will build main.

## flash
_use cargo-flash_: `cargo flash --release --chip stm32f405vgtx`

_use openocd_:
- `openocd -f openocd.cfg` to start server
- `telnet localhost 4444` to connect as client
- In openocd telnet client:
    - `halt`
    - `flash write_image erase /home/liam/programming/sbc3/firmware/target/thumbv7em-none-eabihf/release/main`, replace with executable location
    - `reset`

_use cortex-debug_: pres F5 in VS code (see `launch.json`)

## debug
It is (partially) set up with `cortex-debug`.