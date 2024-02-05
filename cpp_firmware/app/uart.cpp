#include "uart.hpp"
#include "error_handler.hpp"

namespace uart {
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
      app::error_handler();
   }
}

void Uart::tx_blocking(std::span<uint8_t const> data) {
   // waits on TXE forever?
   HAL_UART_Transmit(&m_handle, data.data(), data.size(), HAL_MAX_DELAY);
}

extern "C" void Uart3RxComplete(UART_HandleTypeDef* huart) {
   
}

} // namespace uart