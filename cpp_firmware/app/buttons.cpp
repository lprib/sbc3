#include "buttons.hpp"

#include "FreeRTOS.h"
#include "task.h"
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"

#include "gpio.hpp"
#include "serial.hpp"

#include <cstdio>

namespace buttons {
static constexpr int CYCLE_TIME_MS = 5;
static constexpr int DEBOUNCE_WAIT_CYCLES = 4;

static int counters[N_BUTTONS] = {0, 0, 0};
static bool states[N_BUTTONS] = {false, false, false};

static void update_gpio(gpio::pin_t const& pin, Button button) {
   auto& counter = counters[static_cast<int>(button)];
   if(counter > 0) {
      --counter;
   } else {
      auto new_state = pin.read();
      auto& current_state = states[static_cast<int>(button)];
      if(new_state != current_state) {
         current_state = new_state;
         counter = DEBOUNCE_WAIT_CYCLES;
      }
   }
}

static void task(void* args) {
   (void)args;

   for(;;) {
      vTaskDelay(pdMS_TO_TICKS(CYCLE_TIME_MS));
      update_gpio(gpio::gp0, Button::Left);
      update_gpio(gpio::gp1, Button::Right);
      update_gpio(gpio::gp2, Button::Rotary);
      // char str[128];
      // std::snprintf(str, 182, "%d %d %d\n", states[0], states[1], states[2]);
      // serial::print(str);
   }
}

void init() {
   gpio::gp0.to_lowspeed_in();
   gpio::gp1.to_lowspeed_in();
   gpio::gp2.to_lowspeed_in();

   xTaskCreate(
      &task,
      "mmi",
      configMINIMAL_STACK_SIZE * 2,
      NULL,
      tskIDLE_PRIORITY + 2,
      NULL
   );
}

bool get_state(Button button) {
   return states[static_cast<int>(button)];
}

} // namespace buttons