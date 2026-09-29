#include "render_internal.h"

#include <math.h>
#include <stdio.h>

namespace meter {

/** Clamps a pointer position while leaving the numeric value unchanged. */
static int bound(int x, int low, int high) { return x < low ? low : x > high ? high : x; }

/** Draws the speed dial, actual numeric speed, and active ATC aspect. */
void renderSpeed(Canvas& c, const State& s) {
  const int cx = 160, cy = 154, r = 91;
  for (int i = 0; i <= 80; ++i) {
    const float a = (150.0f + 240.0f * i / 80.0f) * 3.14159265f / 180.0f;
    const int x1 = cx + int(cosf(a) * (r - ((i % 10) == 0 ? 13 : 6)));
    const int y1 = cy + int(sinf(a) * (r - ((i % 10) == 0 ? 13 : 6)));
    c.line(x1, y1, cx + int(cosf(a) * r), cy + int(sinf(a) * r), WHITE, (i % 10) == 0 ? 2 : 1);
    if (i % 10 == 0) {
      char label[8];
      snprintf(label, sizeof(label), "%d", i * 2);
      c.text(cx + int(cosf(a) * (r + 13)), cy + int(sinf(a) * (r + 13)) - 6, label, WHITE, 11,
             true);
    }
  }
  const float a = (150.0f + 240.0f * bound(int(s.speed), 0, 160) / 160.0f) * 3.14159265f / 180.0f;
  c.line(cx, cy, cx + int(cosf(a) * 73), cy + int(sinf(a) * 73), WHITE, 5);
  c.circle(cx, cy, 6, WHITE);
  if (s.safety != Safety::Ats) {
    const int signal = s.signalSpeed();
    const bool stop = signal <= 0;
    const float sa =
        (150.0f + 240.0f * bound(signal < 0 ? 0 : signal, 0, 160) / 160.0f) * 3.14159265f / 180.0f;
    const int tx = cx + int(cosf(sa) * (r + 4)), ty = cy + int(sinf(sa) * (r + 4));
    const Color signalColor = stop ? RED : GREEN;
    c.line(tx, ty - 6, tx - 6, ty + 5, signalColor, 2);
    c.line(tx - 6, ty + 5, tx + 6, ty + 5, signalColor, 2);
    c.line(tx + 6, ty + 5, tx, ty - 6, signalColor, 2);
    if (s.safety == Safety::Datc || signal < 0 || s.lamp(101)) c.text(162, 180, "×", RED, 20, true);
  }
  char value[32];
  snprintf(value, sizeof(value), "%.0f km/h", floorf(s.speed + 0.5f));
  c.rect(108, 186, 110, 24, BG);
  c.text(160, 188, value, WHITE, 19, true);
}

/** Draws one vertical pressure scale with an optional red 200 kPa tick. */
static void pressureGauge(Canvas& c, int x, int top, int bottom, float value, const char* label,
                          Color fill, bool warning, bool blink) {
  const int y0 = 50, y1 = 204, h = y1 - y0;
  c.text(x, 32, label, WHITE, 17, true);
  c.rect(x - 27, y0, 27, h, 0x141c2b);
  const float fraction = fmaxf(0.0f, fminf(1.0f, (value - top) / float(bottom - top)));
  const int fillH = int(fraction * h);
  c.rect(x - 26, y1 - fillH, 25, fillH, fill);
  outline(c, x - 27, y0, 27, h, EDGE);
  const int step = top == 0 ? 20 : 10;
  const int majorStep = top == 0 ? 200 : 100;
  for (int n = top; n <= bottom; n += step) {
    const int y = y1 - int((n - top) * h / float(bottom - top));
    const bool major = (n - top) % majorStep == 0;
    Color color = (warning && n == 200 && blink) ? RED : WHITE;
    c.line(x, y, x + (major ? 13 : 7), y, color, major ? 2 : 1);
    if (major) {
      char num[12];
      snprintf(num, sizeof(num), "%d", n);
      c.text(x + 18, y - 6, num, color, 10);
    }
  }
  const int marker = y1 - fillH;
  c.line(x - 34, marker, x + 3, marker, fill, 4);
  char v[24];
  snprintf(v, sizeof(v), "%.0f kPa", floorf(value + 0.5f));
  c.text(x + 8, 209, v, WHITE, 12, true);
}

/** Draws the BC and MR pressure gauges from the latest BIDS readings. */
void renderPressure(Canvas& c, const State& s, uint32_t nowMs) {
  pressureGauge(c, 94, 0, 800, s.bc, "BC", WHITE, s.rollingWarning(), s.warningRed(nowMs));
  pressureGauge(c, 231, 700, 1000, s.mr, "MR", ORANGE, false, false);
}

/** Draws brake notches, emergency status, and holding brake. */
void renderBrake(Canvas& c, const State& s) {
  c.text(160, 37, "制動ハンドル", MUTED, 13, true);
  c.rect(23, 57, 272, 28, s.brake == 9 ? 0x782d35 : PANEL);
  outline(c, 23, 57, 272, 28, s.brake == 9 ? RED : EDGE);
  c.text(160, 62,
         s.brake == 9   ? "非常"
         : s.brake == 0 ? "緩解"
                        : "常用ブレーキ",
         s.brake == 9 ? RED : WHITE, 17, true);
  if (s.holdingBrake()) {
    c.rect(100, 90, 120, 25, 0x245c3b);
    outline(c, 100, 90, 120, 25, GREEN);
    c.text(160, 94, "抑速", GREEN, 15, true);
  } else
    c.text(160, 94, "抑速", MUTED, 14, true);
  for (int i = 0; i < 8; ++i) {
    const int y = 191 - i * 9;
    c.rect(58, y, 205, 7, (s.brake >= i + 1 && s.brake <= 8) ? YELLOW : 0x374253);
  }
  char value[12];
  snprintf(value, sizeof(value), "B%d", s.brake);
  c.text(160, 201, s.brake == 9 ? "EB" : s.brake == 0 ? "B0" : value, WHITE, 17, true);
}

}  // namespace meter
