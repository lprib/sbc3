#include <cstdio>

#include "Machine.hpp"
namespace vm {

#define MACHINE_TRACE (1)

#if MACHINE_TRACE
#define trace(fmt, ...) std::printf("machine_trace: " fmt "\n", ##__VA_ARGS__)
#else
#define trace(...)
#endif

enum Instruction {
   I_NOP = 0,
   I_ADD = 1,
   I_SUB = 2,
   I_MUL = 3,
   I_DIV = 4,
   I_CALL_IMM = 18,
   I_PUSH_IMM = 28,
   I_RETURN = 23,
   I_LOAD_MODULE = 24,
   I_EXTERN_CALL = 25,
   I_LOAD_WORD = 26,
   I_STORE_WORD = 27,
};

std::optional<Error> Machine::execute(std::string_view module_name) {
   m_errorno = std::nullopt;

   auto index = get_or_load_module(module_name);
   if(index < 0) {
      return Error::ModuleNotFound;
   }

   m_current_module_idx = index;
   m_pc = m_modules[m_current_module_idx].code_start_index();

   while(instr()) {
   }

   return m_errorno;
}

#define BINARY_OP(_opcode, _op)                                                \
   case _opcode: {                                                             \
      auto l = m_stack.pop();                                                  \
      auto r = m_stack.pop();                                                  \
      trace(#_opcode " %hu %hu", l, r);                                        \
      m_stack.push(l _op r);                                                   \
   } break

bool Machine::instr() {
   auto instr = pop_progmem();
   switch(instr) {
   case I_NOP:
      trace("I_NOP");
      break;

      BINARY_OP(I_ADD, +);
      BINARY_OP(I_SUB, -);
      BINARY_OP(I_MUL, *);
      BINARY_OP(I_DIV, /);

   case I_CALL_IMM: {
      auto dest = pop_progmem_word();
      m_return_stack.push(m_pc);
      trace("I_CALL_IMM %hu", dest);
      m_pc = dest;
   } break;
   case I_PUSH_IMM: {
      auto imm = pop_progmem_word();
      trace("I_PUSH_IMM %hu", imm);
      m_stack.push(imm);
   } break;
   case I_RETURN: {
      auto caller = m_return_stack.pop();
      trace("I_RETURN %hu", caller);
      // TODO inter-module return
      m_pc = caller;
   } break;
   case I_LOAD_MODULE: {
      // TODO unsafe as fuck
      auto name = std::string_view(
         reinterpret_cast<char const*>(&current_code().data()[m_pc])
      );
      auto index = get_or_load_module(name);
      trace("I_LOAD_MODULE %s -> %d", name, index);
      if(index < 0) {
         m_errorno = Error::ModuleNotFound;
         return false;
      }
      m_stack.push(index);
   } break;
   case I_EXTERN_CALL: {
      auto fn_id = m_stack.pop();
      auto module_id = m_stack.pop();
      // TODO
   } break;
   case I_LOAD_WORD: {
      auto address = m_stack.pop();
      auto& code = current_code();
      auto val = code[address] | (code[address + 1] << 8);
      m_stack.push(val);
   } break;
   case I_STORE_WORD: {
   } break;
   }
};

int Machine::get_or_load_module(std::string_view name) {
   auto idx = module_index_by_name(name);
   if(idx < 0) {
      return idx;
   } else {
      // not found, we need to load it
      auto mod = m_platform.get_module(name);
      if(mod.has_value()) {
         m_modules.push_back(mod.value());
         return m_modules.size() - 1;
      } else {
         // platform couldn't load it
         return -1;
      }
   }
}

} // namespace vm