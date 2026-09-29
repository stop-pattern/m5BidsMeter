#include "core2_input.h"

#include <M5Unified.h>

#include "render.h"

namespace meter {

bool Core2Input::update(State& state) {
  if (M5.BtnB.isPressed() && M5.BtnB.pressedFor(800) && !longPressHandled_) {
    state.screen = Screen::Select;
    longPressHandled_ = true;
  }
  if (M5.BtnB.wasReleased()) {
    if (!longPressHandled_) state.screen = Screen::Home;
    longPressHandled_ = false;
  }

  const auto touch = M5.Touch.getDetail();
  if (!touch.wasPressed() || touch.y < 0 || touch.y >= 240) return false;

  const uint8_t previousBrightness = state.brightness;
  tap(state, touch.x, touch.y);
  return previousBrightness != state.brightness;
}

}  // namespace meter
