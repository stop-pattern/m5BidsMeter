#include "frame_diff.h"
#include "meter.h"
#include "render.h"
#include "render_internal.h"

#include <assert.h>
#include <math.h>
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
    if (color == meter::LAMP_GREEN) greenLamp = true;
    if (color == meter::LAMP_ORANGE) orangeLamp = true;
    if (color == meter::LAMP_RED) redLamp = true;
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

/** Captures home menu label positions and communication warning font. */
class HomeCanvas : public meter::Canvas {
 public:
  /** Number of visible home destination labels. */
  int count = 0;
  /** Horizontal coordinate shared by the four menu labels. */
  int x[4] = {};
  /** Vertical coordinates of the four menu labels. */
  int y[4] = {};
  /** Requested font size for the communication warning. */
  int warningSize = 0;
  /** True when the warning uses the same solid red as an illuminated lamp. */
  bool redWarningFace = false;
  /** True when the warning label uses dark text on its lit face. */
  bool darkWarningText = false;
  /** Count of bevel face rectangles in the safety selection screen. */
  int selectionFaces = 0;
  /** White horizontal fill strokes inside the speaker icon. */
  int speakerFill = 0;

  /** Records the red warning and wide beveled selection faces. */
  void rect(int, int, int width, int height, meter::Color color) override {
    if (color == meter::LAMP_RED && width >= 60 && height >= 18) redWarningFace = true;
    if (color == 0x61799e && width >= 280 && height >= 35) ++selectionFaces;
  }
  /** Counts the white fill inside the speaker without counting its outline. */
  void line(int x1, int y1, int x2, int y2, meter::Color color, int = 1) override {
    if (color == meter::WHITE && x1 >= 264 && x2 <= 270 && y1 == y2 && y1 >= 218 && y1 <= 229)
      speakerFill += x2 - x1 + 1;
  }
  /** Records destination labels and warning font size. */
  void text(int left, int top, const char* value, meter::Color color, int size,
            bool = false) override {
    if (strcmp(value, "通信断") == 0) {
      warningSize = size;
      darkWarningText = color == meter::LAMP_TEXT;
    }
    if (strcmp(value, "速度計") == 0 || strcmp(value, "圧力計") == 0 ||
        strcmp(value, "ブレーキ段数") == 0 || strcmp(value, "保安装置") == 0) {
      if (count < 4) {
        x[count] = left;
        y[count] = top;
      }
      ++count;
    }
  }
  /** Circles are irrelevant to label geometry. */
  void circle(int, int, int, meter::Color) override {}
};

/** Records CS-ATC's red and green aspect lamps above the dial. */
class SpeedCanvas : public meter::Canvas {
 public:
  /** Number of bright red aspect circles. */
  int redAspects = 0;
  /** Number of bright green aspect circles. */
  int greenAspects = 0;
  /** Closest radial coordinate reached by a green signal arrow. */
  int nearestArrowX = -1;
  /** Closest radial coordinate reached by a green signal arrow. */
  int nearestArrowY = -1;
  /** Squared radius of the nearest green arrow point. */
  int nearestRadius = 1000000;
  /** Orange CS-ATC arrow strokes. */
  int orangeArrowLines = 0;
  /** Red stop arrow strokes. */
  int redArrowLines = 0;
  /** Dark D-ATC band strokes. */
  int bandLines = 0;
  /** Number of D-ATC-only lamp labels drawn. */
  int datcLabels = 0;

  /** Rectangles are irrelevant to aspect circles. */
  void rect(int, int, int, int, meter::Color) override {}
  /** Tracks the inward tip of a green signal arrow. */
  void line(int x1, int y1, int x2, int y2, meter::Color color, int = 1) override {
    if (color == meter::ORANGE) ++orangeArrowLines;
    if (color == meter::RED) ++redArrowLines;
    if (color == 0x111b28) ++bandLines;
    if (color != meter::GREEN && color != meter::ORANGE) return;
    const int xs[] = {x1, x2};
    const int ys[] = {y1, y2};
    for (int endpoint = 0; endpoint < 2; ++endpoint) {
      const int dx = xs[endpoint] - 160;
      const int dy = ys[endpoint] - 154;
      const int radius = dx * dx + dy * dy;
      if (radius < nearestRadius) {
        nearestRadius = radius;
        nearestArrowX = xs[endpoint];
        nearestArrowY = ys[endpoint];
      }
    }
  }
  /** Counts D-ATC's three unlit speed-screen labels. */
  void text(int, int, const char* value, meter::Color, int, bool = false) override {
    if (strcmp(value, "入換") == 0 || strcmp(value, "パターン接近") == 0 || strcmp(value, "×") == 0)
      ++datcLabels;
  }
  /** Counts only bright colored circles above the speed dial. */
  void circle(int, int y, int, meter::Color color) override {
    if (y >= 65) return;
    if (color == meter::RED) ++redAspects;
    if (color == meter::GREEN) ++greenAspects;
  }
};

