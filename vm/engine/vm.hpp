#pragma once

#include <expected>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

namespace vm {

enum class Error {
   InvalidHeader,
};

using StackWord = short;

class Stack {
public:
   Stack(int size) : m_stack(size, 0), m_sp(0) {}

   StackWord peek() const {
      return m_stack[m_sp - 1];
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

class Module {
public:
   struct ExportFunction {
      std::string_view name;
      int bytecode_offset;
   };

   static std::expected<Module, Error> load(std::span<unsigned char> bytecode
   );

   std::string_view name() const {
      return m_module_name;
   }

   std::optional<ExportFunction> nth_export(int n) const {
      if(n < m_exports.size()) {
         return m_exports[n];
      } else {
         return std::nullopt;
      }
   }

private:
   static constexpr int STACK_SIZE = 0x100;
   static constexpr int RETURN_STACK_SIZE = 0x80;

   /// @brief local copy of bytecode
   ///
   /// Be careful not to re-allocate this, since the string_views are references
   /// in to this
   std::vector<unsigned char> m_bytecode;
   Stack m_stack;
   Stack m_return_stack;

   /// @brief module name, view into m_bytecode
   std::string_view m_module_name;

   /// @brief exports names, view into m_bytecode
   std::vector<ExportFunction> m_exports;

   Module(
      std::vector<unsigned char> bytecode, std::string_view module_name,
      std::vector<ExportFunction> exports
   );
};

class Machine {};
} // namespace vm