#pragma once

#include "meter.h"

namespace meter {

using Color = unsigned;
constexpr Color BG = 0x202a3c;
constexpr Color PANEL = 0x303d55;
constexpr Color EDGE = 0x6d7f99;
constexpr Color WHITE = 0xf4f5f4;
constexpr Color MUTED = 0xa5afbc;
constexpr Color GREEN = 0x8fe472;
constexpr Color RED = 0xff5555;
constexpr Color YELLOW = 0xffe04c;
constexpr Color ORANGE = 0xffaa67;

class Canvas {
 public:
  virtual ~Canvas() {}
  virtual void rect(int x, int y, int w, int h, Color color) = 0;
  virtual void line(int x1, int y1, int x2, int y2, Color color, int width = 1) = 0;
  virtual void text(int x, int y, const char* value, Color color, int size, bool center = false) = 0;
  virtual void circle(int x, int y, int r, Color color) = 0;
};

void render(Canvas& c, const State& state, uint32_t nowMs);
void tap(State& state, int x, int y);

}  // namespace meter
