#pragma once

#include <vector>

#include "ISystemModule.hpp"
#include "Module.hpp"

namespace vm {

using StackWord = short;

class Stack {
public:
   Stack(int size) : m_stack(size, 0), m_sp(0) {}

   StackWord peek() const {
      return m_stack[m_sp - 1];
   }

   StackWord peek_n(int n) const {
      return m_stack[m_sp - 1 - n];
   }

   StackWord pop() {
      --m_sp;
      return m_stack[m_sp];
   }

   void push(StackWord n) {
      m_stack[m_sp] = n;
      ++m_sp;
   }

private:
   std::vector<StackWord> m_stack;
   int m_sp = 0;
};

using LoadModuleCallback = std::optional<Module> (*)(std::string_view name);

class Machine {
public:
   Machine(LoadModuleCallback load_module_callback) :
      m_stack(STACK_SIZE),
      m_return_stack(RETURN_STACK_SIZE),
      m_pc(0),
      m_load_module_callback(load_module_callback) {}

   /// @brief Add system module
   /// @param system_module Module to add. Reference must outlive this Machine
   void add_system_module(ISystemModule& system_module) {
      m_system_modules.push_back(system_module);
   }

private:
   static constexpr int STACK_SIZE = 0x100;
   static constexpr int RETURN_STACK_SIZE = 0x100;

   Stack m_stack;
   Stack m_return_stack;
   int m_pc;

   // TODO: when interpreting `loadmodule`, we need some way to differentiate
   // between a user module ID and a system module ID. either encode as a high
   // bit, or have a single array holding all and generalize IModule to cover
   // both types.

   std::vector<Module> m_modules;
   std::vector<ISystemModule&> m_system_modules;

   LoadModuleCallback m_load_module_callback;
};

} // namespace vm