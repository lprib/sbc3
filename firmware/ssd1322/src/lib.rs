use std::marker::PhantomData;

use embedded_hal::{
    blocking::delay::DelayUs,
    digital::v2::{InputPin, IoPin, OutputPin, PinState},
};

pub trait CommandInterface {}
pub trait CycleDelay {
    fn delay_cycles(cycles: usize);
}

pub struct Parallel8080<NRD, NWR, NCS, NDC, NRES, DX, TDelay> {
    nrd: NRD,
    nwr: NWR,
    ncs: NCS,
    ndc: NDC,
    nres: NRES,
    data: [DX; 8],
    delay: PhantomData<TDelay>,
}

pub enum TransactionType {
    Data,
    Command,
}

impl<NRD, NWR, NCS, NDC, NRES, DXIN, DXOUT, DX, TDelay, PinError>
    Parallel8080<NRD, NWR, NCS, NDC, NRES, DX, TDelay>
where
    NRD: OutputPin<Error = PinError>,
    NWR: OutputPin<Error = PinError>,
    NCS: OutputPin<Error = PinError>,
    NDC: OutputPin<Error = PinError>,
    NRES: OutputPin<Error = PinError>,

    DXIN: InputPin<Error = PinError> + IoPin<DXIN, DXOUT>,
    DXOUT: OutputPin<Error = PinError> + IoPin<DXIN, DXOUT>,
    DX: IoPin<DXIN, DXOUT>,

    TDelay: CycleDelay,
{
    /// Reset the chip. Wait at least 300ms for chip to come up.
    pub fn reset<DELAY: DelayUs<u8>>(&mut self, mut delay: DELAY) -> Result<(), PinError> {
        self.nres.set_low()?;
        delay.delay_us(100);
        self.nres.set_high()?;
        Ok(())
    }

    pub fn write(&mut self, data: u8, typ: TransactionType) -> Result<(), PinError> {
        for i in 0..8 {
            self.data[i].set_state(((data & (1 << i)) != 0).into())?;
        }
        self.ncs.set_low()?;
        self.ndc.set_state(match typ {
            TransactionType::Data => PinState::Low,
            TransactionType::Command => PinState::High,
        })?;
        // t_as (10ns min)
        TDelay::delay_cycles(11); // at 168 MHZ (TOOD(liam): make generic)
        self.nwr.set_low()?;
        // max(t_dsw, t_pwlw): t_dsw 40ns, t_pwlw 60ns
        TDelay::delay_cycles(11);

        self.nwr.set_high()?;
        // t_pwhw
        TDelay::delay_cycles(11);
        self.ncs.set_high()?;
        Ok(())
    }

    // pub fn read(&mut self)
}
