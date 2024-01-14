#!/usr/bin/python3
import sys
import math

def rindex(lst, value):
    return len(lst) - 1 - lst[::-1].index(value)


SPACE_WIDTH = 2

class Glyph:
    def __init__(self, atlas, char):
        self.char = char
        self.ascii = ord(char)
        assert self.ascii < 128

        if char == ' ':
            self.stride_bytes = 0
            self.sprite = []
            self.origin_y_offset = 0
            self.next_char_offset_x = SPACE_WIDTH + 1
            return


        # font input is 16 sections wide, 8 high.
        # indexed by ascii value in reading direction
        section_x = self.ascii % 16
        section_y = self.ascii // 16

        pixel_x = section_x*16
        pixel_y = section_y*16
        # section is the 16x16 area that contains the glyph
        # glyph will be offset in this area to show origin offset and kerning
        self.section = Glyph.subsprite(atlas, pixel_x, pixel_x+16, pixel_y, pixel_y+16)
        self.detect_bounding_box()

        self.origin_x_offset = 0 # forced to always zero in this specification
        baseline_offset_in_section = 8
        self.origin_y_offset = baseline_offset_in_section - self.sprite_y_offset

        virtual_next_char_y = 8
        self.next_char_offset_x = virtual_next_char_y - self.sprite_x_offset
    
    def subsprite(main_tex, min_x, max_x, min_y, max_y):
        return [row[min_x:max_x] for row in main_tex[min_y:max_y]]

    def detect_bounding_box(self):
        # detect BB of set pixels
        min_x = min(row.index(True) for row in self.section if any(row))
        max_x_inclusive = max(rindex(row, True) for row in self.section if any(row))
        min_y = [any(row) for row in self.section].index(True)
        max_y_inclusive = rindex([any(row) for row in self.section], True)

        self.sprite = Glyph.subsprite(self.section, min_x, max_x_inclusive+1, min_y, max_y_inclusive+1)
        self.sprite_x_offset = min_x
        self.sprite_y_offset = min_y

        width = len(self.sprite[0])
        self.stride_bytes = math.ceil(width / 8)
    
    def to_bytearray(self):
        b = bytearray()
        b.append(self.stride_bytes)
        b.append(len(self.sprite))
        b.append(self.origin_y_offset)
        b.append(self.next_char_offset_x)
        for row in self.sprite:
            b.extend(self.bitpack_byte_aligned(row))
        return b
    
    def bitpack_byte_aligned(self, row):
        b = bytearray()
        for byte_idx in range(0, self.stride_bytes):
            bits = row[byte_idx*8:((byte_idx+1)*8)]
            byte = 0

            # note we store the leftmost pixel in bit 0 (least significant)
            for i, bit in enumerate(bits):
                if(bit):
                    byte |= 1 << i
            b.append(byte)
        return b


    def __str__(self):
        out = ""
        out += f"char: {self.char}\n"
        out += f"sprite:\n  stride: {self.stride_bytes}\n  y_off: {self.origin_y_offset}\n  next_x: {self.next_char_offset_x}\n"
        for row in self.sprite:
            for cell in row:
                out += "@" if cell else "."
            out += "\n"
        return out


def make_font(atlas: list[list[bool]]):
    b = bytearray()
    n_chars = 0x7f-0x20
    header_entry_size = 4 # 32-bit LE offset
    header_size = n_chars * header_entry_size;
    b.extend(bytearray(header_size))
    for i in range(0x20, 0x7f):
        current_offset = len(b)

        header_idx = (i - 0x20) * 4 # we dont reserve header space for control chars (below 0x20)
        # add 32bit le
        b[header_idx + 0] = (current_offset >> 0) & 0xff;
        b[header_idx + 1] = (current_offset >> 8) & 0xff;
        b[header_idx + 2] = (current_offset >> 16) & 0xff;
        b[header_idx + 3] = (current_offset >> 24) & 0xff;
        
        glyph = Glyph(atlas, chr(i))
        # print(glyph)
        b.extend(glyph.to_bytearray())
    return b


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("expected [INPUT .pbm] [OUTPUT .bin]");

    lines = open(sys.argv[1]).readlines();

    assert lines[0].strip() == "P1", "image should be ASCII (uncompressed) Portable BitMap"
    assert lines[1].strip() == "256 128", "image should be 256x128"
    assert len([l for l in lines if len(l.strip()) != 0]) == 128 + 2, "image should have 128 rows of data and 2 rows of header"

    def to_bool(s):
        return True if s == "1" else False

    bits = [[to_bool(cell) for cell in line.split()] for line in lines[2:]]

    font_data = make_font(bits)

    with open(sys.argv[2], "wb") as outfile:
        outfile.write(font_data)
    
    print("written font data")