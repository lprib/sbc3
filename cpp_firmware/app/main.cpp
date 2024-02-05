#include <span>

#include "FreeRTOS.h"
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "task.h"

#include "error_handler.hpp"
#include "gpio.hpp"
#include "ledstrip.hpp"
#include "system_clock_config.hpp"
#include "uart.hpp"

static void task(void* thing) {
   uart::Uart3.init(115200);
   gpio::dbg_led.to_lowspeed_pp_out();

   char const* helo = "helo\n";
   uint8_t const* x = reinterpret_cast<uint8_t const*>(helo);

   for(;;) {
      // gpio::dbg_led.toggle();
      for(int i = 0; i < 16; ++i) {
         ledstrip::write(0x8000 >> i);
         vTaskDelay(pdMS_TO_TICKS(5));
      }
      for(int i = 15; i >= 0; --i) {
         ledstrip::write(0x8000 >> i);
         vTaskDelay(pdMS_TO_TICKS(5));
      }
      vTaskDelay(pdMS_TO_TICKS(100));

      uart::Uart3.tx_blocking(std::span(x, x + 5));
   }
}

int main(void) {
   HAL_Init();

   // TODO(liam) these are for uart3, put into uart.cpp?
   __HAL_RCC_USART3_CLK_ENABLE();
   __HAL_RCC_GPIOB_CLK_ENABLE();
   gpio::Pin uart3_tx{GPIOB, GPIO_PIN_10};
   gpio::Pin uart3_rx{GPIOB, GPIO_PIN_12};
   uart3_tx.use_alternate_function(7); // UART1..3
   uart3_rx.use_alternate_function(7); // UART1..3

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

/// @brief Need to call the STM32 HAL tick function otherwise timeouts will
/// never work
extern "C" void vApplicationTickHook() {
   HAL_IncTick();
}