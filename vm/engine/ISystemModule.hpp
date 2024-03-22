#pragma once

#include <string_view>

namespace vm {

class ISystemModule {
public:
   ISystemModule(std::string_view name) : m_module_name(name) {}

   std::string_view name() const {
      return m_module_name;
   }

private:
   std::string_view m_module_name;
};
} // namespace vm