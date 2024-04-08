#pragma once

namespace buttons {
constexpr int N_BUTTONS = 3;
enum class Button { Left, Right, Rotary };

void init();
bool get_state(Button button);
} // namespace buttons
