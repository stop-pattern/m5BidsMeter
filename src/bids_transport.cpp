#include "bids_transport.h"

#include <Arduino.h>

namespace meter {

void BidsTransport::begin(uint32_t nowMs) {
  Serial.begin(115200);
  Serial.println("TRV202");
  lastCorePollMs_ = nowMs;
  lastPanelPollMs_ = nowMs;
}

void BidsTransport::update(State& state, uint32_t nowMs) {
  readResponses(state, nowMs);
  if (uint32_t(nowMs - lastCorePollMs_) >= 100) {
    lastCorePollMs_ += 100;
    pollCore();
  }
  if (uint32_t(nowMs - lastPanelPollMs_) >= 200) {
    lastPanelPollMs_ += 200;
    pollPanel(state);
  }
}

void BidsTransport::readResponses(State& state, uint32_t nowMs) {
  while (Serial.available()) {
    const int character = Serial.read();
    if (character == '\n' || character == '\r') {
      if (lineLength_ != 0) {
        line_[lineLength_] = '\0';
        state.apply(line_, nowMs);
        lineLength_ = 0;
      }
    } else if (character >= 32 && character < 127) {
      if (lineLength_ + 1 < sizeof(line_)) {
        line_[lineLength_++] = char(character);
      } else {
        lineLength_ = 0;
      }
    }
  }
}

void BidsTransport::pollCore() {
  /** Core BIDS queries in their fixed request order. */
  static const char* const kCommands[] = {"TRIE1", "TRIE3", "TRIE4", "TRIH0", "TRIH1"};
  for (const char* command : kCommands) {
    Serial.println(command);
  }
}

void BidsTransport::pollPanel(const State& state) {
  /** Confirmed ATS-P/Sn lamp indices. */
  static const uint8_t kAtsLamps[] = {2, 3, 4, 6, 7};
  /** Confirmed CS-ATC lamp indices. */
  static const uint8_t kCsLamps[] = {1, 19, 22, 23, 29, 31, 73, 92, 96, 155, 175};

  if (state.screen == Screen::Safety) {
    if (state.safety == Safety::Ats) {
      for (uint8_t index : kAtsLamps) sendPanel(index);
    } else if (state.safety == Safety::Csatc) {
      for (uint8_t index : kCsLamps) sendPanel(index);
    }
  }
  if (state.screen == Screen::Speed && state.safety == Safety::Csatc) {
    sendPanel(101);
    sendPanel(102);
    for (uint8_t index = 104; index <= 125; ++index) sendPanel(index);
  }
}

void BidsTransport::sendPanel(uint8_t index) {
  Serial.print("TRIP");
  Serial.println(index);
}

}  // namespace meter
