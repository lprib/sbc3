#include "RaylibGraphicsModule.hpp"
#include "raylib.h"

#include <algorithm>
#include <array>
#include <iostream>

bool RaylibGraphicsModule::is_key_down(vm::StackWord key) {
   return IsKeyDown(key);
}
bool RaylibGraphicsModule::is_key_pressed(vm::StackWord key) {
   return IsKeyPressed(key);
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
   auto& sbuf = machine.sbufs()[m_sbuf_idx];
   for(int y = 0; y < SCREEN_HEIGHT; ++y) {
      for(int x = 0; x < SCREEN_WIDTH; ++x) {
         auto pix = sbuf[y * SCREEN_WIDTH + x];
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