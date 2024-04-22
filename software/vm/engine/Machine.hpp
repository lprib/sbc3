#pragma once

#include <optional>
#include <vector>

#include "BytecodeModule.hpp"
#include "IPlatform.hpp"
#include "ISystemModule.hpp"
#include "Stack.hpp"

namespace vm {

using StackWord = short;

static constexpr StackWord TRUE_WORD = 0xffff;
static constexpr StackWord FALSE_WORD = 0;

inline constexpr StackWord to_stackword(bool boolean) {
   return boolean ? TRUE_WORD : FALSE_WORD;
}

class Machine {
public:
   Machine(IPlatform& platform) :
      m_stack(STACK_SIZE),
      m_return_stack(RETURN_STACK_SIZE),
      m_pc(0),
      m_platform(platform) {}

   std::optional<Error> execute(
      std::string_view module_name, std::string_view fn_name
   );

   /// @brief Add system module
   /// @param system_module Module to add. Reference must outlive this Machine
   void add_system_module(ISystemModule* system_module) {
      m_system_modules.push_back(system_module);
   }

   void add_module(BytecodeModule module) {
      m_modules.push_back(std::move(module));
   }

   /// @brief Get index of module from name. Does not load it if it doesn't
   /// exist
   /// @param name
   /// @return index if found, otherwise -1
   int module_index_by_name(std::string_view name);

   /// @brief get the module, or load it from platform
   /// @param name name
   /// @return index if found, otherwise -1
   int get_or_load_module(std::string_view name);

   Stack<StackWord>& stack() {
      return m_stack;
   }

   BytecodeModule& current_module() {
      return m_modules[m_current_module_idx];
   }

   BytecodeModule& module_by_index(int index) {
      return m_modules[index];
   }

   std::vector<std::vector<unsigned char>>& sbufs() {
      return m_sbufs;
   }

private:
   static constexpr int STACK_SIZE = 0x100;
   static constexpr int RETURN_STACK_SIZE = 0x100;

   // Don't want to touch the sign bit for 16-bit ints since we use negative
   // numbers as module not found.
   static constexpr int SYSTEM_MODULE_MASK = 0x4000;

   static constexpr StackWord EXTERNAL_RETURN_BIT = 0x8000;

   Stack<StackWord> m_stack;
   Stack<StackWord> m_return_stack;
   int m_pc;
   int m_current_module_idx = -1;

   std::vector<BytecodeModule> m_modules;
   std::vector<ISystemModule*> m_system_modules;

   // sbufs: shared buffers. Userland interface is defined in
   // BaseModuleCommon.hpp
   std::vector<std::vector<unsigned char>> m_sbufs;

   IPlatform& m_platform;
   std::optional<Error> m_errorno;

   bool instr();

   std::span<unsigned char> current_code() {
      return current_module().code();
   }

   unsigned char pop_progmem() {
      // todo that's a lot of lookup for one bit of program memory
      auto out = current_code()[m_pc];
      ++m_pc;
      return out;
   }

   StackWord pop_progmem_word() {
      auto code = current_code();
      // little endian
      StackWord out =
         static_cast<StackWord>(code[m_pc] | (code[m_pc + 1] << 8));
      m_pc += 2;
      return out;
   }

   std::optional<Error> execute_by_index(
      int module_index, std::string_view fn_name
   );

   static bool is_external_return_addr(StackWord return_address) {
      return return_address & EXTERNAL_RETURN_BIT;
   }
};

} // namespace vm