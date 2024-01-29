#include "ledstrip.hpp"

#include "gpio.hpp"
#include "util.hpp"

#include "FreeRTOS.h"
#include "stm32f4xx_hal.h"
#include "task.h"

namespace ledstrip {

void init() {
   gpio::led_ser.to_lowspeed_pp_out();
   gpio::led_srclk.to_lowspeed_pp_out();
   gpio::led_srclr.to_lowspeed_pp_out();
   gpio::led_rclk.to_lowspeed_pp_out();
   gpio::led_srclr.write(true);
}

void write(uint16_t mask) {
   for(int i = 0; i < 16; ++i) {
      gpio::led_ser.write((mask & (0x8000 >> i)) != 0);
      gpio::led_srclk.write(true);
      util::spinloop_us(1);
      gpio::led_srclk.write(false);
   }
   util::spinloop_us(1);
   gpio::led_rclk.write(true);
   util::spinloop_us(1);
   gpio::led_rclk.write(false);
}

} // namespace ledstrip