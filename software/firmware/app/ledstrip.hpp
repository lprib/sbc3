#pragma once

#include "stdint.h"

namespace ledstrip
{
void init();
void write(uint16_t mask);
}