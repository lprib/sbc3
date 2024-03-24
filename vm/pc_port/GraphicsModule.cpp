#include "GraphicsModule.hpp"
#include "raylib.h"

#include <array>
#include <iostream>

enum Fn {
   SET_DISPLAY_BUF = 0,
   IS_KEY_DOWN = 1,
};

void GraphicsModule::invoke_index(vm::Machine& machine, int fn_id) {
   switch(fn_id) {
   case SET_DISPLAY_BUF:
      // ( buffptr -- )
      m_display_buff_bytecode_address = machine.stack().pop();
      break;
   case IS_KEY_DOWN: {
      // ( key -- down? )
      auto key = machine.stack().pop();
      machine.stack().push(
         IsKeyDown(key) ? vm::Machine::TRUE_WORD : vm::Machine::FALSE_WORD
      );
   } break;
   default:
      std::cout << "unknown graphics call " << fn_id << "\n";
   }
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

void GraphicsModule::draw(vm::Machine& machine) {
   auto progmem = machine.module_by_index(0).code();
   for(int y = 0; y < 64; ++y) {
      for(int x = 0; x < 256; ++x) {
         auto pix = progmem[m_display_buff_bytecode_address + (y * 256 + x)];
         DrawRectangle(x * 4, y * 4, 3, 3, colormap[pix]);
      }
   }
}