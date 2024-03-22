#include "BytecodeModule.hpp"

#include <iostream>
#include <optional>

namespace vm {

BytecodeModule::BytecodeModule(
   std::vector<unsigned char> bytecode, std::string_view module_name,
   std::vector<ExportFunction> exports, int code_start_index
) :
   m_bytecode(std::move(bytecode)),
   m_code_start_index(code_start_index),
   m_module_name(module_name),
   m_exports(std::move(exports)) {}

std::expected<BytecodeModule, Error> BytecodeModule::load(
   std::span<unsigned char> bytecode
) {
   std::size_t cursor = 0;

   std::vector<unsigned char> bytecode_copy(bytecode.begin(), bytecode.end());

   if(cursor >= bytecode.size())
      return std::unexpected(Error::InvalidHeader);
   auto module_name_len = std::size_t{bytecode[cursor]};
   cursor += 1;

   if(module_name_len + cursor > bytecode.size())
      return std::unexpected(Error::InvalidHeader);

   auto module_name = std::string_view(
      reinterpret_cast<char*>(&bytecode_copy.data()[cursor]),
      module_name_len
   );

   cursor += module_name_len;

   if(cursor >= bytecode.size())
      return std::unexpected(Error::InvalidHeader);
   auto num_exports = std::size_t{bytecode[cursor]};
   cursor += 1;

   std::vector<ExportFunction> exports;
   exports.reserve(num_exports);

   for(int i = 0; i < num_exports; ++i) {
      if(cursor >= bytecode.size())
         return std::unexpected(Error::InvalidHeader);
      auto fn_name_len = std::size_t{bytecode[cursor]};
      cursor += 1;

      if(fn_name_len + cursor > bytecode.size())
         return std::unexpected(Error::InvalidHeader);

      auto fn_name = std::string_view(
         reinterpret_cast<char const*>(&bytecode_copy.data()[cursor]),
         fn_name_len
      );
      cursor += fn_name_len;

      if(cursor + 1 >= bytecode.size())
         return std::unexpected(Error::InvalidHeader);

      auto fn_offset = int{bytecode[cursor]} | (int{bytecode[cursor + 1]} << 8);
      cursor += 2;

      exports.push_back(ExportFunction(fn_name, fn_offset));
   }

   return BytecodeModule(
      std::move(bytecode_copy),
      module_name,
      std::move(exports),
      cursor
   );
}

} // namespace vm