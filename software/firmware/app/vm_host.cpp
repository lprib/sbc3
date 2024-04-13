#include "vm_host.hpp"

#include <cstdio>

#include "FreeRTOS.h"
#include "task.h"

#include "buttons.hpp"
#include "error.hpp"
#include "screen.hpp"
#include "serial.hpp"

#include "BytecodeModule.hpp"
#include "IPlatform.hpp"
#include "ISystemModule.hpp"
#include "Machine.hpp"

namespace vmhost {

void task(void* arg);

// TODO this is copy-pasted from raylib port, should share more for generic fns

class Platform final : public vm::IPlatform {
public:
   Platform(const Platform&) = delete;
   Platform& operator=(const Platform&) = delete;
   static Platform& instance() {
      static Platform inst;
      return inst;
   }

   std::optional<vm::BytecodeModule> get_module(std::string_view name
   ) override {
      return std::nullopt;
   }

private:
   Platform(){};
};

class ProfileTimer {
public:
   void start() {
      m_start_time = xTaskGetTickCount();
   }
   void end() {
      m_end_time = xTaskGetTickCount();
   }
   long elapsed() {
      return m_end_time - m_start_time;
   }

private:
   long m_start_time = 0;
   long m_end_time = 0;
};

class System final : public vm::ISystemModule {
public:
   System(const System&) = delete;
   System& operator=(const System&) = delete;
   static System& instance() {
      static System inst;
      return inst;
   }

   enum Fn {
      PRINT_DECIMAL = 0,
      PROFILE_START = 1,
      PROFILE_END = 2,
   };

   void invoke_index(vm::Machine& machine, int fn_id) override {
      switch(fn_id) {
      case PRINT_DECIMAL: {
         // std::printf("%d\n", machine.stack().pop());
         char out[20];
         std::snprintf(out, 20, "%d", machine.stack().pop());
         serial::println(out);
      } break;
      case PROFILE_START: {
         auto idx = machine.stack().pop();
         if(idx < N_PROFILE_TIMER) {
            m_profile_timers[idx].start();
         }
      } break;
      case PROFILE_END: {
         auto idx = machine.stack().pop();
         if(idx < N_PROFILE_TIMER) {
            m_profile_timers[idx].end();
         }
      } break;
      default:
         error::error();
         break;
      }
   }

   std::span<ProfileTimer> profile_timers() {
      return m_profile_timers;
   }

private:
   System() : vm::ISystemModule("system") {}
   static constexpr int N_PROFILE_TIMER = 8;
   std::array<ProfileTimer, N_PROFILE_TIMER> m_profile_timers;
};

class GraphicsModule final : public vm::ISystemModule {
public:
   GraphicsModule(const GraphicsModule&) = delete;
   GraphicsModule& operator=(const GraphicsModule&) = delete;
   static GraphicsModule& instance() {
      static GraphicsModule inst;
      return inst;
   }

   static constexpr int SCREEN_WIDTH = 256;
   static constexpr int SCREEN_HEIGHT = 64;
   enum Fn {
      SET_DISPLAY_BUF = 0,
      IS_KEY_DOWN = 1,
      BLIT = 2,
      IS_KEY_PRESSED = 3,
      CLEAR_SCREEN = 4,
   };

