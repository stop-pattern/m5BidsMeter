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

/** Draws one solid navigation or brightness button. */
static void button(Canvas& c, int x, int y, int w, int h, const char* label) {
  c.rect(x, y, w, h, 0x344e71);
  outline(c, x, y, w, h, 0xa3bad4);
  c.text(x + w / 2, y + (h - 13) / 2, label, WHITE, 12, true);
}

/** Draws the background and the common header/navigation controls. */
void renderCommon(Canvas& c, const State& s) {
  c.rect(0, 0, 320, 240, BG);
  c.rect(0, 0, 320, 27, 0x172235);
  if (s.screen != Screen::Home) button(c, 4, 3, 64, 21, "画面変更");
  /** Title of the currently selected screen. */
  const char* title = s.screen == Screen::Home       ? "TIMS メーター"
                      : s.screen == Screen::Speed    ? "速度計"
                      : s.screen == Screen::Pressure ? "BC / MR 圧力計"
                      : s.screen == Screen::Brake    ? "ブレーキ段数"
                      : s.screen == Screen::Safety   ? "保安装置"
                                                     : "保安装置選択";
  c.text(162, 5, title, WHITE, 14, true);
  button(c, 287, 213, 29, 23, "明");
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
