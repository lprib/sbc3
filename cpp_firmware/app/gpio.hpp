#pragma once

#include <stdint.h>

#include "stm32f405xx.h"
#include "stm32f4xx_ll_gpio.h"

namespace gpio {


void configure_clocks();

class Pin {
public:
   constexpr Pin(GPIO_TypeDef* _port, uint16_t _pin) : port(_port), pin(_pin) {}
   void to_lowspeed_pp_out() const {
      configure(
         LL_GPIO_MODE_OUTPUT,
         LL_GPIO_OUTPUT_PUSHPULL,
         LL_GPIO_PULL_NO,
         LL_GPIO_SPEED_FREQ_LOW,
         LL_GPIO_AF_0
      );
   }

   void to_lowspeed_in() const {
      configure(
         LL_GPIO_MODE_INPUT,
         LL_GPIO_OUTPUT_PUSHPULL,
         LL_GPIO_PULL_NO,
         LL_GPIO_SPEED_FREQ_LOW,
         LL_GPIO_AF_0
      );
   }

   void write(bool value) const {
      if(value) {
         LL_GPIO_SetOutputPin(port, pin);
      } else {
         LL_GPIO_ResetOutputPin(port, pin);
      }
   }

   void toggle() const {
      LL_GPIO_TogglePin(port, pin);
   }

   bool read() const {
      return LL_GPIO_IsInputPinSet(port, pin);
   }

private:
   GPIO_TypeDef* const port;
   uint16_t const pin;

   void configure(
      uint32_t mode, uint32_t otype, uint32_t pull, uint32_t speed, uint32_t alt
   ) const {
      LL_GPIO_SetPinMode(port, pin, mode);
      LL_GPIO_SetPinOutputType(port, pin, otype);
      LL_GPIO_SetPinSpeed(port, pin, speed);
      LL_GPIO_SetPinPull(port, pin, pull);
      if(pin & 0xff) {
         LL_GPIO_SetAFPin_0_7(port, pin, alt);
      } else {
         LL_GPIO_SetAFPin_8_15(port, pin, alt);
      }
   }
};

const Pin DebugLed{GPIOB, LL_GPIO_PIN_0};
const Pin LedSer{GPIOE, LL_GPIO_PIN_2};
const Pin LedSrClk{GPIOE, LL_GPIO_PIN_3};
const Pin LedSrClr{GPIOE, LL_GPIO_PIN_4};
const Pin LedRclk{GPIOE, LL_GPIO_PIN_5};
const Pin Gp0{GPIOD, LL_GPIO_PIN_8};
const Pin Gp1{GPIOD, LL_GPIO_PIN_9};
const Pin Gp2{GPIOD, LL_GPIO_PIN_10};
const Pin Gp3{GPIOD, LL_GPIO_PIN_11};
const Pin Gp4{GPIOD, LL_GPIO_PIN_12};
const Pin Gp5{GPIOD, LL_GPIO_PIN_13};
const Pin Gp6{GPIOD, LL_GPIO_PIN_14};
const Pin Gp7{GPIOD, LL_GPIO_PIN_15};

} // namespace gpio