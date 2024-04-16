#pragma once

#include <string_view>

namespace vm {

enum class Error {
   InvalidHeader,
   ModuleNotFound,
   EntryNotFound,
   EofWithoutReturn,
   ExternCallInvalidFunctionId,
};

char const* error_to_str(Error error);

} // namespace vm