#include "render_internal.h"

namespace meter {

/** Draws the common frame and exactly one selected screen. */
void render(Canvas& c, const State& state, uint32_t nowMs) {
  renderCommon(c, state);
  switch (state.screen) {
    case Screen::Home:
      renderHome(c, state);
      break;
    case Screen::Speed:
      renderSpeed(c, state);
      break;
    case Screen::Pressure:
      renderPressure(c, state, nowMs);
      break;
    case Screen::Brake:
      renderBrake(c, state);
      break;
    case Screen::Safety:
      renderSafety(c, state);
      break;
    case Screen::Select:
      renderSelect(c, state);
      break;
  }
  renderStatus(c, state, nowMs);
}

}  // namespace meter
