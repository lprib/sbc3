#pragma once

#include <span>

namespace vmhost {
void init(std::span<unsigned char const> program);
}