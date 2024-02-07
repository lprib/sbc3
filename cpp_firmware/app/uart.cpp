#include "uart.hpp"

#include "error.hpp"
#include "irq_declarations.hpp"
#include "stm32f405xx.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_usart.h"

namespace uart {

struct callbacks_t {
   isr_callback_t rx_ready = nullptr;
   isr_callback_t tx_ready = nullptr;
   isr_callback_t tx_complete = nullptr;
};

callbacks_t callbacks[static_cast<int>(uart_t::NumUarts)];

static constexpr USART_TypeDef* to_periph(uart_t u) {
   switch(u) {
   case uart_t::One:
      return USART1;
   case uart_t::Two:
      return USART2;
   case uart_t::Three:
      return USART3;
   case uart_t::Four:
      return UART4;
   case uart_t::Five:
      return UART5;
   case uart_t::Six:
      return USART6;
   default:
      error::error();
   }
}

static constexpr IRQn_Type get_irqn(uart_t u) {
   switch(u) {
   case uart_t::One:
      return USART1_IRQn;
   case uart_t::Two:
      return USART2_IRQn;
   case uart_t::Three:
      return USART3_IRQn;
   case uart_t::Four:
      return UART4_IRQn;
   case uart_t::Five:
      return UART5_IRQn;
   case uart_t::Six:
      return USART6_IRQn;
   default:
      error::error();
   }
}

static unsigned get_pclk(USART_TypeDef* periph) {
   if((periph == USART1) || (periph == USART6)) {
      return __LL_RCC_CALC_PCLK2_FREQ(
         SystemCoreClock,
         LL_RCC_GetAPB2Prescaler()
      );
   } else {
      return __LL_RCC_CALC_PCLK1_FREQ(
         SystemCoreClock,
         LL_RCC_GetAPB1Prescaler()
      );
   }
}

void init(uart_t u, unsigned baud_rate) {
   auto periph = to_periph(u);
   LL_USART_Disable(periph);
   LL_USART_InitTypeDef init{
      .BaudRate = 0,
      .DataWidth = LL_USART_DATAWIDTH_8B,
      .StopBits = LL_USART_STOPBITS_1,
      .Parity = LL_USART_PARITY_NONE,
      .TransferDirection = LL_USART_DIRECTION_TX_RX,
      .HardwareFlowControl = LL_USART_HWCONTROL_NONE,
      .OverSampling = LL_USART_OVERSAMPLING_16,
   };
   LL_USART_Init(periph, &init);

   // We have to do this manually because the version in LL_USART_Init is broken
   // when calculating clock rates
   LL_USART_SetBaudRate(
      periph,
      get_pclk(periph),
      LL_USART_OVERSAMPLING_16,
      baud_rate
   );

   LL_USART_Enable(periph);

   auto irqn = get_irqn(u);
   NVIC_SetPriority(irqn, 6);
   NVIC_EnableIRQ(irqn);
}

void tx_block(uart_t u, unsigned char n) {
   auto periph = to_periph(u);
   while(!LL_USART_IsActiveFlag_TXE(periph)) {
   }
   LL_USART_TransmitData8(periph, n);
}

void wait_for_tx_complete(uart_t u) {
   auto periph = to_periph(u);
   while(!LL_USART_IsActiveFlag_TC(periph)) {
   }
}

void write_byte(uart_t u, unsigned char n) {
   LL_USART_TransmitData8(to_periph(u), n);
}
unsigned char read_byte(uart_t u) {
   return LL_USART_ReceiveData8(to_periph(u));
}

void set_isr_tx_ready(uart_t u, isr_callback_t cb) {
   callbacks[static_cast<int>(u)].tx_ready = cb;
   LL_USART_EnableIT_TXE(to_periph(u));
}
void set_isr_tx_complete(uart_t u, isr_callback_t cb) {
   callbacks[static_cast<int>(u)].tx_complete = cb;
   LL_USART_EnableIT_TC(to_periph(u));
}
void set_isr_rx_ready(uart_t u, isr_callback_t cb) {
   callbacks[static_cast<int>(u)].rx_ready = cb;
   LL_USART_EnableIT_RXNE(to_periph(u));
}

static void irq_handler(uart_t u) {
   auto periph = to_periph(u);
   // todo check errors

   auto& cbs = callbacks[static_cast<int>(u)];

   if(LL_USART_IsActiveFlag_TXE(periph)) {
      if(cbs.tx_ready != nullptr) {
         cbs.tx_ready();
      }
   }
   if(LL_USART_IsActiveFlag_TC(periph)) {
      if(cbs.tx_complete != nullptr) {
         cbs.tx_complete();
      }
   }
   if(LL_USART_IsActiveFlag_RXNE(periph)) {
      if(cbs.rx_ready != nullptr) {
         cbs.rx_ready();
      }
   }
}

} // namespace uart

extern "C" {
void USART1_IRQHandler(void) {
   uart::irq_handler(uart::uart_t::One);
}
void USART2_IRQHandler(void) {
   uart::irq_handler(uart::uart_t::Two);
}
void USART3_IRQHandler(void) {
   uart::irq_handler(uart::uart_t::Three);
}
void UART4_IRQHandler(void) {
   uart::irq_handler(uart::uart_t::Four);
}
void UART5_IRQHandler(void) {
   uart::irq_handler(uart::uart_t::Five);
}
void USART6_IRQHandler(void) {
   uart::irq_handler(uart::uart_t::Six);
}
}