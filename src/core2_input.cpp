#include "core2_input.h"

#include <M5Unified.h>

#include "render.h"

namespace meter {
namespace {

/** Single-frequency button tone, adjustable without changing input logic. */
constexpr unsigned kClickHz = 1600;
/** Duration of one button tone in milliseconds. */
constexpr unsigned kClickMs = 55;

/** Plays one action tone only while the user has enabled sound. */
void playClick(const State& state) {
  if (state.soundEnabled) M5.Speaker.tone(kClickHz, kClickMs);
}

}  // namespace

bool Core2Input::update(State& state) {
  if (M5.BtnB.isPressed() && M5.BtnB.pressedFor(800) && !longPressHandled_) {
    state.screen = Screen::Select;
    longPressHandled_ = true;
    playClick(state);
  }
  if (M5.BtnB.wasReleased()) {
    if (!longPressHandled_) {
      state.screen = Screen::Home;
      playClick(state);
    }
    longPressHandled_ = false;
  }

  const auto touch = M5.Touch.getDetail();
  if (!touch.wasPressed() || touch.y < 0 || touch.y >= 240) return false;

  const uint8_t previousBrightness = state.brightness;
  if (tap(state, touch.x, touch.y)) playClick(state);
  return previousBrightness != state.brightness;
}

}  // namespace meter
