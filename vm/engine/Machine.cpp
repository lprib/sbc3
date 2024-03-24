#include <cstdio>
#include <iostream>

#include "Machine.hpp"
namespace vm {

#define MACHINE_TRACE (0)

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
   I_MOD = 5,
   I_SHR = 6,
   I_SHL = 7,
   I_INVERT = 8,
   I_GT = 9,
   I_LT = 10,
   I_GE = 11,
   I_LE = 12,
   I_EQ = 13,
   I_NEQ = 14,
   I_JUMP_IMM = 16,
   I_CALL_IMM = 18,
   I_BTRUE_IMM = 20,
   I_BFALSE_IMM = 22,
   I_RETURN = 23,
   I_LOAD_MODULE = 24,
   I_EXTERN_CALL = 25,
   I_LOAD_WORD = 26,
   I_STORE_WORD = 27,
   I_PUSH_IMM = 28,
   I_DUP = 29,
   I_SWAP = 30,
   I_DROP = 31,
   I_RPUSH = 38,
   I_RPOP = 39,
   I_RCOPY = 40,
   I_INC = 41,
   I_RCOPY2 = 43,
   I_LOAD_BYTE = 44,
   I_STORE_BYTE = 45,
};

std::optional<Error> Machine::execute_first_module() {
   if(m_modules.size() == 0) {
      return Error::ModuleNotFound;
   }
   return execute_by_index(0, "entry");
}

std::optional<Error> Machine::execute(
   std::string_view module_name, std::string_view fn_name
) {
   m_errorno = std::nullopt;

   auto index = get_or_load_module(module_name);
   if(index < 0) {
      return Error::ModuleNotFound;
   }

   return execute_by_index(index, fn_name);
}

std::optional<Error> Machine::execute_by_index(
   int module_index, std::string_view fn_name
) {
   m_current_module_idx = module_index;
   auto entry = current_module().get_export(fn_name);
   if(!entry.has_value()) {
      return Error::EntryNotFound;
   }

   m_pc = entry.value().bytecode_offset;

   while(instr()) {
   }

   return m_errorno;
}

