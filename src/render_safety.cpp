#include "render_internal.h"

#include <string.h>

namespace meter {

/** Fits Japanese glyphs into right-to-left vertical columns in one lamp. */
void drawVerticalLabel(Canvas& canvas, int x, int y, int width, int height, const char* label,
                       Color color) {
  /** Height of one Gothic glyph and its vertical writing cell. */
  constexpr int kGlyphSize = 12;
  /** UTF-8 glyphs in the label, including a terminating byte per glyph. */
  char glyphs[20][5] = {};
  /** Number of decoded glyphs in the label. */
  int glyphCount = 0;

  const auto* cursor = reinterpret_cast<const unsigned char*>(label);
  while (*cursor && glyphCount < 20) {
    const int byteCount = (*cursor < 0x80) ? 1 : (*cursor < 0xE0) ? 2 : (*cursor < 0xF0) ? 3 : 4;
    for (int byte = 0; byte < byteCount && cursor[byte]; ++byte)
      glyphs[glyphCount][byte] = char(cursor[byte]);
    cursor += byteCount;
    ++glyphCount;
  }
  if (glyphCount == 0) return;

  /** Maximum number of complete glyph cells in one vertical column. */
  const int rowsPerColumn = (height - 4) / kGlyphSize;
  if (rowsPerColumn <= 0) return;
  /** Number of right-to-left columns required to fit the label. */
  const int columns = (glyphCount + rowsPerColumn - 1) / rowsPerColumn;
  /** Horizontal width allotted to each vertical column. */
  const int columnWidth = width / columns;

  for (int glyph = 0; glyph < glyphCount; ++glyph) {
    const int column = glyph / rowsPerColumn;
    const int row = glyph % rowsPerColumn;
    const int centerX = x + width - column * columnWidth - columnWidth / 2;
    const int topY = y + 2 + row * kGlyphSize;
    if (strcmp(glyphs[glyph], "ー") == 0) {
      canvas.line(centerX, topY + 1, centerX, topY + kGlyphSize - 2, color, 2);
    } else {
      canvas.text(centerX, topY, glyphs[glyph], color, kGlyphSize, true);
    }
  }
}

/** Draws a complete row of lamp frames and any known lit states. */
static void lampRow(Canvas& c, const State& s, Safety safety, const char* const* labels, int count,
                    int y, int h) {
  const int margin = 8, gap = 2, w = (320 - 2 * margin - gap * (count - 1)) / count;
  for (int i = 0; i < count; ++i) {
    const int x = margin + i * (w + gap);
    if (!labels[i]) continue;
    const bool lit = lampOn(s, safety, labels[i]);
    const Color color = lit ? GREEN : MUTED;
    c.rect(x, y, w, h, lit ? 0x407246 : 0x111827);
    outline(c, x, y, w, h, lit ? GREEN : 0x445065);
    if (*labels[i]) drawVerticalLabel(c, x, y, w, h, labels[i], color);
  }
}

/** Draws lamp frames for the selected ATS, D-ATC, or CS-ATC layout. */
void renderSafety(Canvas& c, const State& s) {
  if (s.safety == Safety::Ats) {
    /** ATS-P/Sn top-row labels in physical order. */
    static const char* top[] = {"P電源",        "パターン接近", "常用ブレーキ",
                                "非常ブレーキ", "ブレーキ開放", "ATS-P",
                                "故障",         "ATS電源",      "ATS動作"};
    /** ATS-P/Sn lower-row labels in physical order. */
    static const char* bottom[] = {"",         "三相", "非常短絡",    "耐雪ブレーキ",
                                   "直通予備", "定速", "駐車ブレーキ"};
    lampRow(c, s, s.safety, top, 9, 38, 77);
    lampRow(c, s, s.safety, bottom, 7, 125, 77);
  } else if (s.safety == Safety::Datc) {
    /** D-ATC upper-row labels. */
    static const char* top[] = {"",         "三相", "非常短絡",    "耐雪ブレーキ",
                                "直通予備", "定速", "駐車ブレーキ"};
    /** D-ATC middle-row labels. */
    static const char* middle[] = {"デジタルATC", "ATC",          "切",
                                   "ATS電源",     "パターン低減", "非常運転"};
    /** D-ATC lower-row labels. */
    static const char* bottom[] = {"ATC常用", "ATC非常", "停通防止動作",
                                   "ATS動作", "ATC電源", "ATC開放"};
    lampRow(c, s, s.safety, top, 7, 32, 55);
    lampRow(c, s, s.safety, middle, 6, 93, 55);
    lampRow(c, s, s.safety, bottom, 6, 154, 55);
  } else {
    /** CS-ATC upper-row labels. */
    static const char* top[] = {"過電流",   "三相",    "非常短絡", "対雪ブレーキ", "直通予備",
                                "非常運転", "ATC開放", "定速",     "駐車ブレーキ"};
    /** CS-ATC middle-row labels. */
    static const char* middle[] = {"TASC", "TASC制御", "TASCブレーキ", "ATO",    "地下鉄",
                                   "JR",   "ATC常用",  "ATC非常",      "ATC電源"};
    /** CS-ATC lower-row labels; null omits its frame. */
    static const char* bottom[] = {"ホームドア", "",    "",        nullptr,  "構内",
                                   "非設",       "ATC", "ATS動作", "ATS電源"};
    lampRow(c, s, s.safety, top, 9, 32, 55);
    lampRow(c, s, s.safety, middle, 9, 93, 55);
    lampRow(c, s, s.safety, bottom, 9, 154, 55);
  }
}

}  // namespace meter
