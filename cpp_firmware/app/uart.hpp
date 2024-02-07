#pragma once

namespace uart {

enum class uart_t { One, Two, Three, Four, Five, Six, NumUarts };

using isr_callback_t = void (*)();

void init(uart_t u, unsigned baud_rate);

/// @brief Block until data register is free to tx again
void tx_block(uart_t u, unsigned char n);
/// @brief Block until transmission complete
void wait_for_tx_complete(uart_t u);

/// @brief (unsafe) write direct to data register
void write_byte(uart_t u, unsigned char n);
/// @brief (unsafe) read direct from data register
unsigned char read_byte(uart_t u);

///  Register callback for interrupts:

void set_isr_tx_ready(uart_t u, isr_callback_t cb);
void set_isr_tx_complete(uart_t u, isr_callback_t cb);
void set_isr_rx_ready(uart_t u, isr_callback_t rb);

} // namespace uart