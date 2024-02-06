#pragma once

namespace error {
void error();

static inline void check(bool condition) {
   if(!condition) {
      error();
   }
}
} // namespace app
