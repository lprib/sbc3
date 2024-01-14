pub mod commands;
mod parallel8080;

pub use parallel8080::Parallel8080;

use libui::{display_device::DisplayDevice, pixel_buffer::PixelBuffer, Rect};
use stm32f4xx_hal::hal::digital::v2::OutputPin;

use self::commands::Command;

// https://github.com/presslab-us/fbtft/blob/f59e71cf7f8a0c717fcb0d6220e12ae2939205f4/fb_ssd1322.c

pub enum TransactionType {
    Data,
    Command,
}

// TODO(liam): This impl only relies on interfaces in trait Ssd1322 (generic
// over all interfaces). However we cannot blankey impl DisplayDevice as it's
// from another crate :(.
impl<RD, WR, CS, DC, RES, PinError> DisplayDevice for Parallel8080<RD, WR, CS, DC, RES>
where
    RD: OutputPin<Error = PinError>,
    WR: OutputPin<Error = PinError>,
    CS: OutputPin<Error = PinError>,
    DC: OutputPin<Error = PinError>,
    RES: OutputPin<Error = PinError>,
{
    type Error = PinError;

    fn refresh(&mut self, buf: &PixelBuffer, area: Rect) -> Result<(), Self::Error> {
        let area = area.best_fit_x(4);

        self.command(Command::SetColumnAddress {
            start: (<Self as Ssd1322>::COL_START + area.x as usize / 4) as u8,
            end: (<Self as Ssd1322>::COL_START + (area.x + area.w) as usize / 4 - 1) as u8,
        })?;

        self.command(Command::SetRowAddress {
            start: (<Self as Ssd1322>::ROW_START + area.y as usize) as u8,
            end: (<Self as Ssd1322>::ROW_START + (area.y + area.h) as usize) as u8,
        })?;

        self.command(Command::WriteRam)?;

        self.multi_write_begin()?;
        for y in area.y..(area.y + area.h) {
            for x in (area.x..(area.x + area.w)).step_by(4) {
                let first_byte = (buf.at(x + 0, y) << 4) | buf.at(x + 1, y);
                let second_byte = (buf.at(x + 2, y) << 4) | buf.at(x + 3, y);
                self.multi_write_byte(first_byte)?;
                self.multi_write_byte(second_byte)?;
            }
        }
        self.multi_write_end()?;
        Ok(())
    }
}

pub trait Ssd1322 {
    type Error;

    const COL_START: usize = 28;
    const COL_END: usize = 92;
    const ROW_START: usize = 0;
    const ROW_END: usize = 64;

    fn write(&mut self, txn_type: TransactionType, data: u8) -> Result<(), Self::Error>;
    fn read(&mut self, txn_type: TransactionType) -> Result<u8, Self::Error>;

    fn multi_write_begin(&mut self) -> Result<(), Self::Error>;
    fn multi_write_byte(&mut self, data: u8) -> Result<(), Self::Error>;
    fn multi_write_end(&mut self) -> Result<(), Self::Error>;

    fn raw_command(&mut self, command: u8, args: &[u8]) -> Result<(), Self::Error> {
        self.write(TransactionType::Command, command)?;
        for &arg in args {
            self.write(TransactionType::Data, arg)?;
        }
        Ok(())
    }

    fn command(&mut self, command: Command) -> Result<(), Self::Error> {
        match command {
            Command::EnableGrayScaleTable => self.raw_command(0x00, &[]),
            Command::SetColumnAddress { start, end } => {
                self.raw_command(0x15, &[start & 0x7f, end & 0x7f])
            }
            Command::WriteRam => self.raw_command(0x5c, &[]),
            Command::ReadRam => self.raw_command(0x5d, &[]),
            Command::SetRowAddress { start, end } => {
                self.raw_command(0x75, &[start & 0x7f, end & 0x7f])
            }
            Command::SetRemapMode {
                address_increment,
                column_address_remap,
                nibble_remap,
                scan_direction,
                split_odd_even,
                dual_com_line,
            } => self.raw_command(
                0xa0,
                &[
                    (address_increment as u8)
                        | ((column_address_remap as u8) << 1)
                        | ((nibble_remap as u8) << 2)
                        | ((scan_direction as u8) << 4)
                        | ((split_odd_even as u8) << 5),
                    0x01 | ((dual_com_line as u8) << 4),
                ],
            ),
            Command::SetStartLine(n) => self.raw_command(0xa1, &[n & 0x7f]),
            Command::SetOffset(n) => self.raw_command(0xa2, &[n & 0x7f]),
            Command::SetMode(mode) => self.raw_command(0xa0 | (mode as u8), &[]),
            Command::EnablePartialDisplay { start, end } => {
                self.raw_command(0xa8, &[start & 0x7f, end & 0x7f])
            }
            Command::ExitPartialDisplay => self.raw_command(0xa9, &[]),
            Command::VddFunctionSelect(mode) => self.raw_command(0xab, &[mode as u8]),
            Command::SetDisplayOn(enable) => self.raw_command(0xae | ((enable) as u8), &[]),
            Command::SetPhaseLength {
                phase_1_period,
                phase_2_period,
            } => self.raw_command(
                0xb1,
                &[(phase_1_period & 0x0f) | ((phase_2_period & 0x0f) << 4)],
            ),
            Command::SetClockDivAndOscFreq {
                clock_div,
                osc_freq,
            } => self.raw_command(
                0xb3,
                &[((clock_div as u8) & 0x0f) | ((osc_freq & 0x0f) << 4)],
            ),
            Command::DisplayEnhancementA(vsl, quality) => {
                self.raw_command(0xb4, &[0xa0 | (vsl as u8), 0x05 | ((quality as u8) << 3)])
            }
            Command::SetGpio { gpio_0, gpio_1 } => {
                self.raw_command(0xb6, &[(gpio_0 as u8) | ((gpio_1 as u8) << 2)])
            }
            Command::SetSecondPrechargePeriod(period) => self.raw_command(0xb6, &[period & 0x0f]),
            Command::SetGrayScaleTable(table) => self.raw_command(0xb8, &table),
            Command::SetDefaultGrayScaleTable => self.raw_command(0xb9, &[]),
            Command::SetPrechargeVoltage(voltage) => self.raw_command(0xbb, &[voltage & 0x1f]),
            Command::SetVcomh(vcomh) => self.raw_command(0xbe, &[vcomh & 0x07]),
            Command::SetContrastCurrent(contrast) => self.raw_command(0xc1, &[contrast]),
            Command::MasterContrastCurrentControl(cc) => self.raw_command(0xc7, &[cc & 0x0f]),
            Command::SetMuxRatio(ratio) => self.raw_command(0xca, &[ratio & 0x7f]),
            Command::SetCommandLock(lock) => self.raw_command(0xfd, &[0x12 | ((lock as u8) << 2)]),
        }
    }
}
