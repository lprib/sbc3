#pragma once

#include <span>
#include <string_view>

namespace serial {
void init();

/// None of these are interrupt safe:

void print(unsigned char n);
void print(std::span<unsigned char const> ns);
void print(std::string_view str);
void println(std::string_view str);

} // namespace serial
