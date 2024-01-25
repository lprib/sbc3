#include "FreeRTOS.h"
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "task.h"

#include "error_handler.hpp"
#include "system_clock_config.hpp"
#include "gpio.hpp"

#define DBG_LED_Pin GPIO_PIN_0
#define DBG_LED_GPIO_Port GPIOB
#define BOOT1_Pin GPIO_PIN_2
#define BOOT1_GPIO_Port GPIOB
#define CODEC_GPIO1_Pin GPIO_PIN_14
#define CODEC_GPIO1_GPIO_Port GPIOB
#define CODEC_NRESET_Pin GPIO_PIN_9
#define CODEC_NRESET_GPIO_Port GPIOA

static void write_leds(uint16_t val) {
   HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_SET);
   HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET);
   HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_RESET);

   for(int i = 0; i < 16; i++) {
      HAL_GPIO_WritePin(
         GPIOE,
         GPIO_PIN_2,
         val & (0x8000 >> i) ? GPIO_PIN_SET : GPIO_PIN_RESET
      );
      // HAL_Delay(1);
      HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_SET);
      // HAL_Delay(1);
      HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET);
   }
   // HAL_Delay(1);
   HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_SET);
   // HAL_Delay(1);
   HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_RESET);
}

static void task(void* thing) {
   gpio::DebugLed.to_lowspeed_pp_out();
   for(;;) {
      gpio::DebugLed.toggle();
      vTaskDelay(pdMS_TO_TICKS(1000));
   }
}

int main(void) {
   HAL_Init();

   app::system_clock_config();
   gpio::configure_clocks();

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