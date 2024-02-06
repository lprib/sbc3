#include "uart.hpp"
#include "error.hpp"

namespace uart {

Uart Uart1{USART1};
Uart Uart2{USART2};
Uart Uart3{USART3};
Uart Uart4{UART4};
Uart Uart5{UART5};
Uart Uart6{USART6};

extern "C" void Uart3RxComplete(UART_HandleTypeDef* huart);

void Uart::init(uint32_t baudrate) {
   m_handle.Instance = m_periph;
   m_handle.Init.BaudRate = baudrate;
   m_handle.Init.WordLength = UART_WORDLENGTH_8B;
   m_handle.Init.StopBits = UART_STOPBITS_1;
   m_handle.Init.Parity = UART_PARITY_NONE;
   m_handle.Init.Mode = UART_MODE_TX_RX;
   m_handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
   m_handle.Init.OverSampling = UART_OVERSAMPLING_16;
   m_handle.RxCpltCallback = Uart3RxComplete;
   if(HAL_UART_Init(&m_handle) != HAL_OK) {
      error::error();
   }
}

void Uart::tx_blocking(std::span<uint8_t const> data) {
   // waits on TXE forever?
   HAL_UART_Transmit(&m_handle, data.data(), data.size(), HAL_MAX_DELAY);
}

void Uart::start_async_rx()
{

}

extern "C" void Uart3RxComplete(UART_HandleTypeDef* huart) {
   (void)huart;
}

} // namespace uart