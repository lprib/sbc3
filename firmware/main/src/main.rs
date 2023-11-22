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

    let mut delay = p.TIM2.delay_ms(&clocks);

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
    delay.delay_ms(100u8);


    // display.write(TransactionType::Command, 0xb3);
    display.write(TransactionType::Command, 0xfd);
    display.write(TransactionType::Data, 0x12);

    display.write(TransactionType::Command, 0xae);

    display.write(TransactionType::Command, 0xb3);
    display.write(TransactionType::Data, 0xf1);

    display.write(TransactionType::Command, 0xca);
    display.write(TransactionType::Data, 0x3f);
    
    display.write(TransactionType::Command, 0xa2);
    display.write(TransactionType::Data, 0);

    display.write(TransactionType::Command, 0xa1);
    display.write(TransactionType::Data, 0);

    display.write(TransactionType::Command, 0xa0);
    display.write(TransactionType::Data, 0x14);
    display.write(TransactionType::Data, 0x11);

    display.write(TransactionType::Command, 0xab);
    display.write(TransactionType::Data, 1);

    display.write(TransactionType::Command, 0xb4);
    display.write(TransactionType::Data, 0xa0);
    display.write(TransactionType::Data, 0xfd);

    display.write(TransactionType::Command, 0xc1);
    display.write(TransactionType::Data, 0xff);

    display.write(TransactionType::Command, 0xc7);
    display.write(TransactionType::Data, 0x0f);

    display.write(TransactionType::Command, 0xb1);
    display.write(TransactionType::Data, 0xf0);

    display.write(TransactionType::Command, 0xd1);
    display.write(TransactionType::Data, 0x82);
    display.write(TransactionType::Data, 0x20);

    display.write(TransactionType::Command, 0xbb);
    display.write(TransactionType::Data, 0x0d);

    display.write(TransactionType::Command, 0xbe);
    display.write(TransactionType::Data, 0x00);

    display.write(TransactionType::Command, 0xa6);

    display.write(TransactionType::Command, 0xaf);


    display.write(TransactionType::Command, 0x15);
    display.write(TransactionType::Data, 0x1c);
    display.write(TransactionType::Data, 0x5b);

    display.write(TransactionType::Command, 0x75);
    display.write(TransactionType::Data, 0);
    display.write(TransactionType::Data, 63);

    display.write(TransactionType::Command, 0x5c);

    for i in 0..(7556*2) {
        display.write(TransactionType::Data, (i&0x0f) as u8);
    }

    // display.write_8080(TransactionType::Command, 0xa4, &mut delay);

    // display.write_8080(TransactionType::Command, 0xa2, &mut delay);
    // display.write_8080(TransactionType::Data, 100, &mut delay);

    // display.write_8080(TransactionType::Command, 0x5c, &mut delay);
    // for i in 0..15000
    // {
    //     display.write_8080(TransactionType::Data, 0x55, &mut delay);
    // }

    // delay.delay_us(100_000u32);
    // display.write(TransactionType::Command, 0xaf).unwrap();
    // delay.delay_us(100_000u32);
    // display.write(TransactionType::Command, 0xa4).unwrap();
    // delay.delay_us(100_000u32);

    // display.write(TransactionType::Command, 0x5c).unwrap();
    // delay.delay_us(100_000u32);
    // for i in 0..65535
    // {
    //     display.write(TransactionType::Data, 0).unwrap();
    // }

    // display.write(TransactionType::Command, 0x15).unwrap();
    // delay.delay_us(100_000u32);

    // display.write(TransactionType::Data, 0).unwrap();
    // delay.delay_us(100_000u32);
    // display.write(TransactionType::Data, 119).unwrap();
    // delay.delay_us(100_000u32);

    // display.write(TransactionType::Command, 0x5d).unwrap();
    // delay.delay_us(100_000u32);
    // let a = display.read(TransactionType::Data).unwrap();
    // delay.delay_us(100_000u32);
    // let b = display.read(TransactionType::Data).unwrap();
    // delay.delay_us(100_000u32);
    // let c = display.read(TransactionType::Data).unwrap();
    // delay.delay_us(100_000u32);

    // writeln!(tx, "Got {a} {b} {c}");

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
