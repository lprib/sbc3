#pragma once

#include "ISystemModule.hpp"
#include "Machine.hpp"

namespace vm {

class BaseModuleShim : public vm::ISystemModule {
public:
   enum Fn {
      PRINT_DECIMAL = 0,
      PROFILE_START = 1,
      PROFILE_END = 2,
   };

   void invoke_index(vm::Machine& machine, int fn_id) override final {
      switch(fn_id) {
      case PRINT_DECIMAL:
         print_decimal(machine.stack().pop());
         break;
      case PROFILE_START:
         profile_start(machine.stack().pop());
         break;
      case PROFILE_END:
         profile_end(machine.stack().pop());
         break;
      default:
         // TODO platform agnostic logging
         // std::printf("unknown call to base: %d\n", fn_id);
         break;
      }
   }

   virtual void print_decimal(StackWord word) = 0;
   virtual void profile_start(StackWord id) = 0;
   virtual void profile_end(StackWord id) = 0;

   BaseModuleShim() : vm::ISystemModule("base") {}

private:
};

} // namespace vm