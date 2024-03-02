#pragma once

#include <array>
#include <span>

namespace engine {

using StackWord = short;

class Engine;

using Intrinsic = void (*)(Engine& engine);

class Engine {
public:
   Engine(std::span<Intrinsic> intrinsics) :
      m_intrinsics(intrinsics),
      m_stack(),
      m_return_stack() {}
   void execute_module(std::span<unsigned char> bytecode);

   StackWord pop() {
      --m_sp;
      return m_stack[m_sp];
   }

   void push(StackWord n) {
      m_stack[m_sp] = n;
      ++m_sp;
   }

   StackWord rpop() {
      --m_rsp;
      return m_return_stack[m_rsp];
   }

   void rpush(StackWord n) {
      m_return_stack[m_rsp] = n;
      ++m_rsp;
   }

   StackWord nextDataWord() {
      StackWord out =
         StackWord{m_bytecode[m_pc]} | StackWord{m_bytecode[m_pc + 1]} << 8;
      m_pc += 2;
      return out;
   }

private:
   static constexpr std::size_t STACK_SIZE = 0x1000;
   static constexpr std::size_t RETURN_STACK_SIZE = 0x1000;

   int m_pc = 0;
   int m_sp = 0;
   int m_rsp = 0;

   std::span<unsigned char> m_bytecode;
   std::array<StackWord, STACK_SIZE> m_stack;
   std::array<StackWord, RETURN_STACK_SIZE> m_return_stack;
   std::span<Intrinsic> m_intrinsics;
};
} // namespace engine
