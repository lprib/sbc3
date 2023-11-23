use stm32f4::stm32f405::GPIOD;
use stm32f4xx_hal::hal::{blocking::delay::DelayMs, digital::v2::*};

use super::{DisplayInterface, TransactionType};

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

impl<RD, WR, CS, DC, RES, PinError> DisplayInterface for Parallel8080<RD, WR, CS, DC, RES>
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

    fn write_data(&mut self, data: &[u8]) -> Result<(), Self::Error> {
        Self::set_data_bus_output();
        self.dc.set_state(dc_state(TransactionType::Data))?;
        self.cs.set_low()?;
        cortex_m::asm::delay(11);

        for &b in data {
            self.wr.set_low()?;
            Self::write_data_bus(b);
            cortex_m::asm::delay(25); // t_cycle/2 => 150ns
            self.wr.set_high()?;
            cortex_m::asm::delay(25);
        }

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
}
