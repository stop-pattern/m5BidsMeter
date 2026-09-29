#include "core2_display.h"

#include "frame_diff.h"
#include "render.h"

namespace meter {
namespace {

/** Native display width in pixels. */
constexpr int kWidth = 320;
/** Native display height in pixels. */
constexpr int kHeight = 240;
/** Height of one LCD comparison and transfer band. */
constexpr int kBandHeight = 16;
/** Number of full-height comparison bands. */
constexpr int kBandCount = kHeight / kBandHeight;

/** Converts the shared RGB888 color to the panel's RGB565 format. */
uint16_t rgb565(Color color) {
  return uint16_t(((color >> 19) & 31) << 11 | ((color >> 10) & 63) << 5 | ((color >> 3) & 31));
}

/** Adapts the platform-independent Canvas calls to an M5GFX target. */
class GfxCanvas final : public Canvas {
 public:
  /** Selects the off-screen sprite or LCD that receives drawing calls. */
  explicit GfxCanvas(lgfx::LovyanGFX& target) : target_(target) {}

  /** Fills one rectangle. */
  void rect(int x, int y, int width, int height, Color color) override {
    if (width > 0 && height > 0) target_.fillRect(x, y, width, height, rgb565(color));
  }

  /** Draws one line, using adjacent lines for greater thickness. */
  void line(int x1, int y1, int x2, int y2, Color color, int width = 1) override {
    for (int offset = 0; offset < width; ++offset)
      target_.drawLine(x1, y1 + offset, x2, y2 + offset, rgb565(color));
  }

  /** Draws readable Japanese text with a Gothic bitmap font. */
  void text(int x, int y, const char* value, Color color, int size, bool center = false) override {
    if (size >= 19)
      target_.setFont(&fonts::lgfxJapanGothic_20);
    else if (size >= 15)
      target_.setFont(&fonts::lgfxJapanGothic_16);
    else
      target_.setFont(&fonts::lgfxJapanGothic_12);
    target_.setTextColor(rgb565(color));
    target_.setTextSize(1);
    target_.drawString(value, center ? x - target_.textWidth(value) / 2 : x, y);
  }

  /** Fills the center dot of a gauge. */
  void circle(int x, int y, int radius, Color color) override {
    target_.fillCircle(x, y, radius, rgb565(color));
  }

 private:
  /** Graphics surface used for every draw call. */
  lgfx::LovyanGFX& target_;
};

}  // namespace

Core2Display::Core2Display()
    : frameA_(&M5.Display), frameB_(&M5.Display), previous_(&frameA_), current_(&frameB_) {}

bool Core2Display::begin() {
  M5.Display.setRotation(1);
  frameA_.setColorDepth(16);
  frameB_.setColorDepth(16);
  buffered_ = frameA_.createSprite(kWidth, kHeight) != nullptr &&
              frameB_.createSprite(kWidth, kHeight) != nullptr;
  lastDrawMs_ = millis() - 250;
  return buffered_;
}

void Core2Display::setBrightness(uint8_t percent) { M5.Display.setBrightness(percent); }

void Core2Display::update(const State& state, uint32_t nowMs) {
  if (uint32_t(nowMs - lastDrawMs_) < 250) return;
  lastDrawMs_ += 250;

  if (!buffered_) {
    GfxCanvas canvas(M5.Display);
    render(canvas, state, nowMs);
    return;
  }

  GfxCanvas canvas(*current_);
  render(canvas, state, nowMs);
  transferChanges(state);
  M5Canvas* oldPrevious = previous_;
  previous_ = current_;
  current_ = oldPrevious;
  previousScreen_ = state.screen;
  previousSafety_ = state.safety;
  hasPrevious_ = true;
}

void Core2Display::transferChanges(const State& state) {
  if (!hasPrevious_ || state.screen != previousScreen_ || state.safety != previousSafety_) {
    current_->pushSprite(&M5.Display, 0, 0);
    return;
  }

  /** Dirty flags for every 16-pixel LCD band. */
  bool changed[kBandCount] = {};
  /** Raw pixels of the newly rendered RGB565 frame. */
  const auto* next = static_cast<const uint8_t*>(current_->getBuffer());
  /** Raw pixels of the previously displayed RGB565 frame. */
  const auto* prior = static_cast<const uint8_t*>(previous_->getBuffer());
  markChangedBands(next, prior, kBandHeight * kWidth * 2, kBandCount, changed);

  M5.Display.startWrite();
  for (int band = 0; band < kBandCount;) {
    if (!changed[band]) {
      ++band;
      continue;
    }
    const int first = band;
    while (band < kBandCount && changed[band]) ++band;
    M5.Display.setClipRect(0, first * kBandHeight, kWidth, (band - first) * kBandHeight);
    current_->pushSprite(&M5.Display, 0, 0);
  }
  M5.Display.clearClipRect();
  M5.Display.endWrite();
}

}  // namespace meter
