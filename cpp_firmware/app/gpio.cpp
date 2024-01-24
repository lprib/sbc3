#include "gpio.hpp"

#include "stm32f4xx_hal.h"
#include <stdint.h>

namespace gpio {

struct HalPortPin {
   GPIO_TypeDef* port;
   uint16_t pin;
};

static constexpr HalPortPin to_hardware_repr(Pin pin) {
   switch(pin) {
   default:
   case Pin::DebugLed:
      return HalPortPin{GPIOB, GPIO_PIN_0};
   case Pin::LedSer:
      return HalPortPin{GPIOE, GPIO_PIN_2};
   case Pin::LedSrClk:
      return HalPortPin{GPIOE, GPIO_PIN_3};
   case Pin::LedSrClr:
      return HalPortPin{GPIOE, GPIO_PIN_4};
   case Pin::LedRclk:
      return HalPortPin{GPIOE, GPIO_PIN_5};
   case Pin::Gp0:
      return HalPortPin{GPIOD, GPIO_PIN_8};
   case Pin::Gp1:
      return HalPortPin{GPIOD, GPIO_PIN_9};
   case Pin::Gp2:
      return HalPortPin{GPIOD, GPIO_PIN_10};
   case Pin::Gp3:
      return HalPortPin{GPIOD, GPIO_PIN_11};
   case Pin::Gp4:
      return HalPortPin{GPIOD, GPIO_PIN_12};
   case Pin::Gp5:
      return HalPortPin{GPIOD, GPIO_PIN_13};
   case Pin::Gp6:
      return HalPortPin{GPIOD, GPIO_PIN_14};
   case Pin::Gp7:
      return HalPortPin{GPIOD, GPIO_PIN_15};
      // NOTE: when adding to this list, enable RCC clock in configure_clocks()
   }
}

static void init_pin(
   Pin pin, uint32_t mode, uint32_t pull, uint32_t speed, uint32_t alternate
) {
   auto hw = to_hardware_repr(pin);
   GPIO_InitTypeDef init = {
      .Pin = hw.pin,
      .Mode = mode,
      .Pull = pull,
      .Speed = speed,
      .Alternate = alternate,
   };

   HAL_GPIO_Init(hw.port, &init);
}

void configure_clocks() {
   __HAL_RCC_GPIOB_CLK_ENABLE();
   __HAL_RCC_GPIOD_CLK_ENABLE();
   __HAL_RCC_GPIOE_CLK_ENABLE();
}

void to_lowspeed_pp_out(Pin pin) {
   init_pin(pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, 0);
}

void to_lowspeed_in(Pin pin) {
   init_pin(pin, GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, 0);
}

void write(Pin pin, bool value) {
   auto hw = to_hardware_repr(pin);
   HAL_GPIO_WritePin(hw.port, hw.pin, value ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

bool read(Pin pin) {
   auto hw = to_hardware_repr(pin);
   switch(HAL_GPIO_ReadPin(hw.port, hw.pin)) {
   default:
   case GPIO_PIN_RESET:
      return false;
   case GPIO_PIN_SET:
      return true;
   }
}

void toggle(Pin pin) {
   auto hw = to_hardware_repr(pin);
   HAL_GPIO_TogglePin(hw.port, hw.pin);
}

} // namespace gpio
