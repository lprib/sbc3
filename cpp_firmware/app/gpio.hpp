#pragma once

namespace gpio {

enum class Pin {
   DebugLed,
   //    Not connected :(
   //    LeftButton,
   //    RightButton,
   //    RotaryA,
   //    RotaryB,
   LedSer,
   LedSrClk,
   LedSrClr,
   LedRclk,
   Gp0,
   Gp1,
   Gp2,
   Gp3,
   Gp4,
   Gp5,
   Gp6,
   Gp7,
};

void configure_clocks();
void to_lowspeed_pp_out(Pin pin);
void to_lowspeed_in(Pin pin);
void write(Pin pin, bool value);
bool read(Pin pin);
void toggle(Pin pin);

} // namespace gpio