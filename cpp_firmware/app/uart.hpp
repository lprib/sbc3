#pragma once

#include "stm32f4xx_hal.h"
#include <span>

namespace uart {

enum class Peripheral { One, Two, Three, Four, Five, Six };

class Uart {
public:
   Uart(Peripheral which);
   void init(uint32_t baudrate);
   void tx_blocking(std::span<uint8_t const> data);
private:
   USART_TypeDef* m_periph;
   UART_HandleTypeDef m_handle;
};

} // namespace uart