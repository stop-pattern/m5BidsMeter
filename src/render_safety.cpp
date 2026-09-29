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
    canvas.rect(left, y, lampWidth, height, lit ? lamp.litColor : 0x111827);
    outline(canvas, left, y, lampWidth, height, lit ? lamp.litColor : 0x445065);
    if (*lamp.label)
      drawVerticalLabel(canvas, left, y, lampWidth, height, lamp.label, lit ? BG : MUTED);
  }
}

/** Draws ATS-P/Sn's photographed upper, TASC, and door groups. */
static void renderAts(Canvas& canvas, const State& state) {
  /** Upper nine lamps in the order shown in img/0.jpeg. */
  static const LampSpec kUpper[] = {
      {"P電源", GREEN},      {"パターン接近", ORANGE}, {"常用ブレーキ", ORANGE},
      {"非常ブレーキ", RED}, {"ブレーキ開放", GREEN},  {"ATS-P", GREEN},
      {"故障", RED},         {"ATS電源", WHITE},       {"ATS動作", ORANGE}};
  /** Five TASC lamps below the left side of the upper group. */
  static const LampSpec kTasc[] = {{"TASC電源", GREEN},
                                   {"TASCパターン", ORANGE},
                                   {"TASCブレーキ", ORANGE},
                                   {"TASC切", ORANGE},
                                   {"TASC故障", RED}};
  /** Lower seven lamps for rolling prevention and train/platform doors. */
  static const LampSpec kDoors[] = {{"転動防止ブレーキ", ORANGE}, {"定位置", GREEN},
                                    {"車両ドア全閉", GREEN},      {"ホームドア全閉", GREEN},
                                    {"ホームドア連携", GREEN},    {"ホームドア分離", ORANGE},
                                    {"ホームドア開放", ORANGE}};
  drawGroup(canvas, state, kUpper, 9, 8, 32, 304, 68);
  drawGroup(canvas, state, kTasc, 5, 8, 106, 184, 61);
  drawGroup(canvas, state, kDoors, 7, 8, 175, 246, 57);
}

/** Draws D-ATC's left equipment group and right ATC groups. */
static void renderDatc(Canvas& canvas, const State& state) {
  /** Seven equipment lamps occupying the left display area. */
  static const LampSpec kEquipment[] = {
      {"", WHITE},         {"三相", WHITE}, {"非常短絡", RED},       {"耐雪ブレーキ", ORANGE},
      {"直通予備", WHITE}, {"定速", GREEN}, {"駐車ブレーキ", ORANGE}};
  /** Upper right ATC status group. */
  static const LampSpec kAtcUpper[] = {{"デジタルATC", GREEN},   {"ATC", GREEN},
                                       {"切", ORANGE},           {"ATS電源", WHITE},
                                       {"パターン低減", ORANGE}, {"非常運転", RED}};
  /** Lower right ATC action group. */
  static const LampSpec kAtcLower[] = {{"ATC常用", ORANGE},      {"ATC非常", RED},
                                       {"停通防止動作", ORANGE}, {"ATS動作", ORANGE},
                                       {"ATC電源", WHITE},       {"ATC開放", GREEN}};
  drawGroup(canvas, state, kEquipment, 7, 8, 32, 153, 174);
  drawGroup(canvas, state, kAtcUpper, 6, 169, 32, 143, 82);
  drawGroup(canvas, state, kAtcLower, 6, 169, 121, 143, 82);
}

/** Draws CS-ATC's photographed left equipment and right ATC regions. */
static void renderCsatc(Canvas& canvas, const State& state) {
  /** Nine equipment lamps at the upper left. */
  static const LampSpec kEquipment[] = {
      {"過電流", RED},          {"三相", WHITE},     {"非常短絡", RED},
      {"対雪ブレーキ", ORANGE}, {"直通予備", WHITE}, {"非常運転", RED},
      {"ATC開放", GREEN},       {"定速", GREEN},     {"駐車ブレーキ", ORANGE}};
  /** Four TASC/ATO lamps at the lower left. */
  static const LampSpec kTasc[] = {
      {"TASC", GREEN}, {"TASC制御", GREEN}, {"TASCブレーキ", ORANGE}, {"ATO", GREEN}};
  /** Door group with two unlabeled lamps and one absent frame. */
  static const LampSpec kDoors[] = {
      {"ホームドア", GREEN}, {"", WHITE}, {"", WHITE}, {nullptr, WHITE}};
  /** Route and ATC action lamps at the upper right. */
  static const LampSpec kAtcUpper[] = {
      {"地下鉄", WHITE}, {"JR", WHITE}, {"ATC常用", ORANGE}, {"ATC非常", RED}, {"ATC電源", WHITE}};
  /** Route/ATS action lamps at the lower right. */
  static const LampSpec kAtcLower[] = {
      {"構内", WHITE}, {"非設", ORANGE}, {"ATC", GREEN}, {"ATS動作", ORANGE}, {"ATS電源", WHITE}};
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
