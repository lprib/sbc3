// #include "engine.hpp"
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <optional>
#include <vector>

#include "BytecodeModule.hpp"
#include "GraphicsModule.hpp"
#include "IPlatform.hpp"
#include "ISystemModule.hpp"
#include "Machine.hpp"

#include "raylib.h"

static std::vector<unsigned char> load_from_filename(char const* filename);

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

      // fudge for now
      if (name == "b") {
         auto file = load_from_filename("./vm/programs/module_test_b.bin");
         auto mod = vm::BytecodeModule::load(file);

         if(!mod.has_value()) {
            std::printf("%s\n", vm::error_to_str(mod.error()));
            return std::nullopt;
         }

         return *mod;
      }

      return std::nullopt;
   }

private:
   Platform(){};
};

class System final : public vm::ISystemModule {
public:
   System(const System&) = delete;
   System& operator=(const System&) = delete;
   static System& instance() {
      static System inst;
      return inst;
   }

   void invoke_index(vm::Machine& machine, int fn_id) override {
      switch(fn_id) {
      case 0:
         std::printf("%d\n", machine.stack().pop());
         break;
      // on-target profiling (ignore):
      case 1:
         machine.stack().pop();
         break;
      case 2:
         machine.stack().pop();
         break;
      default:
         std::printf("unknown System call: %d\n", fn_id);
         break;
      }
   }

private:
   System() : vm::ISystemModule("system") {}
};

#define CONSOLE

// TODO need to build an index of working_dir module names first!

int main(int argc, char** argv) {
   if(argc != 2) {
      std::printf("usage: vm program.bin\n");
      std::exit(1);
   }

   auto m = vm::Machine(Platform::instance());
   m.add_system_module(&System::instance());

   auto file = load_from_filename(argv[1]);
   auto mod = vm::BytecodeModule::load(file);

   if(!mod.has_value()) {
      std::printf("%s\n", vm::error_to_str(mod.error()));
      return 1;
   }

   auto mod_v = mod.value();

   m.add_module(mod_v);

#ifdef CONSOLE
   // std::printf("exe program entry")
   // TODO don't hardcode "a" module name
   auto res = m.execute("a", "entry");
   if(res.has_value()) {
      std::printf("%s\n", vm::error_to_str(mod.error()));
   } else {
      std::printf("ok\n");
   }

#else
   m.add_system_module(&GraphicsModule::instance());

   auto err = m.execute("program", "entry");

   if(err.has_value()) {
      std::cout << vm::error_to_str(err.value()) << "\n";
      exit(1);
   }

   InitWindow(
      GraphicsModule::SCREEN_WIDTH_PX,
      GraphicsModule::SCREEN_HEIGHT_PX,
      "vm graphics"
   );

   SetTargetFPS(60);           // Set our game to run at 60 frames-per-second
   while(!WindowShouldClose()) // Detect window close button or ESC key
   {
      m.execute("program", "frame");
      BeginDrawing();
      {
         ClearBackground(BLACK);
         GraphicsModule::instance().draw(m);
         DrawFPS(0, 0);
      }
      EndDrawing();
   }

   CloseWindow(); // Close window and OpenGL context
#endif
}

static std::vector<unsigned char> load_from_filename(char const* filename) {
   std::ifstream file(filename, std::ifstream::binary | std::ifstream::in);
   file.unsetf(std::ios::skipws);
   std::streampos filesize;
   file.seekg(0, std::ios::end);
   filesize = file.tellg();
   file.seekg(0, std::ios::beg);

   std::vector<unsigned char> vec;
   vec.reserve(filesize);

   // read the data:
   vec.insert(
      vec.begin(),
      std::istream_iterator<unsigned char>(file),
      std::istream_iterator<unsigned char>()
   );

   return vec;
}