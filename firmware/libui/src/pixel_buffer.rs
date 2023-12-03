use crate::Rect;

#[derive(Debug)]
pub struct MonochomeSpriteRef<'a> {
    pub width: u8,
    pub height: u8,
    pub stride: u8,
    pub data: &'a [u8],
}

#[derive(Debug)]
pub struct PixelBuffer<'a> {
    pub buf: &'a mut [u8],
    /// width (px)
    w: usize,
    /// height (px)
    h: usize,
    /// stride (px)
    stride: usize,
}

impl<'a> PixelBuffer<'a> {
    pub fn new(buf: &'a mut [u8], w: usize, h: usize, stride: usize) -> PixelBuffer<'a> {
        // note: buffer must have stride == w_px
        Self { buf, w, h, stride }
    }

    pub fn all(&self) -> Rect {
        Rect {
            x: 0,
            y: 0,
            w: self.w as i32,
            h: self.h as i32,
        }
    }

    pub fn hline(&mut self, x1: i32, x2: i32, y: i32, color: u8) {
        let start = (y * self.stride as i32 + x1) as usize;
        let end = (y * self.stride as i32 + x2) as usize;
        self.buf[start..=end].fill(color)
    }

    pub fn vline(&mut self, x: i32, y1: i32, y2: i32, color: u8) {
        for y in y1..=y2 {
            self.buf[(y * self.stride as i32 + x) as usize] = color;
        }
    }

    pub fn rect(&mut self, r: Rect, color: u8) {
        self.hline(r.x, r.x + r.w - 1, r.y, color);
        self.hline(r.x, r.x + r.w - 1, r.y + r.h - 1, color);
        self.vline(r.x, r.y, r.y + r.h - 1, color);
        self.vline(r.x + r.w - 1, r.y, r.y + r.h - 1, color);
    }

    pub fn clear(&mut self, color: u8) {
        self.buf.fill(color);
    }

    pub fn at(&self, x: i32, y: i32) -> u8 {
        self.buf[(y * self.stride as i32 + x) as usize]
    }

    pub fn at_mut(&mut self, x: i32, y: i32) -> &mut u8 {
        &mut self.buf[(y * self.stride as i32 + x) as usize]
    }

    pub fn blit_monochrome(&mut self, sprite_ref: &MonochomeSpriteRef, x: i32, y: i32, color: u8) {
        'row_loop: for y_iter in 0..sprite_ref.height {
            for byte_idx in 0..sprite_ref.stride {
                for bit_idx in 0..8 {
                    let x_iter = (byte_idx as i32) * 8 + bit_idx;

                    if x_iter >= sprite_ref.width as i32 {
                        continue 'row_loop;
                    }

                    let px = self.at_mut(x + x_iter, y + (y_iter as i32));
                    let bit = sprite_ref.data[(y_iter * sprite_ref.stride + byte_idx) as usize]
                        & 1 << bit_idx;
                    *px = if bit != 0 { color } else { 0 };
                }
            }
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_extent_best_fit_x() {
        let e = Rect {
            x: 19,
            y: 11,
            w: 13,
            h: 15,
        };
        let fit = e.best_fit_x(10);
        assert_eq!(fit.x, 10);
        assert_eq!(fit.w, 30);
        assert_eq!(fit.y, e.y);
        assert_eq!(fit.h, e.h);
    }
}
