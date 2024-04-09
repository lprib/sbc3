#pragma once

#include <span>

namespace screen {
constexpr int DISPLAY_WIDTH = 256;
constexpr int DISPLAY_HEIGHT = 64;
constexpr std::size_t DISPLAY_SIZE = DISPLAY_WIDTH * DISPLAY_HEIGHT;

void init();
std::span<unsigned char, DISPLAY_SIZE> get_dislay_buffer();
void update();
}
