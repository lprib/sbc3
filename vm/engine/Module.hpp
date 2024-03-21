#pragma once

#include <expected>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

#include "engine_common.hpp"

namespace vm {

class Module {
public:
   struct ExportFunction {
      std::string_view name;
      int bytecode_offset;
   };

   static std::expected<Module, Error> load(std::span<unsigned char> bytecode);

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

   unsigned char code_byte(int index) const {
      return m_bytecode[m_code_start_index + index];
   }

private:
   /// @brief local copy of bytecode
   ///
   /// NOTE: Be careful not to re-allocate this, since the string_views are
   /// references pointing in to this
   std::vector<unsigned char> m_bytecode;
   int m_code_start_index;

   /// @brief module name, view into m_bytecode
   std::string_view m_module_name;

   /// @brief exports names, view into m_bytecode
   std::vector<ExportFunction> m_exports;

   Module(
      std::vector<unsigned char> bytecode, std::string_view module_name,
      std::vector<ExportFunction> exports, int code_start_index
   );
};

} // namespace vm