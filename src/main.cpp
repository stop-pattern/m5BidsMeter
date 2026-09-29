#include <Arduino.h>
#include <M5Unified.h>

#include "bids_transport.h"
#include "core2_display.h"
#include "core2_input.h"
#include "meter.h"

namespace {

/** Current BIDS readings and selected display settings. */
meter::State state;
/** USB serial parser and request scheduler. */
meter::BidsTransport transport;
/** Core2 touch and center-button navigation. */
meter::Core2Input input;
/** Buffered screen renderer and changed-area LCD transfer. */
meter::Core2Display display;

}  // namespace

/** Initializes the Core2 hardware, screen, and BIDS transport. */
void setup() {
  auto config = M5.config();
  M5.begin(config);
  M5.Speaker.begin();
  M5.Speaker.setVolume(64);
  display.begin();
  display.setBrightness(state.brightness);
  transport.begin(millis());
}

/** Runs nonblocking input, serial polling, and display refresh. */
void loop() {
  M5.update();
  const uint32_t nowMs = millis();
  transport.update(state, nowMs);
  if (input.update(state)) display.setBrightness(state.brightness);
  display.update(state, nowMs);
  delay(2);
}
