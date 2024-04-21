#pragma once

#include "ISystemModule.hpp"
#include "Machine.hpp"

namespace vm {

class GraphicsModuleShim : public vm::ISystemModule {
public:
   enum Fn {
      SET_DISPLAY_BUF = 0,
      IS_KEY_DOWN = 1,
      BLIT = 2,
      IS_KEY_PRESSED = 3,
      CLEAR_SCREEN = 4,
   };

   void invoke_index(vm::Machine& machine, int fn_id) override final {
      switch(fn_id) {
      case SET_DISPLAY_BUF:
         set_display_buf(machine.stack().pop());
         break;
      case IS_KEY_DOWN:
         machine.stack().push(vm::to_stackword(is_key_down(machine.stack().pop()
         )));
         break;
      case IS_KEY_PRESSED:
         machine.stack().push(
            vm::to_stackword(is_key_pressed(machine.stack().pop()))
         );
         break;
      case BLIT: {
         auto spriteptr = machine.stack().pop();
         auto y = machine.stack().pop();
         auto x = machine.stack().pop();
         blit(machine, x, y, spriteptr);
      } break;
      case CLEAR_SCREEN:
         clear_screen(machine);
         break;
      default:
         std::printf("unknown call to graphics: %d\n", fn_id);
         break;
      }
   }

   virtual void set_display_buf(StackWord buff_ptr) = 0;
   virtual bool is_key_down(StackWord key) = 0;
   virtual bool is_key_pressed(StackWord key) = 0;
   virtual void blit(
      vm::Machine& m, StackWord x, StackWord y, StackWord sprite_ptr
   ) = 0;
   virtual void clear_screen(vm::Machine& m) = 0;

   GraphicsModuleShim() : vm::ISystemModule("graphics") {}

private:
};

} // namespace vm