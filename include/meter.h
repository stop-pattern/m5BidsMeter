#pragma once

#include <stdint.h>

namespace meter {

/** User-selectable meter and safety configuration screens. */
enum class Screen : uint8_t { Home, Speed, Pressure, Brake, Safety, Select };
/** Safety systems represented by the lamp display. */
enum class Safety : uint8_t { Ats, Datc, Csatc };

/** Latest BIDS readings and local display settings. */
struct State {
  /** Vehicle speed in km/h. */
  float speed = 0;
  /** Brake-cylinder pressure in kPa. */
  float bc = 0;
  /** Main-reservoir pressure in kPa. */
  float mr = 0;
  /** Brake handle position, zero to nine. */
  int brake = 0;
  /** Power handle value; negative values mean holding brake. */
  int power = 0;
  /** Last numeric value received for each BIDS Panel index. */
  int panel[256] = {};
  /** Whether each Panel index has received a valid value. */
  bool panelValid[256] = {};
  /** Whether any valid response has been received. */
  bool received = false;
  /** Timestamp of the most recent valid BIDS response. */
  uint32_t lastResponseMs = 0;
  /** Screen currently selected by the user. */
  Screen screen = Screen::Home;
  /** Zero-based home menu page, retained when returning from a meter. */
  uint8_t homePage = 0;
  /** Safety system currently selected by the user. */
  Safety safety = Safety::Ats;
  /** Backlight brightness percentage. */
  uint8_t brightness = 100;

  /** Parses one complete response and updates readings; returns success. */
  bool apply(const char* line, uint32_t nowMs);
  /** Reports whether no valid reply has arrived in five seconds. */
  bool disconnected(uint32_t nowMs) const;
  /** Reports the standstill and low-BC rolling-warning condition. */
  bool rollingWarning() const;
  /** Returns the red half of the one-second rolling-warning phase. */
  bool warningRed(uint32_t nowMs) const;
  /** Reports holding brake when the power value is negative. */
  bool holdingBrake() const;
  /** Reports whether one Panel index has a nonzero value. */
  bool lamp(int index) const;
  /** Returns the CS-ATC signal limit, or -1 when no aspect is known. */
  int signalSpeed() const;
  /** Reports whether a CS-ATC stop aspect is active. */
  bool signalStop() const;
  /** Advances brightness through 25, 50, 75, and 100 percent. */
  void cycleBrightness();
};

/** Looks up a verified Panel index; returns -1 when none is assigned. */
int lampIndex(Safety safety, const char* label);
/** Applies the safety-system-specific lamp value rule. */
bool lampOn(const State& state, Safety safety, const char* label);

}  // namespace meter
