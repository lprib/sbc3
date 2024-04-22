#include "BaseModuleCommon.hpp"

#include <algorithm>
#include <vector>

namespace vm {

StackWord BaseModuleCommon::sbuf_new(Machine& m, StackWord len) {
   auto buf = std::vector<unsigned char>(
      static_cast<std::size_t>(len),
      static_cast<unsigned char>(0)
   );
   auto handle_idx = m.sbufs().size();
   m.sbufs().push_back(std::move(buf));
   return handle_idx;
}
void BaseModuleCommon::sbuf_delete(Machine& m, StackWord sbuf) {
   // todo
}
void BaseModuleCommon::sbuf_copyto(
   Machine& m, StackWord start, StackWord len, StackWord dest_start,
   StackWord sbuf
) {
   // todo bounds check
   auto& dest_sbuf = m.sbufs()[sbuf];
   std::copy(
      m.current_module().code().begin() + start,
      m.current_module().code().begin() + start + len,
      dest_sbuf.begin() + dest_start
   );
}
void BaseModuleCommon::sbuf_copyfrom(
   Machine& m, StackWord start, StackWord len, StackWord dest_start,
   StackWord sbuf
) {
   auto src_sbuf = m.sbufs()[sbuf];
   std::copy(
      src_sbuf.begin() + start,
      src_sbuf.begin() + start + len,
      m.current_module().code().begin() + dest_start
   );
}
void BaseModuleCommon::sbuf_writebyte(
   Machine& m, StackWord index, StackWord byte, StackWord sbuf
) {
   auto& dest_sbuf = m.sbufs()[sbuf];
   dest_sbuf[index] = static_cast<unsigned char>(byte);
}

StackWord BaseModuleCommon::sbuf_readbyte(
   Machine& m, StackWord index, StackWord sbuf
) {
   auto& src_sbuf = m.sbufs()[sbuf];
   return static_cast<StackWord>(src_sbuf[index]);
}
void BaseModuleCommon::sbuf_fill(
   Machine& m, StackWord start, StackWord len, StackWord byte, StackWord sbuf
) {
   auto& dest_sbuf = m.sbufs()[sbuf];
   std::fill(
      dest_sbuf.begin() + start,
      dest_sbuf.begin() + start + len,
      static_cast<unsigned char>(byte)
   );
}

} // namespace vm