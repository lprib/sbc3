mod parallel8080;

pub use parallel8080::Parallel8080;

use crate::println;


// https://github.com/presslab-us/fbtft/blob/f59e71cf7f8a0c717fcb0d6220e12ae2939205f4/fb_ssd1322.c

pub trait CycleDelay {
    fn delay_cycles(cycles: usize);
}

pub enum TransactionType {
    Data,
    Command,
}

pub trait Ssd1322 {
    type Error;
    fn write(&mut self, txn_type: TransactionType, data: u8) -> Result<(), Self::Error>;
    fn write_data(&mut self, data: &[u8]) -> Result<(), Self::Error>;
    fn read(&mut self, txn_type: TransactionType) -> Result<u8, Self::Error>;

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

pub enum Command {
    EnableGrayScaleTable,
    SetColumnAddress {
        start: u8,
        end: u8,
    },
    WriteRam,
    ReadRam,
    SetRowAddress {
        start: u8,
        end: u8,
    },
    SetRemapMode {
        address_increment: AddressIncrement,
        column_address_remap: bool,
        nibble_remap: bool,
        scan_direction: ScanDirection,
        split_odd_even: bool,
        dual_com_line: bool,
    },
    SetStartLine(u8),
    SetOffset(u8),
    SetMode(DisplayMode),
    EnablePartialDisplay {
        start: u8,
        end: u8,
    },
    ExitPartialDisplay,
    VddFunctionSelect(VddMode),
    SetDisplayOn(bool),
    SetPhaseLength {
        phase_1_period: u8,
        phase_2_period: u8,
    },
    SetClockDivAndOscFreq {
        clock_div: ClockDivide,
        // todo no docs on osc freq?
        osc_freq: u8,
    },
    DisplayEnhancementA(VslMode, GsDisplayQuality),
    SetGpio {
        gpio_0: GpioMode,
        gpio_1: GpioMode,
    },
    SetSecondPrechargePeriod(u8),
    // TODO this will make struct phat?
    SetGrayScaleTable([u8; 16]),
    SetDefaultGrayScaleTable,
    SetPrechargeVoltage(u8),
    SetVcomh(u8),
    SetContrastCurrent(u8),
    MasterContrastCurrentControl(u8),
    SetMuxRatio(u8),
    SetCommandLock(bool),
}

pub enum AddressIncrement {
    Horizontal = 0,
    Vertical = 1,
}

pub enum ScanDirection {
    Upwards = 0,
    Downwards = 1,
}

pub enum DisplayMode {
    AllOff = 4,
    AllOn = 5,
    Normal = 6,
    Inverse = 7,
}

pub enum VddMode {
    External = 0,
    Internal = 1,
}

pub enum ClockDivide {
    DivBy1 = 0,
    DivBy2 = 1,
    DivBy4 = 2,
    DivBy8 = 3,
    DivBy16 = 4,
    DivBy32 = 5,
    DivBy64 = 6,
    DivBy128 = 7,
    DivBy256 = 8,
    DivBy512 = 9,
    DivBy1024 = 10,
}

pub enum VslMode {
    External = 0,
    Internal = 2,
}

pub enum GsDisplayQuality {
    Normal = 0b10110,
    Enhanced = 0b11111,
}

pub enum GpioMode {
    HighZInputDisabled = 0,
    HighZInputEnabled = 1,
    OutputLow = 2,
    OutputHigh = 3,
}
