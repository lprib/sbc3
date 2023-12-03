use crate::pixel_buffer::{MonochomeSpriteRef, PixelBuffer};

const FONT_DATA: &'static [u8] = include_bytes!("./font.bin");
pub const NORMAL_FONT: Font<'static> = Font::new(FONT_DATA);

#[derive(Debug)]
pub struct GlyphInfo<'a> {
    baseline_offset: u8,
    advance: u8,
    sprite: MonochomeSpriteRef<'a>,
}

pub struct Font<'a> {
    bin: &'a [u8],
}

impl<'a> Font<'a> {
    pub const fn new(bin: &'a [u8]) -> Self {
        Font { bin }
    }

    pub fn glyph_info(&self, c: u8) -> GlyphInfo {
        let header_idx = (c as usize - 0x20) * 4;

        let glyph_offset = &self.bin[header_idx..(header_idx + 4)];
        let glyph_offset = u32::from_le_bytes(glyph_offset.try_into().unwrap());

        let stride = self.bin[glyph_offset as usize];
        let height = self.bin[(glyph_offset + 1) as usize];
        let baseline_offset = self.bin[(glyph_offset + 2) as usize];
        let advance = self.bin[(glyph_offset + 3) as usize];
        let data_offset = (glyph_offset + 4) as usize;

        let sprite_ref = MonochomeSpriteRef {
            // for now, just assume stride == width since we will draw over
            // empty parts anyway
            width: stride * 8,
            height,
            stride,
            data: &self.bin[data_offset..data_offset + (stride * height) as usize],
        };

        GlyphInfo {
            baseline_offset,
            advance,
            sprite: sprite_ref,
        }
    }

    /// draw a string to pixel buffer. returns width
    pub fn draw<'x, 'y>(
        &'x self,
        pb: &'y mut PixelBuffer,
        string: &str,
        x: i32,
        baseline_y: i32,
        color: u8,
    ) -> i32 {
        let mut cur_x = x;

        // no unicode lol
        for c in string.bytes() {
            let info = self.glyph_info(c);
            let sprite_x = cur_x;
            let sprite_y = baseline_y - info.baseline_offset as i32;
            pb.blit_monochrome(&info.sprite, sprite_x, sprite_y, color);

            cur_x += info.advance as i32;
        }

        cur_x
    }
}
