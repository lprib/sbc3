#![no_main]
#![no_std]

use panic_halt as _;

use cortex_m_rt::entry;

use stm32f4xx_hal::{pac, prelude::*};

// use `main` as the entry point of this application
// `main` is not allowed to return
#[entry]
fn main() -> ! {
    let p = pac::Peripherals::take().unwrap();
    let rcc = p.RCC.constrain();
    let clocks = rcc
        .cfgr
        .use_hse(16.MHz())
        .hclk(168.MHz())
        .sysclk(168.MHz())
        .freeze();

    let mut gpiob = p.GPIOB.split();
    let mut dbg_led = gpiob.pb0.into_push_pull_output();

    let mut delay = p.TIM2.delay_us(&clocks);

    loop {
        dbg_led.set_high();
        delay.delay_us(200_000u32);
        dbg_led.set_low();
        delay.delay_us(200_000u32);
    }
}
