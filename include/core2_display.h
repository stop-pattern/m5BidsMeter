#pragma once

#include <M5Unified.h>
#include <stdint.h>

#include "meter.h"

namespace meter {

/** Renders into off-screen buffers and transfers only changed LCD bands. */
class Core2Display {
 public:
  /** Creates the two 320 by 240 RGB565 frame buffers. */
  Core2Display();

  /** Sets up the display after M5.begin(); returns false on allocation failure.
   */
  bool begin();

  /** Sets the Core2 backlight to the current user-selected level. */
  void setBrightness(uint8_t percent);

  /** Renders the current state and sends changed LCD bands. */
  void update(const State& state, uint32_t nowMs);

 private:
  /** First full-resolution off-screen frame. */
  M5Canvas frameA_;
  /** Second full-resolution off-screen frame. */
  M5Canvas frameB_;
  /** Previously displayed frame for pixel comparison. */
  M5Canvas* previous_;
  /** Frame used for the next draw. */
  M5Canvas* current_;
  /** True when both off-screen frames are available. */
  bool buffered_ = false;
  /** True after the first frame reaches the LCD. */
  bool hasPrevious_ = false;
  /** Screen shown in the previous frame. */
  Screen previousScreen_ = Screen::Home;
  /** Safety type shown in the previous frame. */
  Safety previousSafety_ = Safety::Ats;
  /** Scheduled time of the last frame render. */
  uint32_t lastDrawMs_ = 0;

  /** Copies only changed horizontal bands from the new frame to the LCD. */
  void transferChanges(const State& state);
};

}  // namespace meter
