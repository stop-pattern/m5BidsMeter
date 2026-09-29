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

/** Label and four-color illuminated face of a safety lamp. */
struct LampSpec {
  /** Text shown inside the lamp; null means no frame in that position. */
  const char* label;
  /** Background color used only when a confirmed Panel value lights the lamp. */
  Color litColor;
};

/** Draws a photographed annunciator without the former blue outline. */
void drawLampFace(Canvas& canvas, int x, int y, int width, int height, bool lit, Color color) {
  canvas.rect(x, y, width, height, LAMP_DARK);
  if (lit) {
    canvas.rect(x + 1, y, width - 2, height, color);
    canvas.rect(x, y + 1, width, height - 2, color);
  }
}

/** Draws one photographed lamp group at its own screen position. */
static void drawGroup(Canvas& canvas, const State& state, const LampSpec* lamps, int count, int x,
                      int y, int width, int height) {
  /** Horizontal gap between neighboring lamp frames. */
  constexpr int kGap = 2;
  /** Equal frame width within this group. */
  const int lampWidth = (width - kGap * (count - 1)) / count;
  for (int index = 0; index < count; ++index) {
    const LampSpec& lamp = lamps[index];
    if (lamp.label == nullptr) continue;
    const int left = x + index * (lampWidth + kGap);
    const bool lit = lampOn(state, state.safety, lamp.label);
    drawLampFace(canvas, left, y, lampWidth, height, lit, lamp.litColor);
    if (*lamp.label)
      drawVerticalLabel(canvas, left, y, lampWidth, height, lamp.label,
                        lit ? LAMP_TEXT : LAMP_UNLIT_TEXT);
  }
}

/** Draws ATS-P/Sn's photographed upper, TASC, and door groups. */
static void renderAts(Canvas& canvas, const State& state) {
  /** Upper nine lamps in the order shown in img/0.jpeg. */
  static const LampSpec kUpper[] = {
      {"P電源", LAMP_GREEN},      {"パターン接近", LAMP_ORANGE}, {"常用ブレーキ", LAMP_ORANGE},
      {"非常ブレーキ", LAMP_RED}, {"ブレーキ開放", LAMP_GREEN},  {"ATS-P", LAMP_GREEN},
      {"故障", LAMP_RED},         {"ATS電源", LAMP_WHITE},       {"ATS動作", LAMP_ORANGE}};
  /** Five TASC lamps below the left side of the upper group. */
  static const LampSpec kTasc[] = {{"TASC電源", LAMP_GREEN},
                                   {"TASCパターン", LAMP_ORANGE},
                                   {"TASCブレーキ", LAMP_ORANGE},
                                   {"TASC切", LAMP_ORANGE},
                                   {"TASC故障", LAMP_RED}};
  /** Lower seven lamps for rolling prevention and train/platform doors. */
  static const LampSpec kDoors[] = {
      {"転動防止ブレーキ", LAMP_ORANGE}, {"定位置", LAMP_GREEN},
      {"車両ドア全閉", LAMP_GREEN},      {"ホームドア全閉", LAMP_GREEN},
      {"ホームドア連携", LAMP_GREEN},    {"ホームドア分離", LAMP_ORANGE},
      {"ホームドア開放", LAMP_ORANGE}};
  drawGroup(canvas, state, kUpper, 9, 8, 32, 304, 68);
  drawGroup(canvas, state, kTasc, 5, 8, 106, 184, 61);
  drawGroup(canvas, state, kDoors, 7, 8, 175, 246, 57);
}

