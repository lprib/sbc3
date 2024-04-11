#include "engine_common.hpp"

namespace vm {

std::string_view error_to_str(Error error) {
   switch(error) {
   case Error::InvalidHeader:
      return "Invalid header";
   case Error::ModuleNotFound:
      return "Module not found";
   case Error::EntryNotFound:
      return "`entry` export not found";
   case Error::EofWithoutReturn:
      return "reached end of module without return opcode";
   default:
      return "<Unknown error>";
   }
}

} // namespace vm