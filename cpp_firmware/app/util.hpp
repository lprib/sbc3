#pragma once

namespace util {
static inline void spinloop_us(int us) {
   /// 168 clocks per us
   /// choose 21 clocks per loop
   /// 168/21 = 8
   /// must loop `us * 8` times;

   int loops = us * 8;
   // clang-format off
   asm volatile(
    "spinloop_us_loop_start:\n\t"
    "    nop\n\t" // 19 nops
    "    nop\n\t"
    "    nop\n\t"
    "    nop\n\t"
    "    nop\n\t"
    "    nop\n\t"
    "    nop\n\t"
    "    nop\n\t"
    "    nop\n\t"
    "    nop\n\t"
    "    nop\n\t"
    "    nop\n\t"
    "    nop\n\t"
    "    nop\n\t"
    "    nop\n\t"
    "    nop\n\t"
    "    nop\n\t"
    "    nop\n\t"
    "    nop\n\t"
    "    subs %[count], %[count], #1\n\t" // 1 clock
    "    bne spinloop_us_loop_start\n\t" // nominal 1 clock
    : [count] "+r"(loops));
    //clang-format on
}
}