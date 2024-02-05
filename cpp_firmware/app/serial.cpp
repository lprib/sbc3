#include "serial.hpp"

#include "stm32f405xx.h"

#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_usart.h"

namespace serial {
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
}

void tx(unsigned char n) {
   while(!LL_USART_IsActiveFlag_TXE(USART3))
      ;
   LL_USART_TransmitData8(USART3, n);
   while(!LL_USART_IsActiveFlag_TC(USART3))
      ;
}
} // namespace serial