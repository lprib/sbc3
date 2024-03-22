#pragma once

#include <vector>

namespace vm {

template <typename T> class Stack {
public:
   Stack(int size) : m_stack(size, 0), m_sp(0) {}

   T peek() const {
      return m_stack[m_sp - 1];
   }

   T peek_n(int n) const {
      return m_stack[m_sp - 1 - n];
   }

   T pop() {
      --m_sp;
      return m_stack[m_sp];
   }

   void push(T n) {
      m_stack[m_sp] = n;
      ++m_sp;
   }

private:
   std::vector<T> m_stack;
   int m_sp = 0;
};

} // namespace vm