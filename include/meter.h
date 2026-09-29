#pragma once

#include <stdint.h>

namespace meter {

enum class Screen : uint8_t { Home, Speed, Pressure, Brake, Safety, Select };
enum class Safety : uint8_t { Ats, Datc, Csatc };

struct State {
  float speed = 0;
  float bc = 0;
  float mr = 0;
  int brake = 0;
  int power = 0;
  int panel[256] = {};
  bool panelValid[256] = {};
  bool received = false;
  uint32_t lastResponseMs = 0;
  Screen screen = Screen::Home;
  Safety safety = Safety::Ats;
  uint8_t brightness = 100;

  bool apply(const char* line, uint32_t nowMs);
  bool disconnected(uint32_t nowMs) const;
  bool rollingWarning() const;
  bool warningRed(uint32_t nowMs) const;
  bool holdingBrake() const;
  bool lamp(int index) const;
  int signalSpeed() const; // -1: no valid speed indication
  bool signalStop() const;
  void cycleBrightness();
};

// 0 means no verified Panel mapping. Special conditions are handled by lampOn.
int lampIndex(Safety safety, const char* label);
bool lampOn(const State& state, Safety safety, const char* label);

}  // namespace meter
