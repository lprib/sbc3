// #include "engine.hpp"
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <optional>
#include <vector>

#include "BytecodeModule.hpp"
#include "IPlatform.hpp"
#include "ISystemModule.hpp"
#include "Machine.hpp"

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
         std::printf("system.print: %hu\n", machine.stack().pop());
         break;
      default:
         std::printf("unknown System call: %d\n", fn_id);
         break;
      }
   }

private:
   System() : vm::ISystemModule("system") {}
};

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

   auto res = m.execute_first_module();
   if(res.has_value()) {
      std::printf("%s\n", vm::error_to_str(mod.error()));
   } else {
      std::printf("ok\n");
   }
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