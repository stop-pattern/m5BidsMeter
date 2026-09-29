#pragma once

#include <stddef.h>
#include <stdint.h>

#include "meter.h"

namespace meter {

/** Exchanges BIDS requests and replies with the PC over USB serial. */
class BidsTransport {
 public:
  /** Starts the serial link and sends the protocol version request. */
  void begin(uint32_t nowMs);

  /** Applies received replies and sends any polling requests due at nowMs. */
  void update(State& state, uint32_t nowMs);

 private:
  /** ASCII response line being assembled. */
  char line_[80] = {};
  /** Number of response characters currently buffered. */
  size_t lineLength_ = 0;
  /** Scheduled start time of the last core poll. */
  uint32_t lastCorePollMs_ = 0;
  /** Scheduled start time of the last Panel poll. */
  uint32_t lastPanelPollMs_ = 0;

  /** Reads complete response lines without blocking the UI loop. */
  void readResponses(State& state, uint32_t nowMs);
  /** Requests speed, pressure, brake, and power values. */
  void pollCore();
  /** Requests lamps or signal aspects needed by the active screen. */
  void pollPanel(const State& state);
  /** Sends one Panel request by index. */
  void sendPanel(uint8_t index);
};

}  // namespace meter
