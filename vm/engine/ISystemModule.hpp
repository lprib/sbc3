#pragma once

#include <string_view>

namespace vm {

class Machine;

class ISystemModule {
public:
   ISystemModule(std::string_view name) : m_module_name(name) {}

   std::string_view name() const {
      return m_module_name;
   }

   virtual void invoke_index(Machine& machine, int fn_id) = 0;

private:
   std::string_view m_module_name;
};
} // namespace vm