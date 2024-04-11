#pragma once

#include "ISystemModule.hpp"
#include "Machine.hpp"

class GraphicsModule final : public vm::ISystemModule {
public:
   GraphicsModule(const GraphicsModule&) = delete;
   GraphicsModule& operator=(const GraphicsModule&) = delete;
   static GraphicsModule& instance() {
      static GraphicsModule inst;
      return inst;
   }

   void invoke_index(vm::Machine& machine, int fn_id) override;
   void draw(vm::Machine& machine);

   static constexpr int SCREEN_WIDTH = 256;
   static constexpr int SCREEN_HEIGHT = 64;
   static constexpr int PX_SIZE = 4;
   static constexpr int PX_PADDED_SIZE = 5;
   static constexpr int SCREEN_WIDTH_PX = SCREEN_WIDTH * PX_PADDED_SIZE;
   static constexpr int SCREEN_HEIGHT_PX = SCREEN_HEIGHT * PX_PADDED_SIZE;

private:
   int m_display_buff_bytecode_address = 0;

   GraphicsModule() : vm::ISystemModule("graphics") {}
};