int main() {
  meter::State home;
  HomeCanvas homeCanvas;
  meter::render(homeCanvas, home, 5000);
  if (homeCanvas.count != 4 || homeCanvas.warningSize < 16) return 10;
  if (!homeCanvas.redWarningFace || !homeCanvas.darkWarningText) return 13;
  home.soundPercent = 100;
  HomeCanvas fullVolumeCanvas;
  meter::render(fullVolumeCanvas, home, 5000);
  home.soundPercent = 0;
  HomeCanvas mutedCanvas;
  meter::render(mutedCanvas, home, 5000);
  if (!(fullVolumeCanvas.speakerFill > homeCanvas.speakerFill &&
        homeCanvas.speakerFill > mutedCanvas.speakerFill))
    return 20;
  home.soundPercent = 50;
  for (int index = 1; index < 4; ++index)
    if (homeCanvas.x[index] != homeCanvas.x[0] || homeCanvas.y[index] <= homeCanvas.y[index - 1])
      return 11;
  const meter::Screen destinations[] = {meter::Screen::Speed, meter::Screen::Pressure,
                                        meter::Screen::Brake, meter::Screen::Safety};
  for (int index = 0; index < 4; ++index) {
    home.screen = meter::Screen::Home;
    meter::tap(home, 100, 54 + index * 43);
    if (home.screen != destinations[index]) return 12;
  }
  home.screen = meter::Screen::Select;
  HomeCanvas selectCanvas;
  meter::render(selectCanvas, home, 0);
  if (selectCanvas.selectionFaces != 3) return 14;
  if (home.soundPercent != 50) return 15;
  home.screen = meter::Screen::Home;
  if (!meter::tap(home, 269, 223) || home.soundPercent != 0) return 16;
  if (!meter::tap(home, 269, 223) || home.soundPercent != 100) return 17;
  if (!meter::tap(home, 269, 223) || home.soundPercent != 50) return 19;
  home.screen = meter::Screen::Safety;
  if (meter::tap(home, 269, 223) || home.soundPercent != 50) return 18;

  meter::State aspectState;
  aspectState.safety = meter::Safety::Csatc;
  aspectState.panelValid[104] = true;
  aspectState.panel[104] = 1;
  SpeedCanvas proceedAspect;
  meter::renderSpeed(proceedAspect, aspectState);
  if (proceedAspect.greenAspects != 1 || proceedAspect.redAspects != 0 ||
      proceedAspect.orangeArrowLines != 17 || proceedAspect.redArrowLines != 0)
    return 21;
  aspectState.panel[104] = 0;
  aspectState.panelValid[105] = true;
  aspectState.panel[105] = 1;
  SpeedCanvas proceed15;
  meter::renderSpeed(proceed15, aspectState);
  if (aspectState.signalSpeed() != 15 || (proceedAspect.nearestArrowX == proceed15.nearestArrowX &&
                                          proceedAspect.nearestArrowY == proceed15.nearestArrowY))
    return 26;
  aspectState.panelValid[101] = true;
  aspectState.panel[101] = 1;
  SpeedCanvas stopAspect;
  meter::renderSpeed(stopAspect, aspectState);
  if (stopAspect.redAspects != 1 || stopAspect.greenAspects != 0 ||
      stopAspect.redArrowLines != 17 || stopAspect.orangeArrowLines != 0)
    return 22;

  meter::State datcState;
  datcState.safety = meter::Safety::Datc;
  datcState.datcLimitKmh = 91;
  SpeedCanvas datc91;
  meter::renderSpeed(datc91, datcState);
  datcState.datcLimitKmh = 92;
  SpeedCanvas datc92;
  meter::renderSpeed(datc92, datcState);
  if (datc91.nearestArrowX < 0 || datc92.nearestArrowX < 0 ||
      (datc91.nearestArrowX == datc92.nearestArrowX &&
       datc91.nearestArrowY == datc92.nearestArrowY))
    return 23;
  datcState.datcLimitKmh = -1;
  SpeedCanvas datcUnknown;
  meter::renderSpeed(datcUnknown, datcState);
  if (datc91.bandLines != datc92.bandLines || datc91.bandLines != datcUnknown.bandLines ||
      datcUnknown.bandLines != 121 || datc91.datcLabels != 3)
    return 24;
  meter::State atsSpeed;
  SpeedCanvas atsBase;
  meter::renderSpeed(atsBase, atsSpeed);
  if (atsBase.bandLines || atsBase.redAspects || atsBase.greenAspects || atsBase.datcLabels ||
      atsBase.orangeArrowLines || atsBase.redArrowLines)
    return 25;

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
