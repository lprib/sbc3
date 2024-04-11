#pragma once

#include "stm32f4xx.h"

namespace util {

#pragma GCC push_options
#pragma GCC optimize("-Os")
/// -Os output of for loop:
/// ```
/// .L2:
///         nop                  // 1 instr
///         subs    r3, r3, #1   // 1 instr
///         bne     .L2          // 1 instr (1+Pc in ref manual, assume branch
///         is predicted and Pc = 0)
/// ```
static inline void spinloop_cycles(int cycles) {
   int const cycles_div_3 = cycles / 3;
   for(int i = 0; i < cycles_div_3; i++) {
      __asm volatile("nop");
   }
}

static inline void spinloop_us(int us) {
   static constexpr int US_PER_S = 1000000;
   int const cycles = us * ((int)SystemCoreClock / US_PER_S);
   spinloop_cycles(cycles);
}

#pragma GCC pop_options

} // namespace util