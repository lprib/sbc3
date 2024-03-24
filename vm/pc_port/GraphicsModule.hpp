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

private:
   int m_display_buff_bytecode_address = 0;

   GraphicsModule() : vm::ISystemModule("graphics") {}
};