#include "vm_host.hpp"

#include <cstdio>

#include "FreeRTOS.h"
#include "task.h"

#include "buttons.hpp"
#include "error.hpp"
#include "screen.hpp"
#include "serial.hpp"

#include "BaseModuleShim.hpp"
#include "BytecodeModule.hpp"
#include "GraphicsModuleShim.hpp"
#include "IPlatform.hpp"
#include "ISystemModule.hpp"
#include "Machine.hpp"

namespace vmhost {

#define PRINT_PROFILE (0)

void task(void* arg);

// TODO this is copy-pasted from raylib port, should share more for generic fns

class Platform final : public vm::IPlatform {
public:
   std::optional<vm::BytecodeModule> get_module(std::string_view name
   ) override {
      return std::nullopt;
   }

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
   long elapsed() const {
      return m_end_time - m_start_time;
   }

private:
   long m_start_time = 0;
   long m_end_time = 0;
};

class Base final : public vm::BaseModuleShim {
public:
   void print_decimal(vm::StackWord word) override {
      char out[20];
      std::snprintf(out, 20, "%d", word);
      serial::println(out);
   }
   void profile_start(vm::StackWord id) override {
      if(id < N_PROFILE_TIMER) {
         m_profile_timers[id].start();
      }
   }
   void profile_end(vm::StackWord id) override {
      if(id < N_PROFILE_TIMER) {
         m_profile_timers[id].end();
      }
   }

   std::span<ProfileTimer> profile_timers() {
      return m_profile_timers;
   }

private:
   static constexpr int N_PROFILE_TIMER = 8;
   std::array<ProfileTimer, N_PROFILE_TIMER> m_profile_timers;
};

class GraphicsModule final : public vm::GraphicsModuleShim {
public:
   bool is_key_down(vm::StackWord key) override {
      if(key == 65) {
         // a
         return !buttons::get_state(buttons::Button::Left);
      } else if(key == 68) {
         // d
         return !buttons::get_state(buttons::Button::Right);
      } else if(key == 83) {
         // s
         return !buttons::get_state(buttons::Button::Rotary);
      } else {
         return false;
      }
   }

   bool is_key_pressed(vm::StackWord key) override {
      error::error(); // TODO
   }

   void draw(vm::Machine& machine) {
      auto progmem = machine.current_module().code();
      screen::update(std::span<unsigned char, screen::DISPLAY_SIZE>(
         progmem.subspan(m_display_buff_bytecode_address, screen::DISPLAY_SIZE)
      ));
   }
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
   auto platform = Platform();
   auto m = vm::Machine(platform);
   auto base = Base();
   auto graphics = GraphicsModule();
   m.add_system_module(&base);
   m.add_system_module(&graphics);

   auto module = vm::BytecodeModule::load(program);

   if(!module.has_value()) {
      error::error();
   }

   m.add_module(std::move(*module));

   auto err = m.execute("tilemap", "entry");
   if(err.has_value()) {
      error::error();
   }

   while(true) {
      long start_vm = xTaskGetTickCount();
      auto err = m.execute("tilemap", "frame");
      long elapsed_vm = xTaskGetTickCount() - start_vm;

      if(err.has_value()) {
         error::error();
      }

      long start_draw = xTaskGetTickCount();
      graphics.draw(m);
      long elapsed_draw = xTaskGetTickCount() - start_draw;

#if PRINT_PROFILE

      char out[40];
      std::snprintf(out, 40, "vm: %ld draw:%ld", elapsed_vm, elapsed_draw);
      serial::println(out);

      // report profile timers
      int i = 0;
      for(auto const& n : base.profile_timers()) {
         char out[10];
         std::snprintf(out, 10, "%d: %ld", i, n.elapsed());
         serial::println(out);
         ++i;
      }
#else
      (void)start_vm;
      (void)elapsed_vm;
      (void)start_draw;
      (void)elapsed_draw;
#endif
   }
}

} // namespace vmhost