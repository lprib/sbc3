#include "RaylibGraphicsModule.hpp"
#include "raylib.h"

#include <algorithm>
#include <array>
#include <iostream>

void RaylibGraphicsModule::set_display_buf(vm::StackWord buff_ptr) {
   m_display_buff_bytecode_address = buff_ptr;
}

bool RaylibGraphicsModule::is_key_down(vm::StackWord key) {
   return IsKeyDown(key);
}
bool RaylibGraphicsModule::is_key_pressed(vm::StackWord key) {
   return IsKeyPressed(key);
}
void RaylibGraphicsModule::blit(
   vm::Machine& m, vm::StackWord x, vm::StackWord y, vm::StackWord sprite_ptr
) {

   auto code = m.current_module().code();

   auto sprite_width = code[sprite_ptr];
   auto sprite_height = code[sprite_ptr + 1];
   auto sprite = code.subspan(sprite_ptr + 2, sprite_width * sprite_height);

   // std::cout << "blit x:" << x << " y:" << y << " ptr:" << sprite_ptr << "
   // w:"
   //           << sprite_width << " h:" << sprite_height << "\n";

   for(int y_iter = 0; y_iter < sprite_height; ++y_iter) {
      auto dest_y = y + y_iter;
      auto src_row = sprite.subspan(y_iter * sprite_width, sprite_width);
      auto dest_row = code.subspan(
         m_display_buff_bytecode_address + dest_y * SCREEN_WIDTH + x,
         sprite_width
      );
      std::copy(src_row.begin(), src_row.end(), dest_row.begin());
   }
}
void RaylibGraphicsModule::clear_screen(vm::Machine& m) {
   auto display = m.current_module().code().subspan(
      m_display_buff_bytecode_address,
      SCREEN_WIDTH * SCREEN_HEIGHT
   );
   std::fill(display.begin(), display.end(), 0);
}

static constexpr std::array<Color, 16> colormap = {
   Color{0, 0, 0, 0xff},
   Color{17, 14, 0, 0xff},
   Color{34, 28, 0, 0xff},
   Color{51, 43, 0, 0xff},
   Color{68, 57, 0, 0xff},
   Color{85, 71, 0, 0xff},
   Color{102, 86, 0, 0xff},
   Color{119, 100, 0, 0xff},
   Color{136, 114, 0, 0xff},
   Color{153, 129, 0, 0xff},
   Color{170, 143, 0, 0xff},
   Color{187, 157, 0, 0xff},
   Color{204, 172, 0, 0xff},
   Color{221, 186, 0, 0xff},
   Color{238, 200, 0, 0xff},
   Color{255, 215, 0, 0xff},
};

void RaylibGraphicsModule::draw(vm::Machine& machine) {
   auto progmem = machine.current_module().code();
   for(int y = 0; y < SCREEN_HEIGHT; ++y) {
      for(int x = 0; x < SCREEN_WIDTH; ++x) {
         auto pix =
            progmem[m_display_buff_bytecode_address + (y * SCREEN_WIDTH + x)];
         DrawRectangle(
            x * PX_PADDED_SIZE,
            y * PX_PADDED_SIZE,
            PX_SIZE,
            PX_SIZE,
            colormap[pix]
         );
      }
   }
}