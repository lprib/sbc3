#pragma once

#include <string_view>

namespace vm {

class ISystemModule {
   /// @brief get registered name of this module
   virtual std::string_view name() = 0;

   /// @brief call a function from this module
   /// @param name name of function to invoke
   /// @return whether the function was found
   virtual bool invoke(std::string_view name) = 0;
};

} // namespace vm