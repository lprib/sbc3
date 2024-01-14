#![no_std]

pub mod display_device;
pub mod pixel_buffer;
pub mod text;

#[derive(Debug, Copy, Clone, Eq, PartialEq)]
pub struct Rect {
    pub x: i32,
    pub y: i32,
    pub w: i32,
    pub h: i32,
}

impl Rect {
    /// Round extent outwards such that it's boundaries land on min_span
    /// multiples.
    ///
    /// Will be used for partial display updates in grahics driver
    pub fn best_fit_x(&self, min_span: u32) -> Self {
        let min_span = min_span as i32;
        let fit_x = (self.x / min_span) * min_span;
        let x2 = self.x + self.w;
        let fit_x2 = ((x2 + min_span - 1) / min_span) * min_span;
        assert!(fit_x2 > fit_x);
        let fit_w = fit_x2 - fit_x;

        Self {
            x: fit_x,
            y: self.y,
            w: fit_w,
            h: self.h,
        }
    }

    pub fn xywh(x: i32, y: i32, w: i32, h: i32) -> Self {
        Rect { x, y, w, h }
    }

    pub fn clip_within(&self, other: &Rect) -> Self {
        todo!()
        // Rect {
        //     x: self.x.max(other.x),
        //     y: self.y.max(other.y),
        //     w: (self.x + self.w)
        // }
    }

    pub fn with_offset(&self, n: i32) -> Self {
        Rect {
            x: self.x - n,
            y: self.y - n,
            w: self.w + n * 2,
            h: self.h + n * 2,
        }
    }
}
