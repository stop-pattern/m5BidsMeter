#include "frame_diff.h"

#include <string.h>

namespace meter {

void markChangedBands(const uint8_t* next, const uint8_t* previous, size_t bytesPerBand,
                      int bandCount, bool* changed) {
  for (int band = 0; band < bandCount; ++band) {
    const size_t offset = size_t(band) * bytesPerBand;
    changed[band] = memcmp(next + offset, previous + offset, bytesPerBand) != 0;
  }
}

}  // namespace meter
