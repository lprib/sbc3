#![no_main]
#![no_std]

use core::fmt::Write;
use cortex_m_rt::entry;
use hal::{gpio, pac, prelude::*};
use nb::block;
use panic_halt as _;
use ssd1322::{
    AddressIncrement, Command, DisplayInterface, DisplayMode, GsDisplayQuality, Parallel8080,
    ScanDirection, TransactionType, VslMode,
};
use stm32f4xx_hal as hal;

mod led_strip;
mod print;
mod ssd1322;

use led_strip::LedStrip;

fn setup_display<T: DisplayInterface>(display: &mut T) -> Result<(), T::Error> {
    display.command(Command::SetCommandLock(false))?;
    display.command(Command::SetDisplayOn(false))?;
    display.command(Command::SetClockDivAndOscFreq {
        clock_div: ssd1322::ClockDivide::DivBy2,
        osc_freq: 16,
    })?;
    display.command(Command::SetMuxRatio(0x3f))?;
    display.command(Command::SetStartLine(0))?;
    display.command(Command::SetOffset(0))?;
    display.command(Command::SetRemapMode {
        address_increment: AddressIncrement::Horizontal,
        column_address_remap: false,
        nibble_remap: true,
        scan_direction: ScanDirection::Downwards,
        split_odd_even: false,
        dual_com_line: true,
    })?;
    display.command(Command::VddFunctionSelect(ssd1322::VddMode::Internal))?;
    display.command(Command::DisplayEnhancementA(
        VslMode::External,
        GsDisplayQuality::Enhanced,
    ))?;
    display.command(Command::SetContrastCurrent(0xff))?;
    display.command(Command::MasterContrastCurrentControl(15))?;
    display.command(Command::SetPhaseLength {
        phase_1_period: 2,
        phase_2_period: 15,
    })?;

    display.command(Command::SetPrechargeVoltage(13))?;
    display.command(Command::SetVcomh(0))?;
    display.command(Command::SetMode(DisplayMode::Normal))?;

    display.command(Command::SetDisplayOn(true))?;

    display.command(Command::SetColumnAddress { start: 28, end: 91 })?;
    display.command(Command::SetRowAddress { start: 0, end: 63 })?;
    Ok(())
}

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


    // setup uart println
    let print_tx_pin = gpiob.pb10.into_alternate();
    let mut print_tx = p
        .USART3
        .tx(
            print_tx_pin,
            hal::serial::Config::default()
                .baudrate(115200.bps())
                .wordlength_8()
                .parity_none(),
            &clocks,
        )
        .unwrap();
    print::init(print_tx);

    let mut delay = p.TIM2.delay_ms(&clocks);

    let mut leds = LedStrip::new(
        gpioe.pe2.into_push_pull_output(),
        gpioe.pe3.into_push_pull_output(),
        gpioe.pe5.into_push_pull_output(),
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

    delay.delay_ms(100u8);

    display.reset(&mut delay).unwrap();

    setup_display(&mut display).unwrap();

    display.command(Command::WriteRam);
    for i in 0..118 {
        for y in 0..64 {
            for x2 in 0..128 {
                if y > 10 && y < 20 && x2 > i && x2 < (i + 10) {
                    display.write(TransactionType::Data, 0xff);
                } else {
                    display.write(TransactionType::Data, 0x00);
                }
            }
        }
        // delay.delay_ms(10u32);
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

        println!("wowza{}", i);

        i += 1;
        if i >= 16 {
            i = 0;
        }
    }
}
