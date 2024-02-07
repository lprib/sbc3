#include "serial.hpp"

#include "stm32f405xx.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_usart.h"

#include "error.hpp"
#include "sync.hpp"

#include "FreeRTOS.h"
#include "semphr.h"

// test
#include "gpio.hpp"

namespace serial {

static sync::mutex serial_mutex;

void init() {
   LL_USART_Disable(USART3);
   LL_USART_InitTypeDef init{
      .BaudRate = 0,
      .DataWidth = LL_USART_DATAWIDTH_8B,
      .StopBits = LL_USART_STOPBITS_1,
      .Parity = LL_USART_PARITY_NONE,
      .TransferDirection = LL_USART_DIRECTION_TX_RX,
      .HardwareFlowControl = LL_USART_HWCONTROL_NONE,
      .OverSampling = LL_USART_OVERSAMPLING_16,
   };
   LL_USART_Init(USART3, &init);

   // We have to do this manually because the version in LL_USART_Init is broken
   // when calculating clock rates
   unsigned pclk1 =
      __LL_RCC_CALC_PCLK1_FREQ(SystemCoreClock, LL_RCC_GetAPB1Prescaler());
   LL_USART_SetBaudRate(USART3, pclk1, LL_USART_OVERSAMPLING_16, 115200);

   LL_USART_Enable(USART3);

   NVIC_SetPriority(USART3_IRQn, 6);
   NVIC_EnableIRQ(USART3_IRQn);

   LL_USART_EnableIT_RXNE(USART3);
}

void tx(std::span<unsigned char const> ns) {
   sync::lock l{serial_mutex};
   for(auto n : ns) {
      while(!LL_USART_IsActiveFlag_TXE(USART3)) {
      }
      LL_USART_TransmitData8(USART3, n);
   }
   while(!LL_USART_IsActiveFlag_TC(USART3)) {
   }
}

void tx(unsigned char n) {
   tx(std::span(&n, 1));
}

void tx(std::string_view str) {
   tx(std::span(
      reinterpret_cast<unsigned char const*>(str.begin()),
      reinterpret_cast<unsigned char const*>(str.end())
   ));
}

extern "C" void USART3_IRQHandler(void) {
   gpio::dbg_led.write(true);
   volatile uint8_t data = LL_USART_ReceiveData8(USART3);

   gpio::dbg_led.write(data & 1);
}

} // namespace serial