#include "engine.hpp"
#include <cstdio>
#include <fstream>
#include <iterator>
#include <vector>

void print(engine::Engine& e) {
   printf("%d\n", int{e.pop()});
}

int main(int argc, char** argv) {
   engine::Intrinsic intrs[] = {&print};
   auto e = engine::Engine{intrs};

   std::ifstream file(argv[1], std::ifstream::binary | std::ifstream::in);
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

   e.execute_module(vec);
}