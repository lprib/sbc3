#![no_main]
#![no_std]

use core::cell::OnceCell;

use cortex_m_rt::entry;
use hal::{pac, prelude::*};
use libui::{display_device::DisplayDevice, pixel_buffer::PixelBuffer, Rect, text::NORMAL_FONT};
use num_traits::real::Real;
use panic_halt as _;
use ssd1322::{
    commands::{
        AddressIncrement, ClockDivide, Command, DisplayMode, GsDisplayQuality, ScanDirection,
        VddMode, VslMode,
    },
    Parallel8080, Ssd1322, TransactionType,
};
use stm32f4xx_hal as hal;

mod led_strip;
mod print;
mod ssd1322;

use led_strip::LedStrip;

static mut underlying_pixel_buffer: [u8; 256 * 64] = [0; { 256 * 64 }];
static mut pixel_buffer: OnceCell<PixelBuffer> = OnceCell::new();

fn setup_display<T: Ssd1322>(display: &mut T) -> Result<(), T::Error> {
    display.command(Command::SetCommandLock(false))?;
    display.command(Command::SetDisplayOn(false))?;
    display.command(Command::SetClockDivAndOscFreq {
        clock_div: ClockDivide::DivBy1,
        osc_freq: 1,
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
    display.command(Command::VddFunctionSelect(VddMode::Internal))?;
    display.command(Command::DisplayEnhancementA(
        VslMode::External,
        GsDisplayQuality::Enhanced,
    ))?;
    display.command(Command::SetContrastCurrent(0x80))?;
    display.command(Command::MasterContrastCurrentControl(8))?;
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
    unsafe {
        pixel_buffer
            .set(PixelBuffer::new(&mut underlying_pixel_buffer, 256, 64, 256))
            .unwrap();
    }

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

    display.command(Command::WriteRam).unwrap();

    for i in 1..100 {
        // unsafe {
        //     let rect = Rect::xywh(i * 2, i, 10, 10);
        //     pixel_buffer.get_mut().unwrap().clear(0);
        //     pixel_buffer.get_mut().unwrap().rect(rect, 15);
        //     display.refresh(
        //         pixel_buffer.get().unwrap(),
        //         // rect
        //         rect.with_offset(1), // pixel_buffer.get().unwrap().all(),
        //     );
        //     // println!("{:?}", rect.outset(1).best_fit_x(4));
        // }
        // delay.delay_ms(10u32);
    }
    leds.write_u16(0);
    leds.latch_output();

    unsafe {
        let pb = pixel_buffer.get_mut().unwrap();
        NORMAL_FONT.draw(pb, "The quick brown fox jumped over the lazy dog!", 0, 8, 15);
        NORMAL_FONT.draw(pb, "THE QUICK BROWN FOX JUMPED OVER THE LAZY DOG!", 0, 20, 15);
        NORMAL_FONT.draw(pb, "0123456789 !@#$%^&*(){}[],./;'<>?:\"\\|", 0, 32, 15);
        // for i in 0..16 {
        //     for hl in 0..10 {
        //         pb.vline(i*10+hl, 5, 59, i as u8);
        //     }
            // pb.rect(
            //     Rect {
            //         x: i * 10,
            //         y: 0,
            //         w: 10,
            //         h: 64,
            //     },
            //     i as u8,
            // );
        // }
        display.refresh(pb, pb.all());
    }

    loop {
        // for i in 0..255 {
        //     let pb = unsafe { pixel_buffer.get_mut().unwrap() };
        //     pb.clear(0);

        //     for which in 0..45 {
        //         let t = ((i + 5*which) as f32) / 255.0 * 2.0 * core::f32::consts::PI;
        //         // for which in 0..5;
        //         let x = t.sin() * 100.0 + 128.0;
        //         let r = Rect::xywh(x as i32, which + 4, 5, 5);
        //         pb.rect(r, (which/3) as u8);
        //         display.refresh(pb, r.with_offset(3));
        //     }

        //     // delay.delay_ms(3u32);
        // }
    }
}
