#pragma once

#include <stddef.h>
#include <stdint.h>

namespace meter {

/** Marks bands whose RGB565 bytes differ from the previously shown frame. */
void markChangedBands(const uint8_t* next, const uint8_t* previous, size_t bytesPerBand,
                      int bandCount, bool* changed);

}  // namespace meter
