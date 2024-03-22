#pragma once

#include <optional>
#include <vector>

#include "BytecodeModule.hpp"
#include "IPlatform.hpp"
#include "ISystemModule.hpp"
#include "Stack.hpp"

namespace vm {

using StackWord = short;

class Machine {
public:
   Machine(IPlatform& platform) :
      m_stack(STACK_SIZE),
      m_return_stack(RETURN_STACK_SIZE),
      m_pc(0),
      m_platform(platform) {}

   std::optional<Error> execute(std::string_view module_name);

   /// @brief Add system module
   /// @param system_module Module to add. Reference must outlive this Machine
   void add_system_module(ISystemModule* system_module) {
      m_system_modules.push_back(system_module);
   }

   /// @brief Get index of module from name. Does not load it if it doesn't
   /// exist
   /// @param name
   /// @return index if found, otherwise -1
   int module_index_by_name(std::string_view name) {
      for(int i = 0; i < m_modules.size(); ++i) {
         if(m_modules[i].name() == name) {
            return i;
         }
      }
      return -1;
   }

   /// @brief get the module, or load it from platform
   /// @param name name
   /// @return index if found, otherwise -1
   int get_or_load_module(std::string_view name);

private:
   static constexpr int STACK_SIZE = 0x100;
   static constexpr int RETURN_STACK_SIZE = 0x100;

   Stack<StackWord> m_stack;
   Stack<StackWord> m_return_stack;
   int m_pc;
   int m_current_module_idx = -1;

   std::vector<BytecodeModule> m_modules;
   std::vector<ISystemModule*> m_system_modules;

   IPlatform& m_platform;
   std::optional<Error> m_errorno;

   bool instr();

   std::vector<unsigned char>& current_code() {
      return m_modules[m_current_module_idx].code();
   }

   unsigned char pop_progmem() {
      // todo that's a lot of lookup for one bit of program memory
      auto out = m_modules[m_current_module_idx].code_byte(m_pc);
      ++m_pc;
      return out;
   }

   StackWord pop_progmem_word() {
      auto& code = current_code();
      // little endian
      StackWord out =
         static_cast<StackWord>(code[m_pc] | (code[m_pc + 1] << 8));
      m_pc += 2;
      return out;
   }
};

} // namespace vm