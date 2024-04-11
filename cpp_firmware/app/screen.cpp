#include "screen.hpp"

#include <array>
#include <span>

#include "gpio.hpp"
#include "util.hpp"

namespace screen {

enum class Command : uint8_t {
   EnableGrayScaleTable = 0x00,
   SetColumnAddress = 0x15,
   WriteRam = 0x5c,
   ReadRam = 0x5d,
   SetRowAddress = 0x75,
   SetRemapMode = 0xa0,
   SetStartLine = 0xa1,
   SetOffset = 0xa2,
   SetModeAllOff = 0xa4,
   SetModeAllOn = 0xa5,
   SetModeNormal = 0xa6,
   SetModeInverse = 0xa7,
   EnablePartialDisplay = 0xa8,
   ExitPartialDisplay = 0xa9,
   VddFunctionSelect = 0xab,
   SetDisplayOff = 0xae,
   SetDisplayOn = 0xaf,
   SetPhaseLength = 0xb1,
   SetClockDivAndOscFreq = 0xb3,
   DisplayEnhancementA = 0xb4,
   SetGpio = 0xb6,
   SetSecondPrechargePeriod = 0xb6,
   SetGrayScaleTable = 0xb8,
   SetDefaultGrayScaleTable = 0xb9,
   SetPrechargeVoltage = 0xbb,
   SetVcomh = 0xbe,
   SetContrastCurrent = 0xc1,
   MasterContrastCurrentControl = 0xc7,
   SetMuxRatio = 0xca,
   SetCommandLock = 0xfd,
};

/// @brief value corresponds to state of DC line
enum class TransactionType : uint8_t {
   Command = 0,
   Data = 1,
};

static std::array<unsigned char, DISPLAY_SIZE> display_buffer;

static inline unsigned char display_buffer_at(int x, int y) {
   return display_buffer[static_cast<std::size_t>(y * DISPLAY_HEIGHT + x)];
}

static std::array<uint32_t, 8> data_pins = {
   LL_GPIO_PIN_0,
   LL_GPIO_PIN_1,
   LL_GPIO_PIN_2,
   LL_GPIO_PIN_3,
   LL_GPIO_PIN_4,
   LL_GPIO_PIN_5,
   LL_GPIO_PIN_6,
   LL_GPIO_PIN_7,
};

static constexpr unsigned char COL_START = 28;
static constexpr unsigned char COL_END = 92;
// static constexpr unsigned char COL_START = 0;
// static constexpr unsigned char COL_END = 64;
static constexpr unsigned char ROW_START = 0;
static constexpr unsigned char ROW_END = 64;

static inline void write_data_bus(unsigned char data) {
   volatile auto gpioc_low8 = reinterpret_cast<volatile uint8_t*>(&GPIOD->ODR);
   *gpioc_low8 = data;
}

static inline void write_byte(TransactionType type, char data) {
   write_data_bus(data);
   gpio::scr_dc.write(static_cast<bool>(type));
   gpio::scr_cs.write(false);
   util::spinloop_us(1);
   gpio::scr_rw.write(false);
   util::spinloop_us(1);
   gpio::scr_rw.write(true);
   util::spinloop_us(1);
   gpio::scr_cs.write(true);
   util::spinloop_us(1);
}

static inline void command(
   Command cmd, std::initializer_list<unsigned char> args = {}
) {
   write_byte(TransactionType::Command, static_cast<unsigned char>(cmd));
   for(auto arg : args) {
      write_byte(TransactionType::Data, arg);
   }
}

static void set_col_addr(unsigned char start, unsigned char end) {
   command(
      Command::SetColumnAddress,
      {(unsigned char)(start & 0x7f), (unsigned char)(end & 0x7f)}
   );
}
static void set_row_addr(unsigned char start, unsigned char end) {
   command(
      Command::SetRowAddress,
      {(unsigned char)(start & 0x7f), (unsigned char)(end & 0x7f)}
   );
}

static void reset() {
   gpio::scr_res.write(true);
   util::spinloop_us(100000);
   gpio::scr_res.write(false);
   util::spinloop_us(100000);
   gpio::scr_res.write(true);

   gpio::scr_cs.write(true);
   gpio::scr_rw.write(true);
   gpio::scr_e.write(true);

   gpio::scr_cs.write(true);
}

void init() {
   gpio::scr_e.to_highspeed_pp_out();
   gpio::scr_rw.to_highspeed_pp_out();
   gpio::scr_dc.to_highspeed_pp_out();
   gpio::scr_cs.to_highspeed_pp_out();
   gpio::scr_res.to_highspeed_pp_out();

   for(unsigned short n : data_pins) {
      gpio::pin_t{GPIOD, n}.to_highspeed_pp_out();
   }

   reset();

   command(Command::SetCommandLock, {0x12});
   command(Command::SetDisplayOn);
   command(
      Command::SetClockDivAndOscFreq,
      {
         0          // clock div by 1
         | (1 << 4) // osc freq 1
      }
   );
   command(Command::SetMuxRatio, {0x3f});
   command(Command::SetStartLine, {0});
   command(Command::SetOffset, {0});
   command(
      Command::SetRemapMode,
      {
         (0 << 0)       // address increment horizontal
            | (0 << 1)  // column address remap off
            | (1 << 2)  // nibble remap on
            | (1 << 4)  // scan direction downwards
            | (0 << 5), // split odd even off

         1 | (1 << 4) // dual com line on
      }
   );
   command(Command::VddFunctionSelect, {1}); // vdd internal
   // command(Command::) // TODO display enhancement A
   command(Command::SetContrastCurrent, {0x80});
   command(Command::MasterContrastCurrentControl, {8});
   command(
      Command::SetPhaseLength,
      {
         (2 << 0)    // phase 1
         | (15 << 4) // phase 2
      }
   );
   command(Command::SetPrechargeVoltage, {13});
   command(Command::SetVcomh, {0});
   command(Command::SetModeNormal);
   // command(Command::SetModeAllOn);
   command(Command::SetDisplayOn);
}

std::span<unsigned char, DISPLAY_SIZE> get_dislay_buffer() {
   return display_buffer;
}

void update() {
   set_col_addr(COL_START, COL_END - 1);
   set_row_addr(ROW_START, ROW_END - 1);
   command(Command::WriteRam);

   gpio::scr_dc.write(static_cast<bool>(TransactionType::Data));
   gpio::scr_cs.write(false);
   util::spinloop_us(1);

   for(int y = 0; y < DISPLAY_HEIGHT; ++y) {
      for(int x_quad = 0; x_quad < (DISPLAY_WIDTH / 4); ++x_quad) {
         auto byte_1 = (display_buffer_at(x_quad * 4 + 0, y) << 4) |
                       display_buffer_at(x_quad * 4 + 1, y);
         auto byte_2 = (display_buffer_at(x_quad * 4 + 2, y) << 4) |
                       display_buffer_at(x_quad * 4 + 3, y);

         gpio::scr_rw.write(false);
         write_data_bus(0xf0);
         util::spinloop_cycles(12); // 10 cycles @ 168Mhz = 59ns t_PWHW(60ns)
         gpio::scr_rw.write(true);
         util::spinloop_cycles(42); // 42 cycles @ 168Mhz = 247ns t_PWHW(60ns), t_cycle(300ns accounting for the previous 60ns)

         gpio::scr_rw.write(false);
         write_data_bus(0x80);
         util::spinloop_cycles(12); // 10 cycles @ 168Mhz = 59ns t_PWHW(60ns)
         gpio::scr_rw.write(true);
         util::spinloop_cycles(42); // 42 cycles @ 168Mhz = 247ns t_PWHW(60ns), t_cycle(300ns accounting for the previous 60ns)
      }
   }

   gpio::scr_cs.write(true);
}

} // namespace screen