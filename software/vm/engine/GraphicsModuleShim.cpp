#include "GraphicsModuleShim.hpp"

namespace vm {

void GraphicsModuleShim::blit(
   Machine& m, StackWord x, StackWord y, StackWord sprite_ptr
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

} // namespace vm