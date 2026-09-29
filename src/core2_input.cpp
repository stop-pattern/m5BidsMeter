#include "core2_input.h"

#include <M5Unified.h>

#include "render.h"
#include "sound_wave.h"

namespace meter {
namespace {

/** Persistent PCM data used by asynchronous M5Unified playback. */
int16_t clickSamples[kClickSampleCount] = {};
/** Whether the button waveform has been generated once. */
bool clickSamplesReady = false;
/** Converts a UI percentage to the speaker hardware level. */
unsigned speakerVolume(uint8_t percent) { return kSoundFullVolume * percent / 100; }

/** Plays one action tone only while the user has enabled sound. */
void playClick(const State& state) {
  if (state.soundPercent == 0) return;
  if (!clickSamplesReady) {
    fillClickWave(clickSamples);
    clickSamplesReady = true;
  }
  M5.Speaker.playRaw(clickSamples, kClickSampleCount, kClickSampleRate, false, 1, 0, true);
}

}  // namespace

bool Core2Input::update(State& state) {
  if (M5.BtnB.isPressed() && M5.BtnB.pressedFor(800) && !longPressHandled_) {
    state.screen = Screen::Select;
    longPressHandled_ = true;
    playClick(state);
  }
  if (M5.BtnB.wasReleased()) {
    if (!longPressHandled_) {
      state.screen = Screen::Home;
      playClick(state);
    }
    longPressHandled_ = false;
  }

  const auto touch = M5.Touch.getDetail();
  if (!touch.wasPressed() || touch.y < 0 || touch.y >= 240) return false;

  const uint8_t previousBrightness = state.brightness;
  const uint8_t previousSound = state.soundPercent;
  const bool activated = tap(state, touch.x, touch.y);
  if (state.soundPercent != previousSound) M5.Speaker.setVolume(speakerVolume(state.soundPercent));
  if (activated) playClick(state);
  return previousBrightness != state.brightness;
}

}  // namespace meter
