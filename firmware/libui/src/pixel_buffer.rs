use crate::Rect;

#[derive(Debug)]
pub struct PixelBuffer<'a> {
    buf: &'a mut [u8],
    /// width (px)
    w: usize,
    /// height (px)
    h: usize,
    /// stride (px)
    stride: usize,
}

impl<'a> PixelBuffer<'a> {
    pub fn new(buf: &'a mut [u8], w: usize, h: usize, stride: usize) -> Self {
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

    const fn max_color(px_per_byte: usize) -> u8 {
        1 << (8 / px_per_byte) - 1
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
