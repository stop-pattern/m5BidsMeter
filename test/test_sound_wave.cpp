#include "sound_wave.h"

#include <math.h>
#include <stdint.h>

/** Measures one frequency component in a generated PCM click. */
static double component(const int16_t* samples, int frequency) {
  double sine = 0;
  double cosine = 0;
  for (int index = 0; index < meter::kClickSampleCount; ++index) {
    const double phase = 2.0 * 3.14159265358979323846 * frequency * index / meter::kClickSampleRate;
    sine += samples[index] * sin(phase);
    cosine += samples[index] * cos(phase);
  }
  return sqrt(sine * sine + cosine * cosine);
}

/** Verifies the two requested sine components and a quiet start and end. */
int main() {
  static int16_t samples[meter::kClickSampleCount];
  meter::fillClickWave(samples);
  if (samples[0] != 0 || samples[meter::kClickSampleCount - 1] != 0) return 1;
  const double low = component(samples, 2900);
  const double high = component(samples, 5800);
  const double other = component(samples, 1500);
  if (low < other * 20 || high < other * 20) return 2;
  if (high / low < 0.8 || high / low > 1.2) return 3;
  return 0;
}
