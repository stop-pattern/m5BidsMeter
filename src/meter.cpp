#include "meter.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

namespace meter {

static bool parseNumber(const char* text, double& out) {
  if (!text || !*text) return false;
  char* end = nullptr;
  out = strtod(text, &end);
  return end != text && *end == '\0' && isfinite(out);
}

bool State::apply(const char* line, uint32_t nowMs) {
  if (!line || strncmp(line, "TR", 2) != 0) return false;
  const char* x = strchr(line, 'X');
  if (!x || x == line + 2) return false;
  double value = 0;
  if (!parseNumber(x + 1, value)) return false;
  const size_t keyLen = size_t(x - line);
  auto keyIs = [&](const char* key) {
    return strlen(key) == keyLen && strncmp(line, key, keyLen) == 0;
  };
  if (keyIs("TRIE1"))
    speed = float(value);
  else if (keyIs("TRIE3"))
    bc = float(value);
  else if (keyIs("TRIE4"))
    mr = float(value);
  else if (keyIs("TRIH0"))
    brake = int(value);
  else if (keyIs("TRIH1"))
    power = int(value);
  else if (keyLen > 4 && strncmp(line, "TRIP", 4) == 0) {
    char indexText[8] = {};
    if (keyLen - 4 >= sizeof(indexText)) return false;
    memcpy(indexText, line + 4, keyLen - 4);
    char* end = nullptr;
    const long index = strtol(indexText, &end, 10);
    if (*end || index < 0 || index > 255 || value < -2147483647.0 || value > 2147483647.0 ||
        floor(value) != value)
      return false;
    panel[index] = int(value);
    panelValid[index] = true;
  } else if (!keyIs("TRV202"))
    return false;
  received = true;
  lastResponseMs = nowMs;
  return true;
}

bool State::disconnected(uint32_t nowMs) const { return uint32_t(nowMs - lastResponseMs) >= 5000; }
bool State::rollingWarning() const { return speed == 0 && bc < 200; }
bool State::warningRed(uint32_t nowMs) const { return rollingWarning() && nowMs % 1000 < 500; }
bool State::holdingBrake() const { return power < 0; }
bool State::lamp(int index) const {
  return index >= 0 && index < 256 && panelValid[index] && panel[index] != 0;
}

int State::signalSpeed() const {
  if (safety != Safety::Csatc) return -1;
  if (signalStop()) return 0;
  int result = -1;
  for (int i = 104; i <= 124; ++i) {
    if (lamp(i)) {
      const int speed = 10 + (i - 104) * 5;
      if (result < 0 || speed < result) result = speed;
    }
  }
  if (lamp(125) && result < 0) result = 120;
  return result;
}

bool State::signalStop() const { return safety == Safety::Csatc && (lamp(101) || lamp(102)); }
void State::cycleBrightness() { brightness = brightness == 100 ? 25 : uint8_t(brightness + 25); }

int lampIndex(Safety safety, const char* label) {
  if (safety == Safety::Ats) {
    if (strcmp(label, "P電源") == 0) return 2;
    if (strcmp(label, "パターン接近") == 0) return 3;
    if (strcmp(label, "ブレーキ開放") == 0) return 4;
    if (strcmp(label, "ATS-P") == 0) return 6;
    if (strcmp(label, "故障") == 0) return 7;
  }
  if (safety == Safety::Csatc) {
    struct Mapping {
      const char* label;
      int index;
    };
    static const Mapping map[] = {
        {"ATC", 19},  {"ATC非常", 22},     {"ATC常用", 23},       {"非設", 29},
        {"構内", 31}, {"過電流", 96},      {"対雪ブレーキ", 175}, {"ATS動作", 1},
        {"TASC", 73}, {"ホームドア", 155}, {"地下鉄", 92},        {"JR", 92},
    };
    for (const auto& item : map)
      if (strcmp(label, item.label) == 0) return item.index;
  }
  return -1;
}

bool lampOn(const State& state, Safety safety, const char* label) {
  const int index = lampIndex(safety, label);
  if (index < 0 || !state.panelValid[index]) return false;
  if (safety == Safety::Csatc) {
    if (strcmp(label, "地下鉄") == 0) return state.panel[index] == 1;
    if (strcmp(label, "JR") == 0) return state.panel[index] == 6;
    if (strcmp(label, "TASC") == 0 || strcmp(label, "ホームドア") == 0)
      return state.panel[index] == 1;
  }
  return state.panel[index] != 0;
}

}  // namespace meter
