#include "meter.h"
#include "render.h"

#include <assert.h>

int main() {
  meter::State s;
  assert(s.rollingWarning());
  assert(s.warningRed(0) && !s.warningRed(500));
  assert(!s.disconnected(4999) && s.disconnected(5000));
  assert(s.apply("TRV202X100", 100));
  assert(s.apply("TRIE1X12.5", 101) && s.speed == 12.5f);
  assert(s.apply("TRIE3X199", 102) && s.bc == 199);
  assert(!s.rollingWarning());
  assert(s.apply("TRIE1X0", 103) && s.rollingWarning());
  assert(s.apply("TRIE3X200", 104) && !s.rollingWarning());
  assert(s.apply("TRIH0X9", 105) && s.brake == 9);
  assert(s.apply("TRIH1X-1", 106) && s.holdingBrake());
  assert(!s.apply("TRIE1Xabc", 107));
  assert(!s.apply("TRIP256X1", 107));
  assert(!s.apply("TRIP1X1.5", 107));
  assert(s.apply("TRIP2X1", 108) && meter::lampOn(s, meter::Safety::Ats, "P電源"));
  assert(!meter::lampOn(s, meter::Safety::Datc, "ATC"));
  s.safety = meter::Safety::Csatc;
  assert(s.signalSpeed() == -1);
  assert(s.apply("TRIP125X1", 110) && s.signalSpeed() == 120);
  assert(s.apply("TRIP110X1", 111) && s.signalSpeed() == 40);
  assert(s.apply("TRIP102X1", 112) && s.signalSpeed() == 0 && s.signalStop());
  assert(s.apply("TRIP92X6", 113) && meter::lampOn(s, s.safety, "JR"));
  assert(!meter::lampOn(s, s.safety, "地下鉄"));
  s.cycleBrightness(); assert(s.brightness == 25);
  s.cycleBrightness(); assert(s.brightness == 50);
  s.screen = meter::Screen::Home;
  meter::tap(s, 30, 60); assert(s.screen == meter::Screen::Speed);
  meter::tap(s, 10, 10); assert(s.screen == meter::Screen::Home);
  s.screen = meter::Screen::Select;
  meter::tap(s, 100, 110); assert(s.safety == meter::Safety::Datc && s.screen == meter::Screen::Home);
  meter::tap(s, 300, 220); assert(s.brightness == 75);
  return 0;
}