   void invoke_index(vm::Machine& machine, int fn_id) override {
      switch(fn_id) {
      case SET_DISPLAY_BUF:
         // ( buffptr -- )
         m_display_buff_bytecode_address = machine.stack().pop();
         break;
      case IS_KEY_DOWN: {
         // ( key -- down? )
         auto key = machine.stack().pop();
         auto result = vm::Machine::TRUE_WORD;
         if(key == 65) {
            // a
            result = buttons::get_state(buttons::Button::Left)
                        ? vm::Machine::TRUE_WORD
                        : vm::Machine::FALSE_WORD;
         } else if(key == 68) {
            // d
            result = buttons::get_state(buttons::Button::Right)
                        ? vm::Machine::TRUE_WORD
                        : vm::Machine::FALSE_WORD;
         } else if(key == 87) {
            // s
            result = buttons::get_state(buttons::Button::Rotary)
                        ? vm::Machine::TRUE_WORD
                        : vm::Machine::FALSE_WORD;
         }
         machine.stack().push(result);
      } break;
      case IS_KEY_PRESSED: {
         // ( key -- down? )
         error::error();
      } break;
      case BLIT: {
         // (x y spriteptr -- )
         auto code = machine.current_module().code();

         auto spriteptr = machine.stack().pop();
         auto y = machine.stack().pop();
         auto x = machine.stack().pop();
         auto sprite_width = code[spriteptr];
         auto sprite_height = code[spriteptr + 1];
         auto sprite =
            code.subspan(spriteptr + 2, sprite_width * sprite_height);

         // std::cout << "blit x:" << x << " y:" << y << " ptr:" << spriteptr <<
         // " w:"
         //           << sprite_width << " h:" << sprite_height << "\n";

         for(int y_iter = 0; y_iter < sprite_height; ++y_iter) {
            auto dest_y = y + y_iter;
            auto src_row = sprite.subspan(y_iter * sprite_width, sprite_width);
            auto dest_row = code.subspan(
               m_display_buff_bytecode_address + dest_y * SCREEN_WIDTH + x,
               sprite_width
            );
            std::copy(src_row.begin(), src_row.end(), dest_row.begin());
         }
      } break;
      case CLEAR_SCREEN: {
         // TODO this should probably be in system module and a generic memset
         auto display = machine.current_module().code().subspan(
            m_display_buff_bytecode_address,
            SCREEN_WIDTH * SCREEN_HEIGHT
         );
         std::fill(display.begin(), display.end(), 0);
      } break;
      default:
         std::cout << "unknown graphics call " << fn_id << "\n";
      }
   }

   void draw(vm::Machine& machine) {
      auto progmem = machine.current_module().code();
      screen::update(std::span<unsigned char, screen::DISPLAY_SIZE>(
         progmem.subspan(m_display_buff_bytecode_address, screen::DISPLAY_SIZE)
      ));
   }

private:
   int m_display_buff_bytecode_address = 0;

   GraphicsModule() : vm::ISystemModule("graphics") {}
};

std::span<unsigned char const> program;

void init(std::span<unsigned char const> _program) {
   program = _program;

   xTaskCreate(
      &task,
      "mmi",
      configMINIMAL_STACK_SIZE * 50,
      NULL,
      tskIDLE_PRIORITY + 2,
      NULL
   );
}

void task(void* arg) {
   auto m = vm::Machine(Platform::instance());
   m.add_system_module(&System::instance());
   m.add_system_module(&GraphicsModule::instance());

   auto module = vm::BytecodeModule::load(program);

   if(!module.has_value()) {
      error::error();
   }

   m.add_module(module.value());

   auto err = m.execute("program", "entry");
   if(err.has_value()) {
      error::error();
   }

   while(true) {
      long start_vm = xTaskGetTickCount();
      auto err = m.execute("program", "frame");
      long elapsed_vm = xTaskGetTickCount() - start_vm;

      if(err.has_value()) {
         error::error();
      }

      long start_draw = xTaskGetTickCount();
      GraphicsModule::instance().draw(m);
      long elapsed_draw = xTaskGetTickCount() - start_draw;

      char out[40];
      std::snprintf(out, 40, "vm: %ld draw:%ld", elapsed_vm, elapsed_draw);
      serial::println(out);

      // report profile timers
      int i = 0;
      for(auto n : System::instance().profile_timers()) {
         char out[10];
         std::snprintf(out, 10, "%d: %ld", i, n.elapsed());
         serial::println(out);
         ++i;
      }
   }
}

} // namespace vmhost