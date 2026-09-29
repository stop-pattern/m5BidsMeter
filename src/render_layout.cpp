#include "render_internal.h"

#include <stdio.h>

namespace meter {

/** One destination and its title in the home menu. */
struct HomeEntry {
  /** Destination screen selected by touching the row. */
  Screen screen;
  /** Japanese title centered on the row button. */
  const char* title;
};

/** Home destinations in display order; adding entries enables further pages. */
static const HomeEntry kHomeEntries[] = {{Screen::Speed, "速度計"},
                                         {Screen::Pressure, "圧力計"},
                                         {Screen::Brake, "ブレーキ段数"},
                                         {Screen::Safety, "保安装置"}};
/** Number of home rows shown at once. */
static constexpr int kHomeRows = 4;
/** First home row's top coordinate. */
static constexpr int kHomeTop = 35;
/** Vertical pitch between home rows. */
static constexpr int kHomePitch = 43;
/** Number of configured home destinations. */
static constexpr int kHomeCount = sizeof(kHomeEntries) / sizeof(kHomeEntries[0]);

/** Returns the number of menu pages needed by configured destinations. */
static int homePageCount() { return (kHomeCount + kHomeRows - 1) / kHomeRows; }

/** Draws a one-pixel rectangular frame. */
void outline(Canvas& c, int x, int y, int w, int h, Color color) {
  c.line(x, y, x + w - 1, y, color);
  c.line(x, y, x, y + h - 1, color);
  c.line(x + w - 1, y, x + w - 1, y + h - 1, color);
  c.line(x, y + h - 1, x + w - 1, y + h - 1, color);
}

/** Builds the blue-gray bevel and soft corners seen in the reference buttons. */
static void beveledButton(Canvas& c, int x, int y, int w, int h) {
  /** Face color reused to expose empty space inside white icon outlines. */
  constexpr Color kFace = 0x61799e;
  c.rect(x + 3, y, w - 6, h, 0x344963);
  c.rect(x, y + 3, w, h - 6, 0x344963);
  c.rect(x + 3, y + 3, w - 6, h - 6, kFace);
  c.line(x + 3, y + 1, x + w - 4, y + 1, 0xc2cee0, 2);
  c.line(x + 1, y + 3, x + 1, y + h - 5, 0xaabbd3, 2);
  c.line(x + 3, y + h - 2, x + w - 4, y + h - 2, 0x22344f, 2);
  c.line(x + w - 2, y + 4, x + w - 2, y + h - 4, 0x2b405f, 2);
}

/** Draws two overlapping white screens matching img/display.png. */
static void screenIcon(Canvas& c) {
  beveledButton(c, 4, 3, 27, 21);
  c.rect(16, 10, 9, 9, WHITE);
  c.rect(17, 11, 7, 7, 0x61799e);
  c.rect(9, 7, 12, 11, WHITE);
  c.rect(11, 9, 8, 7, 0x61799e);
}

/** Draws a white sun with eight rays matching img/bright.png. */
static void lightIcon(Canvas& c) {
  beveledButton(c, 287, 213, 29, 23);
  c.circle(301, 224, 5, WHITE);
  c.line(301, 216, 301, 219, WHITE);
  c.line(301, 229, 301, 232, WHITE);
  c.line(292, 224, 296, 224, WHITE);
  c.line(306, 224, 310, 224, WHITE);
  c.line(294, 217, 297, 220, WHITE);
  c.line(305, 228, 308, 231, WHITE);
  c.line(305, 220, 308, 217, WHITE);
  c.line(294, 231, 297, 228, WHITE);
}

/** Draws the background and the common header/navigation controls. */
void renderCommon(Canvas& c, const State& s) {
  c.rect(0, 0, 320, 240, BG);
  c.rect(0, 0, 320, 27, 0x172235);
  if (s.screen != Screen::Home) screenIcon(c);
  /** Title of the currently selected screen. */
  const char* title = s.screen == Screen::Home       ? "TIMS メーター"
                      : s.screen == Screen::Speed    ? "速度計"
                      : s.screen == Screen::Pressure ? "BC / MR 圧力計"
                      : s.screen == Screen::Brake    ? "ブレーキ段数"
                      : s.screen == Screen::Safety   ? "保安装置"
                                                     : "保安装置選択";
  c.text(162, 5, title, WHITE, 14, true);
  lightIcon(c);
}

/** Marks the communication status only after the reply timeout. */
void renderStatus(Canvas& c, const State& s, uint32_t nowMs) {
  if (s.disconnected(nowMs)) {
    c.rect(247, 3, 66, 21, 0x4b2530);
    outline(c, 247, 3, 66, 21, RED);
    c.text(280, 5, "通信断", WHITE, 16, true);
  }
}

/** Draws one page of vertically stacked home buttons. */
void renderHome(Canvas& c, const State& s) {
  for (int row = 0; row < kHomeRows; ++row) {
    const int index = int(s.homePage) * kHomeRows + row;
    if (index >= kHomeCount) break;
    const int y = kHomeTop + row * kHomePitch;
    beveledButton(c, 28, y, 260, 39);
    c.text(158, y + 9, kHomeEntries[index].title, WHITE, 20, true);
  }
  /** Name of the selected safety system shown on the home screen. */
  const char* safety = s.safety == Safety::Ats    ? "ATS-P / Sn"
                       : s.safety == Safety::Datc ? "D-ATC"
                                                  : "CS-ATC / ATC-10";
  c.text(160, 216, safety, MUTED, 12, true);
  if (homePageCount() > 1) {
    if (s.homePage > 0) {
      beveledButton(c, 24, 210, 26, 25);
      c.line(41, 217, 34, 222, WHITE, 2);
      c.line(34, 222, 41, 227, WHITE, 2);
    }
    if (s.homePage + 1 < homePageCount()) {
      beveledButton(c, 245, 210, 26, 25);
      c.line(253, 217, 260, 222, WHITE, 2);
      c.line(260, 222, 253, 227, WHITE, 2);
    }
  }
}

/** Draws the three selectable safety-system cards. */
void renderSelect(Canvas& c, const State& s) {
  /** Safety-system names ordered by selection row. */
  static const char* names[] = {"ATS-P / Sn  E233-0・3000", "D-ATC  E233-1000",
                                "CS-ATC / ATC-10  E233-2000"};
  for (int i = 0; i < 3; ++i) {
    int y = 41 + i * 55;
    c.rect(12, y, 296, 46, PANEL);
    outline(c, 12, y, 296, 46, EDGE);
    c.text(160, y + 13, names[i], WHITE, 14, true);
  }
  (void)s;
}

/** Converts a touch point to one screen or brightness action. */
void tap(State& s, int x, int y) {
  if (x >= 287 && y >= 211) {
    s.cycleBrightness();
    return;
  }
  if (s.screen != Screen::Home && x < 70 && y < 27) {
    s.screen = Screen::Home;
    return;
  }
  if (s.screen == Screen::Home) {
    if (homePageCount() > 1 && y >= 210 && y < 236) {
      if (x >= 24 && x < 50 && s.homePage > 0) --s.homePage;
      if (x >= 245 && x < 271 && s.homePage + 1 < homePageCount()) ++s.homePage;
      return;
    }
    if (x >= 28 && x < 288 && y >= kHomeTop) {
      const int row = (y - kHomeTop) / kHomePitch;
      const int index = int(s.homePage) * kHomeRows + row;
      if (row < kHomeRows && y < kHomeTop + row * kHomePitch + 39 && index < kHomeCount)
        s.screen = kHomeEntries[index].screen;
    }
  } else if (s.screen == Screen::Select && y >= 41 && y < 197) {
    int row = (y - 41) / 55;
    if (row >= 0 && row < 3) {
      s.safety = Safety(row);
      s.screen = Screen::Home;
    }
  }
}

}  // namespace meter
