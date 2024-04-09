#include <span>

#include "FreeRTOS.h"
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "task.h"
#include "timers.h"

#include "buttons.hpp"
#include "error.hpp"
#include "gpio.hpp"
#include "ledstrip.hpp"
#include "system_clock_config.hpp"
// #include "uart.hpp"
#include "comms.hpp"
#include "screen.hpp"
#include "serial.hpp"
#include "sync.hpp"
#include "util.hpp"

enum class Qt { Thing, Thang };

static sync::queue<Qt> q(100);

static void task(void* arg) {
   (void)arg;
   // uart::Uart3.init(115200);

   for(;;) {
      auto rump = q.blocking_receive();
      /*
      for(int i = 0; i < 16; ++i) {
         ledstrip::write(0x8000 >> i);
         vTaskDelay(pdMS_TO_TICKS(5));
      }
      for(int i = 15; i >= 0; --i) {
         ledstrip::write(0x8000 >> i);
         vTaskDelay(pdMS_TO_TICKS(5));
      }
      */
      vTaskDelay(pdMS_TO_TICKS(100));

      switch(rump) {
      case Qt::Thang:
         // serial::print("Qt::Thang\n");
         break;
      case Qt::Thing:
         // serial::print("Qt::Thing\n");
         break;
      default:
         // serial::print("borked\n");
         break;
      }
      // uart::Uart3.tx_blocking(std::span(x, x + 5));
   }
}

int main(void) {
   HAL_Init();

   // TODO(liam) these are for uart3, put into uart.cpp?
   __HAL_RCC_USART3_CLK_ENABLE();
   __HAL_RCC_GPIOB_CLK_ENABLE();
   gpio::pin_t uart3_tx{GPIOB, GPIO_PIN_10};
   gpio::pin_t uart3_rx{GPIOB, GPIO_PIN_11};
   uart3_tx.use_alternate_function(7); // USART[1..3]
   uart3_rx.use_alternate_function(7); // USART[1..3]

   app::system_clock_config();
   gpio::init();
   ledstrip::init();
   serial::init();
   buttons::init();
   screen::init();

   screen::get_dislay_buffer()[10 * screen::DISPLAY_WIDTH + 10] = 0xf;
   screen::update();

   gpio::dbg_led.to_lowspeed_pp_out();

   // HAL_Init() contains a call to:
   // HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

   xTaskCreate(
      &task,
      "BLINK",
      configMINIMAL_STACK_SIZE * 8,
      NULL,
      tskIDLE_PRIORITY + 1U,
      NULL
   );

   vTaskStartScheduler();

   while(1) {
   }
}

[[noreturn]] void error::error(void) {
   __disable_irq();
   while(1) {
      gpio::dbg_led.toggle();
      util::spinloop_us(1000 * 40);
   }
}

extern "C" {
/** Handler for pure virtual function calls.  This should be included, otherwise
 * the exception handling is pulled in. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-declarations"
extern "C" void __cxa_pure_virtual() {
   error::error();
}

// some nosys bullshit
void _close_r(void) {}
void _lseek_r(void) {}
void _read_r(void) {}
void _write_r(void) {}
#pragma GCC diagnostic pop

/// @brief Need to call the STM32 HAL tick function otherwise timeouts will
/// never work
extern "C" void vApplicationTickHook() {
   HAL_IncTick();
}

/// @brief FreeRTOS static allocation hook
void vApplicationGetTimerTaskMemory(
   StaticTask_t** ppxTimerTaskTCBBuffer, StackType_t** ppxTimerTaskStackBuffer,
   uint32_t* pulTimerTaskStackSize
) {
   static StaticTask_t xTimerTaskTCB;
   static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];
   *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
   *ppxTimerTaskStackBuffer = uxTimerTaskStack;
   *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

/// @brief FreeRTOS static allocation hook
void vApplicationGetIdleTaskMemory(
   StaticTask_t** ppxIdleTaskTCBBuffer, StackType_t** ppxIdleTaskStackBuffer,
   uint32_t* pulIdleTaskStackSize
) {
   static StaticTask_t xIdleTaskTCB;
   static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];
   *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
   *ppxIdleTaskStackBuffer = uxIdleTaskStack;
   *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
}