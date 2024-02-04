#pragma once

#include <stdint.h>

#include "stm32f405xx.h"
#include "stm32f4xx_ll_gpio.h"

namespace gpio {

void init();

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

   void use_alternate_function(uint8_t alternate_function) const {
      configure(
         LL_GPIO_MODE_ALTERNATE,
         LL_GPIO_OUTPUT_PUSHPULL,
         LL_GPIO_PULL_NO,
         LL_GPIO_SPEED_FREQ_VERY_HIGH,
         static_cast<uint32_t>(alternate_function)
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

const Pin dbg_led{GPIOB, LL_GPIO_PIN_0};
const Pin led_ser{GPIOE, LL_GPIO_PIN_2};
const Pin led_srclk{GPIOE, LL_GPIO_PIN_3};
const Pin led_srclr{GPIOE, LL_GPIO_PIN_4};
const Pin led_rclk{GPIOE, LL_GPIO_PIN_5};
const Pin gp0{GPIOD, LL_GPIO_PIN_8};
const Pin gp1{GPIOD, LL_GPIO_PIN_9};
const Pin gp2{GPIOD, LL_GPIO_PIN_10};
const Pin gp3{GPIOD, LL_GPIO_PIN_11};
const Pin gp4{GPIOD, LL_GPIO_PIN_12};
const Pin gp5{GPIOD, LL_GPIO_PIN_13};
const Pin gp6{GPIOD, LL_GPIO_PIN_14};
const Pin gp7{GPIOD, LL_GPIO_PIN_15};

} // namespace gpio