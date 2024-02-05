#pragma once

#include "stm32f4xx_hal.h"
#include <span>

namespace uart {

class Uart {
public:
   Uart(USART_TypeDef* periph) : m_periph{periph} {};
   void init(uint32_t baudrate);
   void tx_blocking(std::span<uint8_t const> data);

private:
   USART_TypeDef* m_periph;
   UART_HandleTypeDef m_handle;
};

Uart Uart1{USART1};
Uart Uart2{USART2};
Uart Uart3{USART3};
Uart Uart4{UART4};
Uart Uart5{UART5};
Uart Uart6{USART6};

} // namespace uart