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

void tx(std::span<unsigned char const> ns) {
   sync::lock_t l{serial_mutex};
   for(auto n : ns) {
      uart::tx_block(SERIAL_UART, n);
   }
   uart::wait_for_tx_complete(SERIAL_UART);
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

static void onrx() {
   unsigned char data = uart::read_byte(SERIAL_UART);
   gpio::dbg_led.write(data & 1);
}

} // namespace serial