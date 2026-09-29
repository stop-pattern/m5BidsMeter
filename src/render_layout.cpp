#include "render_internal.h"

#include <stdio.h>

namespace meter {

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
  if (s.disconnected(nowMs)) c.text(280, 5, "通信断", RED, 11, true);
}

/** Draws four home-screen destination cards. */
void renderHome(Canvas& c, const State& s) {
  /** Home-screen labels ordered left to right, top to bottom. */
  static const char* names[] = {"速度計", "圧力計", "ブレーキ段数", "保安装置"};
  for (int i = 0; i < 4; ++i) {
    const int x = 20 + (i % 2) * 150, y = 47 + (i / 2) * 76;
    c.rect(x, y, 130, 62, PANEL);
    outline(c, x, y, 130, 62, EDGE);
    c.rect(x + 4, y + 4, 5, 54, i == 0 ? GREEN : i == 1 ? ORANGE : i == 2 ? YELLOW : 0x8aabff);
    c.text(x + 70, y + 20, names[i], WHITE, 16, true);
  }
  /** Name of the selected safety system shown on the home screen. */
  const char* safety = s.safety == Safety::Ats    ? "ATS-P / Sn"
                       : s.safety == Safety::Datc ? "D-ATC"
                                                  : "CS-ATC / ATC-10";
  c.text(160, 207, safety, MUTED, 12, true);
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
    if (x >= 20 && x < 300 && y >= 47 && y < 185) {
      int col = x >= 170 ? 1 : 0, row = y >= 123 ? 1 : 0;
      s.screen = Screen(1 + row * 2 + col);
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
