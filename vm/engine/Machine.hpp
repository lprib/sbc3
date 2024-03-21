#pragma once

#include <vector>

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

class Machine {
public:
   Machine() :
      m_stack(STACK_SIZE),
      m_return_stack(RETURN_STACK_SIZE),
      m_pc(0) {}

private:
   static constexpr int STACK_SIZE = 0x100;
   static constexpr int RETURN_STACK_SIZE = 0x100;

   Stack m_stack;
   Stack m_return_stack;
   // TODO how to handle external calls?
   // Push module ID and PC to stack?
   // When returning from a function, how can the function know if it's returing
   // across a module boundary or just a normal within-module return?
   int m_pc;
   std::vector<Module> m_modules;
};

} // namespace vm