#define BINARY_OP(_opcode, _op)                                                \
   case _opcode: {                                                             \
      auto r = m_stack.pop();                                                  \
      auto l = m_stack.pop();                                                  \
      trace(#_opcode " %hu %hu", l, r);                                        \
      m_stack.push(l _op r);                                                   \
   } break

#define COMPARISON_OP(_opcode, _op)                                            \
   case _opcode: {                                                             \
      auto r = m_stack.pop();                                                  \
      auto l = m_stack.pop();                                                  \
      trace(#_opcode " %d %d", l, r);                                          \
      m_stack.push((l _op r) ? TRUE_WORD : FALSE_WORD);                        \
   } break

bool Machine::instr() {
   if(m_pc >= current_code().size()) {
      m_errorno = Error::EofWithoutReturn;
      return false;
   }

   // trace("exe %d", m_pc);
   auto instr = pop_progmem();
   switch(instr) {
   case I_NOP:
      trace("I_NOP");
      break;

      BINARY_OP(I_ADD, +);
      BINARY_OP(I_SUB, -);
      BINARY_OP(I_MUL, *);
      BINARY_OP(I_DIV, /);
      BINARY_OP(I_MOD, %);
      BINARY_OP(I_SHR, >>);
      BINARY_OP(I_SHL, <<);

      COMPARISON_OP(I_GT, >);
      COMPARISON_OP(I_LT, <);
      COMPARISON_OP(I_GE, >=);
      COMPARISON_OP(I_LE, <=);
      COMPARISON_OP(I_EQ, ==);
      COMPARISON_OP(I_NEQ, !=);

   case I_JUMP_IMM: {
      auto dest = pop_progmem_word();
      trace("I_JUMP_IMM %hu", dest);
      m_pc = dest;
   } break;
   case I_CALL_IMM: {
      auto dest = pop_progmem_word();
      m_return_stack.push(m_pc);
      trace("I_CALL_IMM %hu", dest);
      m_pc = dest;
   } break;
   case I_BTRUE_IMM: {
      auto dest = pop_progmem_word();
      auto test = m_stack.pop();
      trace("I_BTRUE_IMM %hu (test %hu)", dest, test);
      if(test) {
         m_pc = dest;
      }
   } break;
   case I_BFALSE_IMM: {
      auto dest = pop_progmem_word();
      auto test = m_stack.pop();
      trace("I_BFALSE_IMM %hu (test %hu)", dest, test);
      if(!test) {
         m_pc = dest;
      }
   } break;
   case I_RETURN: {
      if(m_return_stack.item_count() == 0) {
         // top level return
         trace("I_RETURN toplevel");
         return false;
      }
      auto caller = m_return_stack.pop();
      trace("I_RETURN %hu", caller);
      // TODO inter-module return
      m_pc = caller;
   } break;
   case I_LOAD_MODULE: {
      // TODO unsafe as fuck
      auto name_ptr = m_stack.pop();
      auto name_cstr =
         reinterpret_cast<char const*>(&current_code().data()[name_ptr]);
      auto name = std::string_view(name_cstr);
      trace("I_LOAD_MODULE `%s`", name_cstr);
      auto index = get_or_load_module(name);
      trace("   -> %d", index);
      if(index < 0) {
         m_errorno = Error::ModuleNotFound;
         return false;
      }
      m_stack.push(index);
   } break;
   case I_EXTERN_CALL: {
      auto fn_id = m_stack.pop();
      auto module_id = m_stack.pop();
      if(module_id & SYSTEM_MODULE_MASK) {
         // system module
         auto module_index = module_id & (~SYSTEM_MODULE_MASK);
         trace("I_EXTERN_CALL SYSTEM %d %d", module_index, fn_id);
         m_system_modules[module_index]->invoke_index(*this, fn_id);
      } else {
         // bytecode module
         trace("unimpl");
         return false;
      }
   } break;
   case I_LOAD_WORD: {
      auto address = m_stack.pop();
      auto code = current_code();
      trace("I_LOAD_WORD %hu", address);
      auto val = code[address] | (code[address + 1] << 8);
      trace("   -> %hu", val);
      m_stack.push(val);
   } break;
   case I_STORE_WORD: {
      auto address = m_stack.pop();
      auto value = m_stack.pop();
      auto code = current_code();
      trace("I_STORE_WORD %d <- %d", address, value);
      code[address] = value & 0xff;
      code[address + 1] = value >> 8;
   } break;
   case I_PUSH_IMM: {
      auto imm = pop_progmem_word();
      trace("I_PUSH_IMM %hu", imm);
      m_stack.push(imm);
   } break;
   case I_DUP: {
      trace("I_DUP");
      m_stack.push(m_stack.peek());
   } break;
   case I_SWAP: {
      trace("I_SWAP");
      auto a = m_stack.pop();
      auto b = m_stack.pop();
      m_stack.push(a);
      m_stack.push(b);
   } break;
   case I_DROP: {
      trace("I_DROP");
      m_stack.pop();
   } break;
   case I_RPUSH: {
      trace("I_RPUSH");
      auto n = m_stack.pop();
      m_return_stack.push(n);
   } break;
   case I_RPOP: {
      trace("I_RPOP");
      auto n = m_return_stack.pop();
      m_stack.push(n);
   } break;
   case I_RCOPY: {
      trace("I_RCOPY");
      auto n = m_return_stack.peek();
      m_stack.push(n);
   } break;
   case I_INC: {
      trace("I_INC");
      m_stack.push(m_stack.pop() + 1);
   } break;
   case I_RCOPY2: {
      trace("I_RCOPY2");
      auto top = m_return_stack.peek_n(0);
      auto second = m_return_stack.peek_n(1);
      m_stack.push(second);
      m_stack.push(top);
   } break;
   case I_LOAD_BYTE: {
      auto address = m_stack.pop();
      auto code = current_code();
      trace("I_LOAD_BYTE %hu", address);
      auto val = code[address];
      trace("   -> %hu", val);
      m_stack.push(val);
   } break;
   case I_STORE_BYTE: {
      auto address = m_stack.pop();
      auto value = m_stack.pop() & 0xff;
      auto code = current_code();
      trace("I_STORE_BYTE %d <- %d", address, value);
      code[address] = value;
   } break;
   default: {
      trace("unknown opcode: %d", instr);
      return false;
   }
   }
   return true;
};

int Machine::module_index_by_name(std::string_view name) {
   // search system modules first
   for(int i = 0; i < m_system_modules.size(); ++i) {
      if(m_system_modules[i]->name() == name) {
         return SYSTEM_MODULE_MASK | i;
      }
   }

   for(int i = 0; i < m_modules.size(); ++i) {
      if(m_modules[i].name() == name) {
         return i;
      }
   }
   return -1;
}

int Machine::get_or_load_module(std::string_view name) {
   auto idx = module_index_by_name(name);
   if(idx >= 0) {
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