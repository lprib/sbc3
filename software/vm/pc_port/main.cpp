// #include "engine.hpp"
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <optional>
#include <vector>

#include "BaseModuleShim.hpp"
#include "BytecodeModule.hpp"
#include "IPlatform.hpp"
#include "ISystemModule.hpp"
#include "Machine.hpp"
#include "RaylibGraphicsModule.hpp"

#include "raylib.h"

static void load_all_bytecode_from_working_dir(
   std::filesystem::path working_dir, vm::Machine& m
);

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
                << "' but it was not found (all modules loaded once on startup "
                   "in this port)\n";
      return std::nullopt;
   }

private:
   Platform(){};
};

class Base final : public vm::BaseModuleShim {
public:
   void print_decimal(vm::StackWord word) override {
      std::printf("%d\n", word);
   }

   void profile_start(vm::StackWord id) override {
      // not impl
   }
   void profile_end(vm::StackWord id) override {
      // not impl
   }

private:
};

// #define CONSOLE

// TODO need to build an index of working_dir module names first!

int main(int argc, char** argv) {
   if(argc != 3) {
      std::printf("usage: vm working_dir module_name\n");
      std::exit(1);
   }

   auto m = vm::Machine(Platform::instance());
   auto base_module = Base();
   m.add_system_module(&base_module);

   load_all_bytecode_from_working_dir(argv[1], m);

#ifdef CONSOLE
   auto res = m.execute(argv[2], "entry");
   if(res.has_value()) {
      std::printf("%s\n", vm::error_to_str(*res));
   } else {
      std::printf("ok\n");
   }

#else
   auto raylib_graphics_module = RaylibGraphicsModule();
   m.add_system_module(&raylib_graphics_module);

   auto err = m.execute(argv[2], "entry");

   if(err.has_value()) {
      std::cout << vm::error_to_str(err.value()) << "\n";
      exit(1);
   }

   InitWindow(
      RaylibGraphicsModule::SCREEN_WIDTH_PX,
      RaylibGraphicsModule::SCREEN_HEIGHT_PX,
      "vm graphics"
   );

   SetTargetFPS(30);
   while(!WindowShouldClose())
   {
      m.execute(argv[2], "frame");
      BeginDrawing();
      {
         ClearBackground(BLACK);
         raylib_graphics_module.draw(m);
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

static void load_all_bytecode_from_working_dir(
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
