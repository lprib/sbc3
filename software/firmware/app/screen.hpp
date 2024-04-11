#pragma once

#include <span>

namespace screen {
constexpr int DISPLAY_WIDTH = 256;
constexpr int DISPLAY_HEIGHT = 64;
constexpr std::size_t DISPLAY_SIZE = DISPLAY_WIDTH * DISPLAY_HEIGHT;

void init();
void update(std::span<unsigned char, DISPLAY_SIZE> display_buffer);
} // namespace screen