/** Draws D-ATC's left equipment group and right ATC groups. */
static void renderDatc(Canvas& canvas, const State& state) {
  /** Seven equipment lamps occupying the left display area. */
  static const LampSpec kEquipment[] = {{"", LAMP_WHITE},
                                        {"三相", LAMP_WHITE},
                                        {"非常短絡", LAMP_RED},
                                        {"耐雪ブレーキ", LAMP_ORANGE},
                                        {"直通予備", LAMP_WHITE},
                                        {"定速", LAMP_GREEN},
                                        {"駐車ブレーキ", LAMP_ORANGE}};
  /** Upper right ATC status group. */
  static const LampSpec kAtcUpper[] = {{"デジタルATC", LAMP_GREEN},   {"ATC", LAMP_GREEN},
                                       {"切", LAMP_ORANGE},           {"ATS電源", LAMP_WHITE},
                                       {"パターン低減", LAMP_ORANGE}, {"非常運転", LAMP_RED}};
  /** Lower right ATC action group. */
  static const LampSpec kAtcLower[] = {{"ATC常用", LAMP_ORANGE},      {"ATC非常", LAMP_RED},
                                       {"停通防止動作", LAMP_ORANGE}, {"ATS動作", LAMP_ORANGE},
                                       {"ATC電源", LAMP_WHITE},       {"ATC開放", LAMP_GREEN}};
  drawGroup(canvas, state, kEquipment, 7, 8, 32, 153, 174);
  drawGroup(canvas, state, kAtcUpper, 6, 169, 32, 143, 82);
  drawGroup(canvas, state, kAtcLower, 6, 169, 121, 143, 82);
}

/** Draws CS-ATC's photographed left equipment and right ATC regions. */
static void renderCsatc(Canvas& canvas, const State& state) {
  /** Nine equipment lamps at the upper left. */
  static const LampSpec kEquipment[] = {
      {"過電流", LAMP_RED},          {"三相", LAMP_WHITE},     {"非常短絡", LAMP_RED},
      {"対雪ブレーキ", LAMP_ORANGE}, {"直通予備", LAMP_WHITE}, {"非常運転", LAMP_RED},
      {"ATC開放", LAMP_GREEN},       {"定速", LAMP_GREEN},     {"駐車ブレーキ", LAMP_ORANGE}};
  /** Four TASC/ATO lamps at the lower left. */
  static const LampSpec kTasc[] = {{"TASC", LAMP_GREEN},
                                   {"TASC制御", LAMP_GREEN},
                                   {"TASCブレーキ", LAMP_ORANGE},
                                   {"ATO", LAMP_GREEN}};
  /** Door group with two unlabeled lamps and one absent frame. */
  static const LampSpec kDoors[] = {
      {"ホームドア", LAMP_GREEN}, {"", LAMP_WHITE}, {"", LAMP_WHITE}, {nullptr, LAMP_WHITE}};
  /** Route and ATC action lamps at the upper right. */
  static const LampSpec kAtcUpper[] = {{"地下鉄", LAMP_WHITE},
                                       {"JR", LAMP_WHITE},
                                       {"ATC常用", LAMP_ORANGE},
                                       {"ATC非常", LAMP_RED},
                                       {"ATC電源", LAMP_WHITE}};
  /** Route/ATS action lamps at the lower right. */
  static const LampSpec kAtcLower[] = {{"構内", LAMP_WHITE},
                                       {"非設", LAMP_ORANGE},
                                       {"ATC", LAMP_GREEN},
                                       {"ATS動作", LAMP_ORANGE},
                                       {"ATS電源", LAMP_WHITE}};
  drawGroup(canvas, state, kEquipment, 9, 8, 32, 153, 92);
  drawGroup(canvas, state, kTasc, 4, 8, 131, 153, 45);
  drawGroup(canvas, state, kDoors, 4, 8, 184, 153, 45);
  drawGroup(canvas, state, kAtcUpper, 5, 169, 32, 143, 92);
  drawGroup(canvas, state, kAtcLower, 5, 169, 131, 143, 95);
}

/** Draws the lamp layout matching the selected photographed safety system. */
void renderSafety(Canvas& canvas, const State& state) {
  switch (state.safety) {
    case Safety::Ats:
      renderAts(canvas, state);
      break;
    case Safety::Datc:
      renderDatc(canvas, state);
      break;
    case Safety::Csatc:
      renderCsatc(canvas, state);
      break;
  }
}

}  // namespace meter
