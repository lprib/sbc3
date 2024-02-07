#pragma once

#include <span>
#include <string_view>

namespace serial {
void init();

/// @brief blocking TX. Not interrupt safe.
void tx(unsigned char n);
/// @brief blocking TX. Not interrupt safe.
void tx(std::span<unsigned char const> ns);
/// @brief blocking TX. Not interrupt safe.
void tx(std::string_view str);

} // namespace serial
