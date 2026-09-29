#include "frame_diff.h"
#include "meter.h"
#include "render.h"
#include "render_internal.h"

#include <assert.h>
#include <string.h>

/** Captures text and lines used to draw one vertical safety label. */
class LabelCanvas : public meter::Canvas {
 public:
  /** True when the horizontal long-sound glyph is drawn unchanged. */
  bool horizontalLongSound = false;
  /** True when a vertical stroke replaces the long-sound glyph. */
  bool verticalLongSound = false;
  /** True when the last glyph of a long label is still visible. */
  bool finalGlyph = false;
  /** Leftmost glyph coordinate; verifies multiple columns are used. */
  int leftmostGlyph = 999;
  /** Rightmost glyph coordinate; verifies multiple columns are used. */
  int rightmostGlyph = -1;
  /** Smallest requested font size for a glyph. */
  int smallestFont = 999;
  /** True when a lit lamp uses the green face color. */
  bool greenLamp = false;
  /** True when a lit lamp uses the orange face color. */
  bool orangeLamp = false;
  /** True when a lit lamp uses the red face color. */
  bool redLamp = false;

  /** Records colors applied to lit lamp faces. */
  void rect(int, int, int, int, meter::Color color) override {
    if (color == meter::GREEN) greenLamp = true;
    if (color == meter::ORANGE) orangeLamp = true;
    if (color == meter::RED) redLamp = true;
  }
  /** Records the vertical long-sound mark. */
  void line(int x1, int y1, int x2, int y2, meter::Color, int = 1) override {
    if (x1 == x2 && y2 > y1) verticalLongSound = true;
  }
  /** Records glyph placement and font size. */
  void text(int x, int, const char* glyph, meter::Color, int size, bool = false) override {
    if (strcmp(glyph, "ー") == 0) horizontalLongSound = true;
    if (strcmp(glyph, "キ") == 0) finalGlyph = true;
    if (x < leftmostGlyph) leftmostGlyph = x;
    if (x > rightmostGlyph) rightmostGlyph = x;
    if (size < smallestFont) smallestFont = size;
  }
  /** Circles are not used for a label alone. */
  void circle(int, int, int, meter::Color) override {}
};

int main() {
  /** Two tiny frames, split into two four-byte bands. */
  const uint8_t previousFrame[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  uint8_t nextFrame[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  bool changedBands[2] = {};
  meter::markChangedBands(nextFrame, previousFrame, 4, 2, changedBands);
  assert(!changedBands[0] && !changedBands[1]);
  nextFrame[5] = 1;
  meter::markChangedBands(nextFrame, previousFrame, 4, 2, changedBands);
  assert(!changedBands[0] && changedBands[1]);

  LabelCanvas label;
  meter::drawVerticalLabel(label, 0, 0, 32, 55, "対雪ブレーキ", meter::WHITE);
  assert(!label.horizontalLongSound && label.verticalLongSound);
  assert(label.finalGlyph && label.leftmostGlyph < label.rightmostGlyph);
  assert(label.smallestFont >= 12);

  meter::State lampState;
  lampState.safety = meter::Safety::Ats;
  lampState.panelValid[2] = lampState.panelValid[3] = lampState.panelValid[7] = true;
  lampState.panel[2] = lampState.panel[3] = lampState.panel[7] = 1;
  LabelCanvas lampColors;
  meter::renderSafety(lampColors, lampState);
  assert(lampColors.greenLamp && lampColors.orangeLamp && lampColors.redLamp);

  meter::State s;
  assert(s.rollingWarning());
  assert(s.warningRed(0) && !s.warningRed(500));
  assert(!s.disconnected(4999) && s.disconnected(5000));
  assert(s.apply("TRV202X100", 100));
  assert(s.apply("TRIE1X12.5", 101) && s.speed == 12.5f);
  assert(s.apply("TRIE3X199", 102) && s.bc == 199);
  assert(!s.rollingWarning());
  assert(s.apply("TRIE1X0", 103) && s.rollingWarning());
  assert(s.apply("TRIE3X200", 104) && !s.rollingWarning());
  assert(s.apply("TRIH0X9", 105) && s.brake == 9);
  assert(s.apply("TRIH1X-1", 106) && s.holdingBrake());
  assert(!s.apply("TRIE1Xabc", 107));
  assert(!s.apply("TRIP256X1", 107));
  assert(!s.apply("TRIP1X1.5", 107));
  assert(s.apply("TRIP2X1", 108) && meter::lampOn(s, meter::Safety::Ats, "P電源"));
  assert(!meter::lampOn(s, meter::Safety::Datc, "ATC"));
  s.safety = meter::Safety::Csatc;
  assert(s.signalSpeed() == -1);
  assert(s.apply("TRIP125X1", 110) && s.signalSpeed() == 120);
  assert(s.apply("TRIP110X1", 111) && s.signalSpeed() == 40);
  assert(s.apply("TRIP102X1", 112) && s.signalSpeed() == 0 && s.signalStop());
  assert(s.apply("TRIP92X6", 113) && meter::lampOn(s, s.safety, "JR"));
  assert(!meter::lampOn(s, s.safety, "地下鉄"));
  s.cycleBrightness();
  assert(s.brightness == 25);
  s.cycleBrightness();
  assert(s.brightness == 50);
  s.screen = meter::Screen::Home;
  meter::tap(s, 30, 60);
  assert(s.screen == meter::Screen::Speed);
  meter::tap(s, 10, 10);
  assert(s.screen == meter::Screen::Home);
  s.screen = meter::Screen::Select;
  meter::tap(s, 100, 110);
  assert(s.safety == meter::Safety::Datc && s.screen == meter::Screen::Home);
  meter::tap(s, 300, 220);
  assert(s.brightness == 75);
  return 0;
}
