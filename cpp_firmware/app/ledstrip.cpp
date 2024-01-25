#include "ledstrip.hpp"

#include "gpio.hpp"
#include "stm32f4xx_hal.h"

#include "FreeRTOS.h"
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
      // todo(liam)
      vTaskDelay(1);
      gpio::led_srclk.write(false);
      vTaskDelay(1);
   }
   gpio::led_rclk.write(true);
   vTaskDelay(1);
   gpio::led_rclk.write(false);
   vTaskDelay(1);
}

} // namespace ledstrip