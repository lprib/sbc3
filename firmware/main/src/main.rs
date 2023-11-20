#![no_main]
#![no_std]

use core::fmt::Write;
use cortex_m_rt::entry;
use hal::{gpio, pac, prelude::*};
use nb::block;
use panic_halt as _;
use stm32f4xx_hal as hal;

use ledstrip::LedStrip;

#[entry]
fn main() -> ! {
    let p = pac::Peripherals::take().unwrap();
    let rcc = p.RCC.constrain();
    let clocks = rcc
        .cfgr
        // .use_hse(16.MHz())
        // .hclk(168.MHz())
        // .sysclk(168.MHz())
        .freeze();

    let mut gpiob = p.GPIOB.split();
    let mut dbg_led = gpiob.pb0.into_push_pull_output();

    let mut delay = p.TIM2.delay_us(&clocks);

    let gpioe = p.GPIOE.split();
    let mut leds = LedStrip::new(
        gpioe.pe2.into_push_pull_output(),
        gpioe.pe3.into_push_pull_output(),
        gpioe.pe5.into_push_pull_output(),
    )
    .unwrap();

    let tx_pin = gpiob.pb10.into_alternate();
    let mut tx = p.USART3.tx(
        tx_pin,
        hal::serial::Config::default()
            .baudrate(115200.bps())
            .wordlength_8()
            .parity_none(),
        &clocks,
    ).unwrap();

    let mut i = 0;

    loop {
        delay.delay_us(200_000u32);
        delay.delay_us(200_000u32);

        leds.write_u16(1 << i);
        leds.latch_output();

        // block!(rx.read()).unwrap();
        writeln!(tx, "Test").unwrap();

        i += 1;
        if i >= 16 {
            i = 0;
        }
    }
}
