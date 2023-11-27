use crate::{pixel_buffer::PixelBuffer, Rect};

pub trait DisplayDevice {
    type Error;
    fn refresh(&mut self, buf: &PixelBuffer, area: Rect) -> Result<(), Self::Error>;
}
