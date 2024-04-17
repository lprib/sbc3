// #include "engine.hpp"
#include <cstdio>
#include <cstdlib>
#include <filesystem>
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

static void load_all_modules(std::filesystem::path working_dir, vm::Machine& m);

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
      std::cout << "bytecode is requesting module '" << name
                << "' but we load everything on startup\n";
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

// #define CONSOLE

// TODO need to build an index of working_dir module names first!

int main(int argc, char** argv) {
   if(argc != 3) {
      std::printf("usage: vm working_dir module_name\n");
      std::exit(1);
   }

   auto m = vm::Machine(Platform::instance());
   m.add_system_module(&System::instance());

   load_all_modules(argv[1], m);

#ifdef CONSOLE
   auto res = m.execute(argv[2], "entry");
   if(res.has_value()) {
      std::printf("%s\n", vm::error_to_str(*res));
   } else {
      std::printf("ok\n");
   }

#else
   m.add_system_module(&GraphicsModule::instance());

   auto err = m.execute(argv[2], "entry");

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
      m.execute(argv[2], "frame");
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

static std::vector<unsigned char> read_entire_contents(std::ifstream& file) {
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

static void load_all_modules(
   std::filesystem::path working_dir, vm::Machine& m
) {
   for(auto const& dir_entry :
       std::filesystem::directory_iterator{working_dir}) {
      if(dir_entry.is_regular_file()) {
         std::ifstream file(dir_entry.path(), std::ifstream::binary);
         unsigned char magic[4];
         file.read(reinterpret_cast<char*>(magic), sizeof(magic));

         if(!file.good())
            continue;

         if(!((magic[0] == 0x5B) && (magic[1] == 0xC3) && (magic[2] == 0xB0) &&
              (magic[3] == 0x0F)))
            continue;

         file.clear();
         file.seekg(0);

         auto bytecode = read_entire_contents(file);
         auto mod = vm::BytecodeModule::load(bytecode);

         if(!mod.has_value()) {
            std::cout << dir_entry.path().c_str()
                      << "had valid magic but could not be loaded: "
                      << vm::error_to_str(mod.error()) << "\n";
            continue;
         }

         std::cout << "loaded " << dir_entry.path().c_str() << " as '"
                   << mod->name() << "'\n";

         m.add_module(std::move(*mod));
      }
   }
}
