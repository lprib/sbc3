#pragma once

#include "ISystemModule.hpp"
#include "Machine.hpp"

namespace vm {

class BaseModuleCommon : public vm::ISystemModule {
public:
   enum Fn {
      PRINT_DECIMAL = 0,
      PROFILE_START = 1,
      PROFILE_END = 2,
      SBUF_NEW = 3,
      SBUF_DELETE = 4,
      SBUF_COPYTO = 5,
      SBUF_COPYFROM = 6,
      SBUF_WRITEBYTE = 7,
      SBUF_READBYTE = 8,
      SBUF_FILL = 9,
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
      case SBUF_NEW:
         machine.stack().push(sbuf_new(machine, machine.stack().pop()));
         break;
      case SBUF_DELETE:
         sbuf_delete(machine, machine.stack().pop());
         break;
      case SBUF_COPYTO: {
         auto sbuf = machine.stack().pop();
         auto dest_start = machine.stack().pop();
         auto len = machine.stack().pop(); // in bytes!
         auto start = machine.stack().pop();
         sbuf_copyto(machine, start, len, dest_start, sbuf);
      } break;
      case SBUF_COPYFROM: {
         auto sbuf = machine.stack().pop();
         auto dest_start = machine.stack().pop();
         auto len = machine.stack().pop(); // in bytes!
         auto start = machine.stack().pop();
         sbuf_copyfrom(machine, start, len, dest_start, sbuf);
      } break;
      case SBUF_WRITEBYTE: {
         auto sbuf = machine.stack().pop();
         auto byte = machine.stack().pop();
         auto index = machine.stack().pop();
         sbuf_writebyte(machine, index, byte, sbuf);
      } break;
      case SBUF_READBYTE: {
         auto sbuf = machine.stack().pop();
         auto index = machine.stack().pop();
         machine.stack().push(sbuf_readbyte(machine, index, sbuf));
      } break;
      case SBUF_FILL: {
         auto sbuf = machine.stack().pop();
         auto byte = machine.stack().pop();
         auto len = machine.stack().pop(); // in bytes!
         auto start = machine.stack().pop();
         sbuf_fill(machine, start, len, byte, sbuf);
      } break;
      default:
         // TODO platform agnostic logging
         // std::printf("unknown call to base: %d\n", fn_id);
         break;
      }
   }

   virtual void print_decimal(StackWord word) = 0;
   virtual void profile_start(StackWord id) = 0;
   virtual void profile_end(StackWord id) = 0;

   StackWord sbuf_new(Machine& m, StackWord len);
   void sbuf_delete(Machine& m, StackWord sbuf);
   void sbuf_copyto(
      Machine& m, StackWord start, StackWord len, StackWord dest_start,
      StackWord sbuf
   );
   void sbuf_copyfrom(
      Machine& m, StackWord start, StackWord len, StackWord dest_start,
      StackWord sbuf
   );
   void sbuf_writebyte(
      Machine& m, StackWord index, StackWord byte, StackWord sbuf
   );
   StackWord sbuf_readbyte(Machine& m, StackWord index, StackWord sbuf);
   void sbuf_fill(
      Machine& m, StackWord start, StackWord len, StackWord byte, StackWord sbuf
   );

   BaseModuleCommon() : vm::ISystemModule("base") {}

private:
};

} // namespace vm