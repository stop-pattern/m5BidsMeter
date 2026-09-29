#pragma once

#include "meter.h"

namespace meter {

/** RGB888 color shared by PC preview and Core2 display backends. */
using Color = unsigned;
/** Main screen background color. */
constexpr Color BG = 0x202a3c;
/** Gauge and selection panel fill color. */
constexpr Color PANEL = 0x303d55;
/** Border color for inactive panels. */
constexpr Color EDGE = 0x6d7f99;
/** Primary text and gauge color. */
constexpr Color WHITE = 0xf4f5f4;
/** Secondary text color. */
constexpr Color MUTED = 0xa5afbc;
/** Lit status and proceed-aspect color. */
constexpr Color GREEN = 0x8fe472;
/** Warning and stop-aspect color. */
constexpr Color RED = 0xff5555;
/** Active brake-bar color. */
constexpr Color YELLOW = 0xffe04c;
/** Main-reservoir pressure color. */
constexpr Color ORANGE = 0xffaa67;

/** Minimal draw API implemented by Core2 and PC preview surfaces. */
class Canvas {
 public:
  /** Releases the polymorphic drawing surface. */
  virtual ~Canvas() {}
  /** Fills a rectangle in screen coordinates. */
  virtual void rect(int x, int y, int w, int h, Color color) = 0;
  /** Draws a line with the requested thickness. */
  virtual void line(int x1, int y1, int x2, int y2, Color color, int width = 1) = 0;
  /** Draws UTF-8 text using a readable font near the requested size. */
  virtual void text(int x, int y, const char* value, Color color, int size,
                    bool center = false) = 0;
  /** Fills a circle at its center point. */
  virtual void circle(int x, int y, int r, Color color) = 0;
};

/** Draws one full meter frame from the current state. */
void render(Canvas& c, const State& state, uint32_t nowMs);
/** Applies one touch on the 320 by 240 screen to navigation state. */
void tap(State& state, int x, int y);

}  // namespace meter
