#include "FreeRTOS.h"
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "task.h"

#include "error_handler.hpp"
#include "gpio.hpp"
#include "ledstrip.hpp"
#include "system_clock_config.hpp"

static void task(void* thing) {
   gpio::dbg_led.to_lowspeed_pp_out();
   for(;;) {
      gpio::dbg_led.toggle();
      for(int i = 0; i < 16; ++i) {
         ledstrip::write(0x8000 >> i);
         vTaskDelay(pdMS_TO_TICKS(10));
      }
      // vTaskDelay(pdMS_TO_TICKS(100));
   }
}

int main(void) {
   HAL_Init();

   app::system_clock_config();
   gpio::init();
   ledstrip::init();

   /* Ensure all priority bits are assigned as preemption priority bits. */
   // TODO(liam) what do, not in hal??
   // NVIC_PriorityGroupConfig(NVIC_PRIORITYGROUP_4);

   xTaskCreate(&task, "BLINK", 256, NULL, tskIDLE_PRIORITY + 1U, NULL);

   vTaskStartScheduler();

   while(1) {
   }
}

void app::error_handler(void) {
   __disable_irq();
   while(1) {
   }
}

/** Handler for pure virtual function calls.  This should be included, otherwise
 * the exception handling is pulled in. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-declarations"
extern "C" void __cxa_pure_virtual() {
   app::error_handler();
}
#pragma GCC diagnostic pop