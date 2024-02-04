#pragma once

#include "stm32f4xx.h"

namespace util {

/// -Os output of for loop:
/// ```
/// .L2:
///         nop                  // 1 instr
///         subs    r3, r3, #1   // 1 instr
///         bne     .L2          // 1 instr (1+Pc in ref manual, assume branch
///                              // is predicted and Pc = 0)
/// ```
#pragma GCC push_options
#pragma GCC optimize("-Os")
static inline void spinloop_us(int us) {
   static constexpr int US_PER_S = 1000000;
   static constexpr int CLK_PER_LOOP = 3;

   int const cycles = us * ((int)SystemCoreClock / US_PER_S / CLK_PER_LOOP);

   for(int i = 0; i < cycles; i++) {
      __asm volatile("nop");
   }
}
#pragma GCC pop_options

} // namespace util