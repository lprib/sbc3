#pragma once

#include <span>
#include <string_view>

namespace serial {
void init();
void block_tx(unsigned char n);
void block_tx(std::span<unsigned char const> ns);
void block_tx(std::string_view str);
} // namespace serial
