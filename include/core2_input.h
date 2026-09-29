#pragma once

#include "meter.h"

namespace meter {

/** Translates Core2 touch and center-button events into meter navigation. */
class Core2Input {
 public:
  /** Updates navigation state; returns true when brightness changed. */
  bool update(State& state);

 private:
  /** Suppresses the short-press action after a long center-button press. */
  bool longPressHandled_ = false;
};

}  // namespace meter
