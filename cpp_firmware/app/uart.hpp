#pragma once

#include "stm32f4xx_hal.h"
#include <span>

namespace uart {

using RxCallback = void (*)(uint8_t);

class Uart {
public:
   Uart(USART_TypeDef* periph) : m_periph{periph} {};
   void init(uint32_t baudrate);
   void tx_blocking(std::span<uint8_t const> data);
   void set_rx_callback(RxCallback callback) {
      m_rx_callback = callback;
   }
   void start_async_rx();

private:
   USART_TypeDef* m_periph;
   UART_HandleTypeDef m_handle;
   RxCallback m_rx_callback = nullptr;
};

extern Uart Uart1;
extern Uart Uart2;
extern Uart Uart3;
extern Uart Uart4;
extern Uart Uart5;
extern Uart Uart6;

} // namespace uart