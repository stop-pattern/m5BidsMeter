#include <Arduino.h>
#include <M5Unified.h>

#include "meter.h"
#include "render.h"

namespace {

meter::State state;
char input[80] = {};
size_t inputLength = 0;
uint32_t lastCorePoll = 0;
uint32_t lastPanelPoll = 0;
uint32_t lastDraw = 0;
bool longHandled = false;

uint16_t rgb565(meter::Color c) {
  return uint16_t(((c >> 19) & 31) << 11 | ((c >> 10) & 63) << 5 | ((c >> 3) & 31));
}

class CoreCanvas final : public meter::Canvas {
 public:
  void rect(int x,int y,int w,int h,meter::Color color) override {
    if(w>0&&h>0) M5.Display.fillRect(x,y,w,h,rgb565(color));
  }
  void line(int x1,int y1,int x2,int y2,meter::Color color,int width=1) override {
    for(int i=0;i<width;++i) M5.Display.drawLine(x1,y1+i,x2,y2+i,rgb565(color));
  }
  void text(int x,int y,const char* value,meter::Color color,int size,bool center=false) override {
    if(size>=19) M5.Display.setFont(&fonts::efontJA_24);
    else if(size>=15) M5.Display.setFont(&fonts::efontJA_16);
    else if(size>=12) M5.Display.setFont(&fonts::efontJA_12);
    else M5.Display.setFont(&fonts::efontJA_10);
    M5.Display.setTextColor(rgb565(color));
    M5.Display.setTextSize(1);
    M5.Display.drawString(value,center?x-M5.Display.textWidth(value)/2:x,y);
  }
  void circle(int x,int y,int r,meter::Color color) override { M5.Display.fillCircle(x,y,r,rgb565(color)); }
};

CoreCanvas canvas;

void readSerial(uint32_t now) {
  while(Serial.available()) {
    const int ch=Serial.read();
    if(ch=='\n'||ch=='\r') {
      if(inputLength) {input[inputLength]=0;state.apply(input,now);inputLength=0;}
    } else if(ch>=32&&ch<127) {
      if(inputLength+1<sizeof(input)) input[inputLength++]=char(ch);
      else inputLength=0;
    }
  }
}

void pollCore() {
  static const char* commands[]={"TRIE1","TRIE3","TRIE4","TRIH0","TRIH1"};
  for(const char* command:commands) Serial.println(command);
}

void pollPanel() {
  static const uint8_t ats[]={2,3,4,6,7};
  static const uint8_t csLamps[]={1,19,22,23,29,31,73,92,96,155,175};
  if(state.screen==meter::Screen::Safety) {
    if(state.safety==meter::Safety::Ats) for(uint8_t n:ats){Serial.print("TRIP");Serial.println(n);}
    if(state.safety==meter::Safety::Csatc) for(uint8_t n:csLamps){Serial.print("TRIP");Serial.println(n);}
  }
  if(state.screen==meter::Screen::Speed&&state.safety==meter::Safety::Csatc) {
    Serial.println("TRIP101");Serial.println("TRIP102");
    for(int n=104;n<=125;++n){Serial.print("TRIP");Serial.println(n);}
  }
}

void handleInput() {
  if(M5.BtnB.isPressed()&&M5.BtnB.pressedFor(800)&&!longHandled) {
    state.screen=meter::Screen::Select;longHandled=true;
  }
  if(M5.BtnB.wasReleased()) {
    if(!longHandled) state.screen=meter::Screen::Home;
    longHandled=false;
  }
  auto touch=M5.Touch.getDetail();
  if(touch.wasPressed()&&touch.y>=0&&touch.y<240) {
    const uint8_t before=state.brightness;
    meter::tap(state,touch.x,touch.y);
    if(before!=state.brightness) M5.Display.setBrightness(state.brightness);
  }
}

} // namespace

void setup() {
  auto cfg=M5.config();
  M5.begin(cfg);
  M5.Display.setRotation(1);
  M5.Display.setBrightness(100);
  Serial.begin(115200);
  Serial.println("TRV202");
  lastCorePoll=lastPanelPoll=lastDraw=millis();
}

void loop() {
  M5.update();
  const uint32_t now=millis();
  readSerial(now);
  handleInput();
  if(uint32_t(now-lastCorePoll)>=100) {lastCorePoll+=100;pollCore();}
  if(uint32_t(now-lastPanelPoll)>=200) {lastPanelPoll+=200;pollPanel();}
  if(uint32_t(now-lastDraw)>=250) {
    lastDraw=now;
    M5.Display.startWrite();
    meter::render(canvas,state,now);
    M5.Display.endWrite();
  }
  delay(2);
}
