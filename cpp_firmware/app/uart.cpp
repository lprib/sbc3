#include "uart.hpp"
#include "error_handler.hpp"

namespace uart {
Uart::Uart(Peripheral which) {
   switch(which) {
   case Peripheral::One:
      m_periph = USART1;
      break;
   case Peripheral::Two:
      m_periph = USART2;
      break;
   case Peripheral::Three:
      m_periph = USART3;
      break;
   case Peripheral::Four:
      m_periph = UART4;
      break;
   case Peripheral::Five:
      m_periph = UART5;
      break;
   case Peripheral::Six:
      m_periph = USART6;
      break;
   default:
      app::error_handler();
   }
}

void Uart::init(uint32_t baudrate) {
   m_handle.Instance = m_periph;
   m_handle.Init.BaudRate = baudrate;
   m_handle.Init.WordLength = UART_WORDLENGTH_8B;
   m_handle.Init.StopBits = UART_STOPBITS_1;
   m_handle.Init.Parity = UART_PARITY_NONE;
   m_handle.Init.Mode = UART_MODE_TX_RX;
   m_handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
   m_handle.Init.OverSampling = UART_OVERSAMPLING_16;
   if(HAL_UART_Init(&m_handle) != HAL_OK) {
      app::error_handler();
   }
}

void Uart::tx_blocking(std::span<uint8_t const> data) {
   // waits on TXE forever?
   HAL_UART_Transmit(&m_handle, data.data(), data.size(), 100);
}
} // namespace uart