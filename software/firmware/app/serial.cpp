#include "serial.hpp"

#include "error.hpp"
#include "sync.hpp"
#include "uart.hpp"

// test
#include "gpio.hpp"

namespace serial {

static sync::mutex_t serial_mutex;

static constexpr uart::uart_t SERIAL_UART = uart::uart_t::Three;

static void onrx();

void init() {
   uart::init(SERIAL_UART, 115200);
   uart::set_isr_rx_ready(SERIAL_UART, &onrx);
}

static void print_inner(std::span<unsigned char const> ns) {
   for(auto n : ns) {
      uart::tx_block(SERIAL_UART, n);
   }
   uart::wait_for_tx_complete(SERIAL_UART);
}

void print(std::span<unsigned char const> ns) {
   sync::lock_t lock{serial_mutex};
   print_inner(ns);
}

void print(unsigned char n) {
   sync::lock_t lock{serial_mutex};
   print_inner(std::span(&n, 1));
}

void print(std::string_view str) {
   sync::lock_t lock{serial_mutex};
   print_inner(std::span(
      reinterpret_cast<unsigned char const*>(str.begin()),
      reinterpret_cast<unsigned char const*>(str.end())
   ));
}

void println(std::string_view str) {
   sync::lock_t lock{serial_mutex};
   for(auto n : str) {
      uart::tx_block(SERIAL_UART, static_cast<unsigned char>(n));
   }
   uart::tx_block(SERIAL_UART, static_cast<unsigned char>('\n'));
   uart::wait_for_tx_complete(SERIAL_UART);
}

static void onrx() {
   unsigned char data = uart::read_byte(SERIAL_UART);
   gpio::dbg_led.write(data & 1);
}

} // namespace serial