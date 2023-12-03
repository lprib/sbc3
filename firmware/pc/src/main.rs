use libui::{pixel_buffer::PixelBuffer, Rect};
use pixels::{Pixels, SurfaceTexture, wgpu::Color};
use winit::{
    dpi::LogicalSize,
    event::{Event, WindowEvent},
    event_loop::{ControlFlow, EventLoop},
    window::WindowBuilder,
};
use winit_input_helper::WinitInputHelper;

const WIDTH: u32 = 256;
const HEIGHT: u32 = 64;

// Rgba8UnormSrgb
// ripped from libui/yellow_16_level_grayscale.pal
const COLOR_LOOKUP: [[u8; 4]; 16] = [
    [0, 0, 0, 0xff],
    [17, 14, 0, 0xff],
    [34, 28, 0, 0xff],
    [51, 43, 0, 0xff],
    [68, 57, 0, 0xff],
    [85, 71, 0, 0xff],
    [102, 86, 0, 0xff],
    [119, 100, 0, 0xff],
    [136, 114, 0, 0xff],
    [153, 129, 0, 0xff],
    [170, 143, 0, 0xff],
    [187, 157, 0, 0xff],
    [204, 172, 0, 0xff],
    [221, 186, 0, 0xff],
    [238, 200, 0, 0xff],
    [255, 215, 0, 0xff],
];

fn draw(pb: &mut PixelBuffer) {
    for p in pb.buf.iter_mut() {
        *p = 1;
    }
    pb.rect(Rect::xywh(1, 1, 10, 10), 15);
}

fn main() {
    let mut buf = vec![0u8; 256 * 64];
    let mut pb = PixelBuffer::new(&mut buf, 256, 64, 256);

    draw(&mut pb);

    let event_loop = EventLoop::new().unwrap();
    let mut input = WinitInputHelper::new();

    let window = {
        let size = LogicalSize::new(WIDTH as f64, HEIGHT as f64);
        WindowBuilder::new()
            .with_title("Hello Pixels")
            .with_inner_size(size)
            .with_min_inner_size(size)
            .build(&event_loop)
            .unwrap()
    };

    let mut pixels = {
        let window_size = window.inner_size();
        let surface_texture = SurfaceTexture::new(window_size.width, window_size.height, &window);
        Pixels::new(WIDTH, HEIGHT, surface_texture).unwrap()
    };

    pixels.clear_color(Color::WHITE);

    event_loop.run(move |event, elwt| {
        if let Event::WindowEvent { window_id, event } = &event {
            if *window_id == window.id() {
                match event {
                    WindowEvent::RedrawRequested => {
                        for (i, pixel) in pixels.frame_mut().chunks_exact_mut(4).enumerate() {
                            let gray = pb.buf[i];
                            let gray = if gray > 0xf { 0xf } else { gray };
                            let rgba = COLOR_LOOKUP[gray as usize];
                            pixel.copy_from_slice(&rgba);
                        }
                        pixels.render();
                    }
                    _ => {}
                }
            }
        }
        if input.update(&event) {
            if input.close_requested() {
                elwt.exit();
            }
            if let Some(size) = input.window_resized() {
                pixels.resize_surface(size.width, size.height).unwrap();
            }
            window.request_redraw();
        }
    }).unwrap();
}
