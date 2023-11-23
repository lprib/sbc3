use core::{cell::OnceCell, fmt};

use stm32f4::stm32f405::USART3;
use stm32f4xx_hal::{
    pac::Peripherals,
    serial::{SerialExt, Tx},
};

#[macro_export]
macro_rules! println {
    {} => ($crate::print!("\r\n"));
    {$( $arg:tt )*} => ($crate::print!("{}\r\n", format_args!($($arg)*)));
}

#[macro_export]
macro_rules! print {
    ($($arg:tt)*) => ($crate::print::_print_inner(format_args!($($arg)*)));
}

static mut PRINTING_UART_CELL: OnceCell<Tx<USART3>> = OnceCell::new();

/// Initialize the printing service from uart. Only call this once!
pub fn init(uart: stm32f4xx_hal::serial::Tx<USART3>) {
    unsafe {
        PRINTING_UART_CELL
            .set(uart)
            .unwrap_or_else(|_| panic!("print already initialized"));
    }
}

#[doc(hidden)]
pub fn _print_inner(args: fmt::Arguments) {
    cortex_m::interrupt::free(|_| {
        let uart = unsafe { PRINTING_UART_CELL.get_mut().unwrap() };
        fmt::write(uart, args).unwrap();
    });
}
