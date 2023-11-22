use stm32f4xx_hal::hal::digital::v2::OutputPin;
use cortex_m::asm::delay;

pub struct LedStrip<SD, SCLK, RCLK> {
    sd: SD,
    sclk: SCLK,
    rclk: RCLK,
}

impl<SD, SCLK, RCLK, PinError> LedStrip<SD, SCLK, RCLK>
where
    SD: OutputPin<Error = PinError>,
    SCLK: OutputPin<Error = PinError>,
    RCLK: OutputPin<Error = PinError>,
{
    pub fn new(mut sd: SD, mut sclk: SCLK, mut rclk: RCLK) -> Result<Self, PinError> {
        sd.set_low()?;
        sclk.set_low()?;
        rclk.set_low()?;
        Ok(LedStrip { sd, sclk, rclk })
    }

    pub fn write_u16(&mut self, state: u16) -> Result<(), PinError> {
        for i in 0..16 {
            self.sclk.set_low()?;
            self.sd.set_state((state & ((0x8000 >> i) as u16) != 0).into())?;
            // 20MHz minimum SCLK speed, 168MHz fastest clock speed = 8.4 => 9 cycles
            delay(9);
            self.sclk.set_high()?;
            delay(9);
        }
        self.sclk.set_low()?;
        Ok(())
    }

    pub fn latch_output(&mut self) -> Result<(), PinError> {
        self.rclk.set_high()?;
        delay(9);
        self.rclk.set_low()?;
        Ok(())
    }
}
