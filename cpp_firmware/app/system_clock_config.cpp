#include "system_clock_config.hpp"

#include "error.hpp"
#include "stm32f4xx_hal.h"

namespace app {

void system_clock_config() {
   __HAL_RCC_SYSCFG_CLK_ENABLE();

   /** Configure the main internal regulator output voltage
    */
   __HAL_RCC_PWR_CLK_ENABLE();
   __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

   /** Initializes the RCC Oscillators according to the specified parameters
    * in the RCC_OscInitTypeDef structure.
    */
   RCC_OscInitTypeDef osc_init_struct =
      {.OscillatorType = RCC_OSCILLATORTYPE_HSE,
       .HSEState = RCC_HSE_ON,
       .LSEState = RCC_LSE_OFF,
       .HSIState = RCC_HSI_OFF,
       .HSICalibrationValue = 0,
       .LSIState = RCC_LSI_OFF,
       .PLL = {
          .PLLState = RCC_PLL_ON,
          .PLLSource = RCC_PLLSOURCE_HSE,
          .PLLM = 8,
          .PLLN = 168,
          .PLLP = RCC_PLLP_DIV2,
          .PLLQ = 7,
       }};
   if(HAL_RCC_OscConfig(&osc_init_struct) != HAL_OK) {
      error::error();
   }

   RCC_ClkInitTypeDef clk_init_struct = {
      .ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                   RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2,
      .SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK,
      .AHBCLKDivider = RCC_SYSCLK_DIV1,
      .APB1CLKDivider = RCC_HCLK_DIV4,
      .APB2CLKDivider = RCC_HCLK_DIV2,
   };

   if(HAL_RCC_ClockConfig(&clk_init_struct, FLASH_LATENCY_5) != HAL_OK) {
      error::error();
   }
} // namespace app

} // namespace app