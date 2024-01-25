#include "gpio.hpp"

#include "stm32f4xx_hal.h"

namespace gpio {
void configure_clocks() {
   __HAL_RCC_GPIOB_CLK_ENABLE();
   __HAL_RCC_GPIOD_CLK_ENABLE();
   __HAL_RCC_GPIOE_CLK_ENABLE();
}
} // namespace gpio
