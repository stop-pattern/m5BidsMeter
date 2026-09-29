#pragma once

#include "render.h"

namespace meter {

/** Draws a rectangular one-pixel border around a lamp or panel. */
void outline(Canvas& canvas, int x, int y, int width, int height, Color color);
/** Draws the header and persistent buttons shared by all screens. */
void renderCommon(Canvas& canvas, const State& state);
/** Draws a communication-loss indicator when the last reply is stale. */
void renderStatus(Canvas& canvas, const State& state, uint32_t nowMs);
/** Draws the four-screen home menu. */
void renderHome(Canvas& canvas, const State& state);
/** Draws the 0 to 160 km/h speed dial and signal aspect. */
void renderSpeed(Canvas& canvas, const State& state);
/** Draws BC and MR vertical gauges, including the rolling warning. */
void renderPressure(Canvas& canvas, const State& state, uint32_t nowMs);
/** Draws brake bars, emergency brake, and holding brake. */
void renderBrake(Canvas& canvas, const State& state);
/** Draws the lamp layout for the selected safety system. */
void renderSafety(Canvas& canvas, const State& state);
/** Draws the safety-system selection menu. */
void renderSelect(Canvas& canvas, const State& state);
/** Draws a readable vertical lamp label within its exact frame. */
void drawVerticalLabel(Canvas& canvas, int x, int y, int width, int height, const char* label,
                       Color color);

}  // namespace meter
