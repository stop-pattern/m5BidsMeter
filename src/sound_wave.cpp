#include "sound_wave.h"

#include <math.h>

namespace meter {
namespace {

/** Fundamental frequency of the requested button sound. */
constexpr int kLowHz = 2900;
/** Second sine frequency of the requested button sound. */
constexpr int kHighHz = 5800;
/** Peak amplitude of each sine before summing into signed 16-bit PCM. */
constexpr double kComponentAmplitude = 12000.0;
/** Fade at both ends to prevent sharp sample discontinuities. */
constexpr int kFadeSamples = kClickSampleRate * 3 / 1000;
/** Circle constant used to convert sample position into sine phase. */
constexpr double kTwoPi = 6.28318530717958647692;

}  // namespace

void fillClickWave(int16_t* samples) {
  for (int index = 0; index < kClickSampleCount; ++index) {
    double envelope = 1.0;
    if (index < kFadeSamples) envelope = double(index) / kFadeSamples;
    if (index >= kClickSampleCount - kFadeSamples)
      envelope = double(kClickSampleCount - 1 - index) / kFadeSamples;
    const double time = double(index) / kClickSampleRate;
    const double low = sin(kTwoPi * kLowHz * time);
    const double high = sin(kTwoPi * kHighHz * time);
    samples[index] = int16_t(envelope * kComponentAmplitude * (low + high));
  }
}

}  // namespace meter
