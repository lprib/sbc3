use stm32f4::stm32f405::GPIOD;
use stm32f4xx_hal::hal::{blocking::delay::DelayMs, digital::v2::*};

use super::{commands::*, Ssd1322, TransactionType};

fn dc_state(txn_type: TransactionType) -> PinState {
    match txn_type {
        TransactionType::Data => PinState::High,
        TransactionType::Command => PinState::Low,
    }
}

pub struct Parallel8080<RD, WR, CS, DC, RES> {
    pub rd: RD,
    pub wr: WR,
    pub cs: CS,
    pub dc: DC,
    pub res: RES,
}

impl<RD, WR, CS, DC, RES, PinError> Parallel8080<RD, WR, CS, DC, RES>
where
    RD: OutputPin<Error = PinError>,
    WR: OutputPin<Error = PinError>,
    CS: OutputPin<Error = PinError>,
    DC: OutputPin<Error = PinError>,
    RES: OutputPin<Error = PinError>,
{
    /// Reset the chip. Wait at least 300ms for chip to come up.
    pub fn reset(&mut self, delay: &mut impl DelayMs<u8>) -> Result<(), PinError> {
        self.res.set_high()?;
        delay.delay_ms(100);
        self.res.set_low()?;
        delay.delay_ms(100);
        self.res.set_high()?;
        Self::setup_data_bus();

        self.cs.set_high()?;
        self.wr.set_high()?;
        self.rd.set_high()?;

        self.cs.set_low()?;
        Ok(())
    }

    fn setup_data_bus() {
        unsafe {
            let gpio_reg = GPIOD::PTR;
            // set all to very high speed, OSPEEDR[1:0] = 3
            (*gpio_reg)
                .ospeedr
                .modify(|r, w| w.bits(r.bits() | 0xffffu32));
            // set all to floating/no PU PD, PUPDR[1:0] = 0
            (*gpio_reg)
                .pupdr
                .modify(|r, w| w.bits(r.bits() & !0xffffu32));
        }
    }

    // TODO(liam) make all this generic over GPIO port and which run of pins
    // (0-7 in this case)
    fn set_data_bus_output() {
        unsafe {
            let gpio_reg = GPIOD::PTR;
            // set direction output, MODER[1:0] = 2
            (*gpio_reg)
                .moder
                .modify(|r, w| w.bits((r.bits() & !0xffffu32) | 0x5555u32));
        }
    }

    fn set_data_bus_input() {
        unsafe {
            let gpio_reg = GPIOD::PTR;
            // set direction input, MODER[1:0] = 0
            (*gpio_reg)
                .moder
                .modify(|r, w| w.bits(r.bits() & !0xffffu32));
        }
    }

    fn write_data_bus(data: u8) {
        unsafe {
            let gpio_reg = GPIOD::PTR;
            // set output, ODR = 0
            (*gpio_reg)
                .odr
                .modify(|r, w| w.bits((r.bits() & !0xff) | (data as u32)));
        }
    }

    fn read_data_bus() -> u8 {
        unsafe {
            let gpio_reg = GPIOD::PTR;
            // read IDR
            ((*gpio_reg).idr.read().bits() & 0xff) as u8
        }
    }
}

impl<RD, WR, CS, DC, RES, PinError> Ssd1322 for Parallel8080<RD, WR, CS, DC, RES>
where
    RD: OutputPin<Error = PinError>,
    WR: OutputPin<Error = PinError>,
    CS: OutputPin<Error = PinError>,
    DC: OutputPin<Error = PinError>,
    RES: OutputPin<Error = PinError>,
{
    type Error = PinError;

    fn write(&mut self, txn_type: TransactionType, data: u8) -> Result<(), PinError> {
        Self::set_data_bus_output();
        Self::write_data_bus(data);

        self.dc.set_state(dc_state(txn_type))?;
        self.cs.set_low()?;
        // t_as (10ns min)
        cortex_m::asm::delay(11); // at 168 MHZ (TOOD(liam): make generic)
        self.wr.set_low()?;
        // max(t_dsw, t_pwlw): t_dsw 40ns, t_pwlw 60ns
        cortex_m::asm::delay(11);

        self.wr.set_high()?;
        // t_pwhw
        cortex_m::asm::delay(11);
        self.cs.set_high()?;

        Ok(())
    }

    fn read(&mut self, txn_type: TransactionType) -> Result<u8, Self::Error> {
        Self::set_data_bus_input();
        self.dc.set_state(dc_state(txn_type))?;
        self.cs.set_low()?;
        // t_as (10ns min)
        cortex_m::asm::delay(11); // at 168 MHZ (TOOD(liam): make generic)
        self.rd.set_low()?;
        // t_acc (140ns max)
        // t_pwlr (150ns min)
        cortex_m::asm::delay(26);
        let data = Self::read_data_bus();

        self.rd.set_high()?;

        // t_pwhr
        cortex_m::asm::delay(11);
        self.cs.set_high()?;

        Ok(data)
    }

    fn multi_write_begin(&mut self) -> Result<(), Self::Error> {
        Self::set_data_bus_output();
        self.dc.set_state(dc_state(TransactionType::Data))?;
        self.cs.set_low()?;
        cortex_m::asm::delay(11);
        Ok(())
    }

    fn multi_write_byte(&mut self, data: u8) -> Result<(), Self::Error> {
        self.wr.set_low()?;
        Self::write_data_bus(data);
        // still works without delay...
        // cortex_m::asm::delay(25); // t_cycle/2 => 150ns
        self.wr.set_high()?;
        // cortex_m::asm::delay(25);
        Ok(())
    }

    fn multi_write_end(&mut self) -> Result<(), Self::Error> {
        self.cs.set_high()?;
        Ok(())
    }

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
