#pragma once

#include "GraphicsModuleCommon.hpp"
#include "Machine.hpp"

class RaylibGraphicsModule final : public vm::GraphicsModuleCommon {
public:
   void draw(vm::Machine& machine);

   static constexpr int PX_SIZE = 5;
   static constexpr int PX_PADDED_SIZE = 5;
   static constexpr int SCREEN_WIDTH_PX = SCREEN_WIDTH * PX_PADDED_SIZE;
   static constexpr int SCREEN_HEIGHT_PX = SCREEN_HEIGHT * PX_PADDED_SIZE;

   bool is_key_down(vm::StackWord key) override;
   bool is_key_pressed(vm::StackWord key) override;
};