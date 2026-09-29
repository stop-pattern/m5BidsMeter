#include "render_internal.h"

#include <math.h>
#include <stdio.h>

namespace meter {

/** Clamps a pointer position while leaving the numeric value unchanged. */
static int bound(int x, int low, int high) { return x < low ? low : x > high ? high : x; }

/** Converts a displayed speed into the dial angle in radians. */
static float speedAngle(int speed) {
  return (150.0f + 240.0f * bound(speed, 0, 160) / 160.0f) * 3.14159265f / 180.0f;
}

/** Draws a filled signal arrow whose tip always points at the needle pivot. */
static void signalArrow(Canvas& canvas, int cx, int cy, int radius, int speed, Color color) {
  const float angle = speedAngle(speed);
  const float radialX = cosf(angle), radialY = sinf(angle);
  const float tangentX = -radialY, tangentY = radialX;
  const int tipX = cx + int(radialX * (radius + 2));
  const int tipY = cy + int(radialY * (radius + 2));
  for (int offset = -8; offset <= 8; ++offset) {
    const int baseX = cx + int(radialX * (radius + 18) + tangentX * offset);
    const int baseY = cy + int(radialY * (radius + 18) + tangentY * offset);
    canvas.line(tipX, tipY, baseX, baseY, color);
  }
}

/** Draws D-ATC's fixed dark arc from 0 through 120 km/h. */
static void datcBand(Canvas& canvas, int cx, int cy, int radius) {
  for (int speed = 0; speed <= 120; ++speed) {
    const float angle = speedAngle(speed);
    const int innerX = cx + int(cosf(angle) * (radius - 11));
    const int innerY = cy + int(sinf(angle) * (radius - 11));
    const int outerX = cx + int(cosf(angle) * (radius - 3));
    const int outerY = cy + int(sinf(angle) * (radius - 3));
    canvas.line(innerX, innerY, outerX, outerY, 0x111b28, 2);
  }
}

/** Draws the speed dial, actual numeric speed, and active ATC aspect. */
void renderSpeed(Canvas& c, const State& s) {
  const int cx = 160, cy = 154, r = 91;
  if (s.safety == Safety::Datc) datcBand(c, cx, cy, r);
  for (int i = 0; i <= 80; ++i) {
    const float a = speedAngle(i * 2);
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
  const float a = speedAngle(int(s.speed));
  c.line(cx, cy, cx + int(cosf(a) * 73), cy + int(sinf(a) * 73), WHITE, 5);
  c.circle(cx, cy, 6, WHITE);
  if (s.safety == Safety::Csatc) {
    /** Top two aspect lamps from the CS-ATC reference speedometer. */
    const bool proceed = s.signalSpeed() > 0;
    c.circle(130, 47, 7, proceed ? 0x49303a : RED);
    c.circle(190, 47, 7, proceed ? GREEN : 0x304734);
  }
  if (s.safety != Safety::Ats) {
    const int signal = s.safety == Safety::Datc ? s.datcLimitKmh : s.signalSpeed();
    const Color arrowColor = signal <= 0 ? RED : s.safety == Safety::Datc ? GREEN : ORANGE;
    signalArrow(c, cx, cy, r, signal < 0 ? 0 : signal, arrowColor);
    if (s.safety == Safety::Datc) {
      drawLampFace(c, 30, 213, 44, 20, false, LAMP_WHITE);
      c.text(52, 216, "入換", LAMP_UNLIT_TEXT, 12, true);
      drawLampFace(c, 143, 213, 34, 20, false, LAMP_RED);
      c.text(160, 214, "×", LAMP_UNLIT_TEXT, 16, true);
      drawLampFace(c, 95, 38, 130, 20, false, LAMP_ORANGE);
      c.text(160, 41, "パターン接近", LAMP_UNLIT_TEXT, 12, true);
    } else if (signal < 0 || s.lamp(101)) {
      c.text(162, 214, "×", RED, 20, true);
    }
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
