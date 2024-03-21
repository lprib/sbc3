#include "engine.hpp"

#include <cstdio>
#include <cstring>

namespace engine {

#define ENGINE_TRACE (1)
#if ENGINE_TRACE
#define engine_trace(fmt, ...)                                                 \
   std::printf("engine_trace: " fmt "\n", ##__VA_ARGS__)
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
   LOADWORD,
   STOREWORD,
   PUSH_IMM,
   LOADMODULE,
   LOADBYTE,
   STOREBYTE,
   JUMP_IMM,
   CALL_IMM,
};

#define BINOP(_opcode, _op)                                                    \
   case Opcode::_opcode: {                                                     \
      auto r = m_stack.pop();                                                  \
      auto l = m_stack.pop();                                                  \
      engine_trace(#_opcode " %hu %hu", l, r);                                 \
      m_stack.push(r _op l);                                                   \
   } break

static constexpr StackWord SYSTEM_MOD_ID = 0;

void Engine::execute_module(std::span<unsigned char const> bytecode) {
   m_bytecode = bytecode;

   parse_module_header();

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

         BINOP(ADD, +);
         BINOP(SUB, -);
         BINOP(MUL, *);
         BINOP(DIV, /);

      case Opcode::IF:
         break;
      case Opcode::JUMP: {
         auto to = m_stack.pop();
         engine_trace("jump %hu", to);
         m_pc = to;
      } break;
      case Opcode::JUMP_IMM: {
         auto to = nextDataWord();
         engine_trace("jump_imm %hu", to);
         m_pc = to;
      } break;

      case Opcode::CALL: {
         auto dest = m_stack.pop();
         m_return_stack.push(m_pc);
         engine_trace("call %hu", dest);
         m_pc = dest;
      } break;
      case Opcode::CALL_IMM: {
         auto dest = nextDataWord();
         m_return_stack.push(m_pc);
         engine_trace("call_imm %hu", dest);
         m_pc = dest;
      } break;
      case Opcode::RETURN:
         if(m_rsp == 0) {
            engine_trace("return toplevel");
            return;
         } else {
            auto parent = m_return_stack.pop();
            engine_trace("return %hu", parent);
            m_pc = parent;
         }
         break;
      case Opcode::EXTERN_CALL: {
         auto fn_number = m_stack.pop();
         auto module = m_stack.pop();
         engine_trace("extern_call mod=%hu fn_number=%hu", module, fn_number);
         if(module == SYSTEM_MOD_ID) {
            if(fn_number < m_intrinsics.size()) {
               auto intrinsic = m_intrinsics[fn_number];
               if(intrinsic != nullptr) {
                  intrinsic(*this);
               }
            }
         }
      } break;
      case Opcode::LOADBYTE: {
         auto addr = m_stack.pop();
         engine_trace("loadbyte %hu", addr);
         m_stack.push(m_bytecode[addr]);
      } break;
      case Opcode::LOADWORD: {
         auto addr = m_stack.pop();
         engine_trace("loadword %hu", addr);
         m_stack.push(m_bytecode[addr] | (m_bytecode[addr + 1] << 8));
      } break;
      case Opcode::STOREBYTE: {
         auto location = m_stack.pop();
         auto value = m_stack.pop();
         m_bytecode[location] = value & 0xff;
      } break;
      case Opcode::STOREWORD: {
         auto location = m_stack.pop();
         auto value = m_stack.pop();
         m_bytecode[location] = value & 0xff;
         m_bytecode[location + 1] = (value >> 8) & 0xff;
      } break;

      case Opcode::PUSH_IMM: {
         auto n = nextDataWord();
         engine_trace("push_imm %hu", n);
         m_stack.push(n);
      } break;
      case Opcode::LOADMODULE: {
         // TODO(liam) safety
         auto module_name_ptr = m_stack.pop();
         auto unsafe_module_name =
            reinterpret_cast<char const*>(&m_bytecode.data()[module_name_ptr]);

         engine_trace("loadmodule name='%s'", unsafe_module_name);

         if(std::strncmp(unsafe_module_name, "system", 7) == 0) {
            m_stack.push(SYSTEM_MOD_ID);
         } else {
            m_stack.push(88);
            engine_trace("(loadmodule resolved=%d)", 88);
            // TODO: load modules
         }

      } break;
      default:
         engine_trace("<unknown %d>", op);
         break;
      }
   }
}

void Engine::parse_module_header()
{

}

} // namespace engine