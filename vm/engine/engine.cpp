#include "engine.hpp"

#include <cstdio>
#include <cstring>

namespace engine {

#define ENGINE_TRACE (1)
#if ENGINE_TRACE
#define engine_trace(fmt, ...) std::printf(fmt "\n", ##__VA_ARGS__)
#else
#define engine_trace(...)
#endif

enum Opcode {
   NOP,
   ADD,
   SUB,
   MUL,
   DIV,
   IF,
   JUMP,
   CALL,
   RETURN,
   EXTERN_CALL,
   LOAD,
   STORE,
   PUSH_IMM,
   LOADMODULE
};

static constexpr StackWord SYSTEM_MOD_ID = 0;

void Engine::execute_module(std::span<unsigned char> bytecode) {
   m_bytecode = bytecode;
   m_pc = 0;

   int pc = 0;
   int index = 0;

   while(true) {
      if(m_pc >= m_bytecode.size()) {
         // end of program
         return;
      }
      auto op = m_bytecode[m_pc];
      ++m_pc;

      switch(op) {
      case Opcode::NOP:
         break;
      case Opcode::ADD: {
         auto r = pop();
         auto l = pop();
         push(r + l);
         engine_trace("add %hu %hu", l, r);
      } break;
      case Opcode::SUB: {
         auto r = pop();
         auto l = pop();
         push(r - l);
         engine_trace("sub %hu %hu", l, r);
      } break;
      case Opcode::MUL: {
         auto r = pop();
         auto l = pop();
         push(r * l);
         engine_trace("mul %hu %hu", l, r);
      } break;
      case Opcode::DIV: {
         auto r = pop();
         auto l = pop();
         push(r / l);
         engine_trace("div %hu %hu", l, r);
      } break;
      case Opcode::IF:
         break;
      case Opcode::JUMP: {
         auto to = pop();
         m_pc = to;
      } break;
      case Opcode::CALL:
         break;
      case Opcode::RETURN:
         if(m_rsp == 0) {
            engine_trace("return");
            return;
         }
         break;
      case Opcode::EXTERN_CALL: {
         auto fn_number = pop();
         auto module = pop();
         if(module == SYSTEM_MOD_ID) {
            if(fn_number < m_intrinsics.size()) {
               auto intrinsic = m_intrinsics[fn_number];
               if(intrinsic != nullptr) {
                  intrinsic(*this);
               }
            }
         }
         engine_trace("extern_call mod=%hu fn_number=%hu", module, fn_number);
      } break;
      case Opcode::LOAD: {
         auto addr = pop();
         engine_trace("load %hu", addr);
         push(m_bytecode[addr]);
      } break;
      case Opcode::STORE:
         break;
      case Opcode::PUSH_IMM: {
         push(nextDataWord());
      } break;
      case Opcode::LOADMODULE: {
         // TODO(liam) safety
         auto dest_id_ptr = pop();
         auto module_name_ptr = pop();
         auto unsafe_module_name =
            reinterpret_cast<char const*>(&m_bytecode.data()[module_name_ptr]);

         if(std::strncmp(unsafe_module_name, "system", 6)) {
            m_bytecode[dest_id_ptr] = SYSTEM_MOD_ID;
         } else {
            // TODO: load modules
         }

         engine_trace(
            "loadmodule name=%s dest=%hu",
            unsafe_module_name,
            dest_id_ptr
         );
      } break;
      default:
         engine_trace("<unknown %d>", op);
         break;
      }
   }
}
} // namespace engine