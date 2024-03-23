#pragma once

#include <expected>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

#define DEBUG_DUMP

#ifdef DEBUG_DUMP
#include <iostream>
#endif

#include "engine_common.hpp"

namespace vm {

class BytecodeModule {
public:
   struct ExportFunction {
      std::string_view name;
      int bytecode_offset;
   };

   static std::expected<BytecodeModule, Error> load(
      std::span<unsigned char> bytecode
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

   std::optional<ExportFunction> get_export(std::string_view name) const {
      for(auto const& exp : m_exports) {
         if(exp.name == name) {
            return exp;
         }
      }
      return std::nullopt;
   }

   std::span<unsigned char> code() {
      return m_bytecode_after_header;
   }

   int code_start_index() const {
      return m_code_start_index;
   }

#ifdef DEBUG_DUMP
   void dump_header() const {
      std::cout << "name: " << m_module_name << "\n";
      for(auto e : m_exports) {
         std::cout << "export: " << e.name << " " << e.bytecode_offset << "\n";
      }
   }
#endif

private:
   /// @brief local copy of bytecode
   ///
   /// NOTE: Be careful not to re-allocate this, since the string_views are
   /// references pointing in to this
   std::vector<unsigned char> m_bytecode;

   /// @brief index of code (skipping header)
   int m_code_start_index;

   std::span<unsigned char> m_bytecode_after_header;

   /// @brief module name, view into m_bytecode
   std::string_view m_module_name;

   /// @brief exports names, view into m_bytecode
   std::vector<ExportFunction> m_exports;

   BytecodeModule(
      std::vector<unsigned char> bytecode, std::string_view module_name,
      std::vector<ExportFunction> exports, int code_start_index
   );
};

} // namespace vm