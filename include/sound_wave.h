#pragma once

#include <stdint.h>

namespace meter {

/** PCM sampling rate used for the two-frequency button sound. */
constexpr int kClickSampleRate = 48000;
/** Duration of one button sound in milliseconds. */
constexpr int kClickDurationMs = 55;
/** Number of mono 16-bit samples in one button sound. */
constexpr int kClickSampleCount = kClickSampleRate * kClickDurationMs / 1000;

/** Fills a persistent PCM buffer with equal 2900 Hz and 5800 Hz sine waves. */
void fillClickWave(int16_t* samples);

}  // namespace meter
