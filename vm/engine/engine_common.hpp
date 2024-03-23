#pragma once

#include <string_view>

namespace vm {

enum class Error {
   InvalidHeader,
   ModuleNotFound,
   EntryNotFound,
   EofWithoutReturn,
};

std::string_view error_to_str(Error error);

} // namespace vm