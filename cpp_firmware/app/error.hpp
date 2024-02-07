#pragma once

namespace error {
[[noreturn]] void error();

static inline void check(bool condition) {
   if(!condition) {
      error();
   }
}
} // namespace error
