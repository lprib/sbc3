# Main board firmware binary

## build
`cargo build --release`

Alternatively there is a vscode tasks.json that will build main.

## flash
use cargo-flash: `cargo flash --release --chip stm32f405vgtx`

use openocd:
- `openocd -f openocd.cfg` to start server
- `telnet localhost 4444` to connect as client
- In openocd telnet client:
    - `halt`
    - `flash write_image erase /home/liam/programming/sbc3/firmware/target/thumbv7em-none-eabihf/release/main`, replace with executable location
    - `reset`

## debug
It is (partially) set up with `cortex-debug`.