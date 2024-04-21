#pragma once

#include "GraphicsModuleShim.hpp"
#include "Machine.hpp"

class RaylibGraphicsModule final : public vm::GraphicsModuleShim {
public:
   void draw(vm::Machine& machine);

   static constexpr int SCREEN_WIDTH = 256;
   static constexpr int SCREEN_HEIGHT = 64;
   static constexpr int PX_SIZE = 5;
   static constexpr int PX_PADDED_SIZE = 5;
   static constexpr int SCREEN_WIDTH_PX = SCREEN_WIDTH * PX_PADDED_SIZE;
   static constexpr int SCREEN_HEIGHT_PX = SCREEN_HEIGHT * PX_PADDED_SIZE;

   void set_display_buf(vm::StackWord buff_ptr) override;
   bool is_key_down(vm::StackWord key) override;
   bool is_key_pressed(vm::StackWord key) override;
   void blit(
      vm::Machine& m, vm::StackWord x, vm::StackWord y, vm::StackWord sprite_ptr
   ) override;
   void clear_screen(vm::Machine& m) override;

private:
   int m_display_buff_bytecode_address = 0;
};