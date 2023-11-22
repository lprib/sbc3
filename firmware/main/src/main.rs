#![no_main]
#![no_std]

use core::fmt::Write;
use cortex_m_rt::entry;
use hal::{gpio, pac, prelude::*};
use nb::block;
use panic_halt as _;
use ssd1322::{DisplayInterface, Parallel8080, TransactionType};
use stm32f4xx_hal as hal;

mod led_strip;
mod ssd1322;

use led_strip::LedStrip;

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

    let gpiob = p.GPIOB.split();
    let gpioc = p.GPIOC.split();
    let _ = p.GPIOD.split();
    let gpioe = p.GPIOE.split();
    let dbg_led = gpiob.pb0.into_push_pull_output();

    let mut delay = p.TIM2.delay_us(&clocks);

    let mut leds = LedStrip::new(
        gpioe.pe2.into_push_pull_output(),
        gpioe.pe3.into_push_pull_output(),
        gpioe.pe5.into_push_pull_output(),
    )
    .unwrap();

    let tx_pin = gpiob.pb10.into_alternate();
    let mut tx = p
        .USART3
        .tx(
            tx_pin,
            hal::serial::Config::default()
                .baudrate(115200.bps())
                .wordlength_8()
                .parity_none(),
            &clocks,
        )
        .unwrap();

    let mut i = 0;

    let mut display = Parallel8080 {
        rd: gpiob.pb4.into_push_pull_output(),
        wr: gpiob.pb5.into_push_pull_output(),
        cs: gpioc.pc11.into_push_pull_output(),
        dc: gpiob.pb8.into_push_pull_output(),
        res: gpioc.pc12.into_push_pull_output(),
    };

    // display.read(typ)

    display.reset(&mut delay).unwrap();
    delay.delay_us(100_000u32);
    display.write(TransactionType::Command, 0xa5).unwrap();
    delay.delay_us(100_000u32);

    display.write(TransactionType::Command, 0x5c).unwrap();
    delay.delay_us(100_000u32);

    display.write(TransactionType::Command, 0x15).unwrap();
    delay.delay_us(100_000u32);
    display.write(TransactionType::Data, 0).unwrap();
    delay.delay_us(100_000u32);
    display.write(TransactionType::Data, 119).unwrap();
    delay.delay_us(100_000u32);

    display.write(TransactionType::Command, 0x5d).unwrap();
    delay.delay_us(100_000u32);
    let a = display.read(TransactionType::Data).unwrap();
    delay.delay_us(100_000u32);
    let b = display.read(TransactionType::Data).unwrap();
    delay.delay_us(100_000u32);
    let c = display.read(TransactionType::Data).unwrap();
    delay.delay_us(100_000u32);

    writeln!(tx, "Got {a} {b} {c}");

    loop {
        delay.delay_us(200_000u32);
        delay.delay_us(200_000u32);

        leds.write_u16(1 << i);
        leds.latch_output();

        // block!(rx.read()).unwrap();
        writeln!(tx, "Test {i}").unwrap();

        i += 1;
        if i >= 16 {
            i = 0;
        }
    }
}
