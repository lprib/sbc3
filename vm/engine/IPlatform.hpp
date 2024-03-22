#pragma once

#include <optional>

#include "BytecodeModule.hpp"

namespace vm {

class IPlatform {
public:
   virtual std::optional<BytecodeModule> get_module(std::string_view name) = 0;

private:
};

} // namespace vm