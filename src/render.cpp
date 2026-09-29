#include "render.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

namespace meter {

static void outline(Canvas& c, int x, int y, int w, int h, Color color) {
  c.line(x, y, x+w-1, y, color); c.line(x, y, x, y+h-1, color);
  c.line(x+w-1, y, x+w-1, y+h-1, color); c.line(x, y+h-1, x+w-1, y+h-1, color);
}

static void button(Canvas& c, int x, int y, int w, int h, const char* label) {
  c.rect(x, y, w, h, 0x344e71); outline(c, x, y, w, h, 0xa3bad4);
  c.text(x+w/2, y+(h-13)/2, label, WHITE, 12, true);
}

static void common(Canvas& c, const State& s) {
  c.rect(0, 0, 320, 240, BG);
  c.rect(0, 0, 320, 27, 0x172235);
  if (s.screen != Screen::Home) button(c, 4, 3, 64, 21, "画面変更");
  const char* title = s.screen == Screen::Home ? "TIMS メーター" :
    s.screen == Screen::Speed ? "速度計" : s.screen == Screen::Pressure ? "BC / MR 圧力計" :
    s.screen == Screen::Brake ? "ブレーキ段数" : s.screen == Screen::Safety ? "保安装置" : "保安装置選択";
  c.text(162, 5, title, WHITE, 14, true);
  button(c, 287, 213, 29, 23, "☼");
}

static void status(Canvas& c, const State& s, uint32_t nowMs) {
  if (s.disconnected(nowMs)) c.text(280, 5, "通信断", RED, 11, true);
}

static void home(Canvas& c, const State& s) {
  static const char* names[] = {"速度計", "圧力計", "ブレーキ段数", "保安装置"};
  for (int i=0; i<4; ++i) {
    const int x=20+(i%2)*150, y=47+(i/2)*76;
    c.rect(x,y,130,62,PANEL); outline(c,x,y,130,62,EDGE);
    c.rect(x+4,y+4,5,54,i==0?GREEN:i==1?ORANGE:i==2?YELLOW:0x8aabff);
    c.text(x+70,y+20,names[i],WHITE,16,true);
  }
  const char* safety = s.safety==Safety::Ats?"ATS-P / Sn":s.safety==Safety::Datc?"D-ATC":"CS-ATC / ATC-10";
  c.text(160,207,safety,MUTED,12,true);
}

static int bound(int x,int low,int high) { return x<low?low:x>high?high:x; }
static void speed(Canvas& c, const State& s) {
  const int cx=160, cy=154, r=91;
  for (int i=0;i<=80;++i) {
    const float a=(150.0f+240.0f*i/80.0f)*3.14159265f/180.0f;
    const int x1=cx+int(cosf(a)*(r-((i%10)==0?13:6)));
    const int y1=cy+int(sinf(a)*(r-((i%10)==0?13:6)));
    c.line(x1,y1,cx+int(cosf(a)*r),cy+int(sinf(a)*r),WHITE,(i%10)==0?2:1);
    if (i%10==0) {
      char label[8]; snprintf(label,sizeof(label),"%d",i*2);
      c.text(cx+int(cosf(a)*(r+13)),cy+int(sinf(a)*(r+13))-6,label,WHITE,11,true);
    }
  }
  const float a=(150.0f+240.0f*bound(int(s.speed),0,160)/160.0f)*3.14159265f/180.0f;
  c.line(cx,cy,cx+int(cosf(a)*73),cy+int(sinf(a)*73),WHITE,5);
  c.circle(cx,cy,6,WHITE);
  if (s.safety != Safety::Ats) {
    const int signal=s.signalSpeed();
    const bool stop=signal<=0;
    const float sa=(150.0f+240.0f*bound(signal<0?0:signal,0,160)/160.0f)*3.14159265f/180.0f;
    const int tx=cx+int(cosf(sa)*(r+4)), ty=cy+int(sinf(sa)*(r+4));
    const Color signalColor=stop?RED:GREEN;
    c.line(tx,ty-6,tx-6,ty+5,signalColor,2);
    c.line(tx-6,ty+5,tx+6,ty+5,signalColor,2);
    c.line(tx+6,ty+5,tx,ty-6,signalColor,2);
    if (s.safety==Safety::Datc || signal<0 || s.lamp(101)) c.text(162,180,"×",RED,20,true);
  }
  char value[32]; snprintf(value,sizeof(value),"%.0f km/h",floorf(s.speed+0.5f));
  c.rect(108,186,110,24,BG); c.text(160,188,value,WHITE,19,true);
}

static void pressureGauge(Canvas& c,int x,int top,int bottom,float value,const char* label,Color fill,bool warning,bool blink) {
  const int y0=50,y1=204,h=y1-y0;
  c.text(x,32,label,WHITE,17,true);
  c.rect(x-27,y0,27,h,0x141c2b);
  const float fraction=fmaxf(0.0f,fminf(1.0f,(value-top)/float(bottom-top)));
  const int fillH=int(fraction*h);
  c.rect(x-26,y1-fillH,25,fillH,fill);
  outline(c,x-27,y0,27,h,EDGE);
  const int step=top==0?20:10;
  const int majorStep=top==0?200:100;
  for(int n=top;n<=bottom;n+=step) {
    const int y=y1-int((n-top)*h/float(bottom-top));
    const bool major=(n-top)%majorStep==0;
    Color color=(warning && n==200 && blink)?RED:WHITE;
    c.line(x,y,x+(major?13:7),y,color,major?2:1);
    if(major){char num[12];snprintf(num,sizeof(num),"%d",n);c.text(x+18,y-6,num,color,10);}
  }
  const int marker=y1-fillH;
  c.line(x-34,marker,x+3,marker,fill,4);
  char v[24];snprintf(v,sizeof(v),"%.0f kPa",floorf(value+0.5f));
  c.text(x+8,209,v,WHITE,12,true);
}

static void pressure(Canvas& c,const State& s,uint32_t nowMs) {
  pressureGauge(c,94,0,800,s.bc,"BC",WHITE,s.rollingWarning(),s.warningRed(nowMs));
  pressureGauge(c,231,700,1000,s.mr,"MR",ORANGE,false,false);
}

static void brake(Canvas& c,const State& s) {
  c.text(160,37,"制動ハンドル",MUTED,13,true);
  c.rect(23,57,272,28,s.brake==9?0x782d35:PANEL);
  outline(c,23,57,272,28,s.brake==9?RED:EDGE);
  c.text(160,62,s.brake==9?"非常":s.brake==0?"緩解":"常用ブレーキ",s.brake==9?RED:WHITE,17,true);
  if (s.holdingBrake()) {
    c.rect(100,90,120,25,0x245c3b); outline(c,100,90,120,25,GREEN);
    c.text(160,94,"抑速",GREEN,15,true);
  } else c.text(160,94,"抑速",MUTED,14,true);
  for(int i=0;i<8;++i){
    const int y=191-i*9;
    c.rect(58,y,205,7,(s.brake>=i+1&&s.brake<=8)?YELLOW:0x374253);
  }
  char value[12];snprintf(value,sizeof(value),"B%d",s.brake);
  c.text(160,201,s.brake==9?"EB":s.brake==0?"B0":value,WHITE,17,true);
}

static void utf8Vertical(Canvas& c,int cx,int y,const char* label,Color color,int fontSize,int spacing) {
  const unsigned char* p=(const unsigned char*)label;
  while(*p && y<209){
    int len=(*p<0x80)?1:((*p&0xE0)==0xC0?2:((*p&0xF0)==0xE0?3:4));
    char ch[5]={}; for(int i=0;i<len&&p[i];++i)ch[i]=char(p[i]);
    c.text(cx,y,ch,color,fontSize,true);p+=len;y+=spacing;
  }
}

static void lampRow(Canvas& c,const State& s,Safety safety,const char* const* labels,int count,int y,int h) {
  const int margin=8,gap=2,w=(320-2*margin-gap*(count-1))/count;
  for(int i=0;i<count;++i){
    const int x=margin+i*(w+gap);
    if(!labels[i])continue;
    const bool lit=lampOn(s,safety,labels[i]);
    const Color color=lit?GREEN:MUTED;
    c.rect(x,y,w,h,lit?0x407246:0x111827);outline(c,x,y,w,h,lit?GREEN:0x445065);
    if(*labels[i])utf8Vertical(c,x+w/2,y+4,labels[i],color,w>=34?11:9,w>=34?12:10);
  }
}

static void safety(Canvas& c,const State& s) {
  if(s.safety==Safety::Ats){
    static const char* top[]={"P電源","パターン接近","常用ブレーキ","非常ブレーキ","ブレーキ開放","ATS-P","故障","ATS電源","ATS動作"};
    static const char* bottom[]={"","三相","非常短絡","耐雪ブレーキ","直通予備","定速","駐車ブレーキ"};
    lampRow(c,s,s.safety,top,9,38,77);lampRow(c,s,s.safety,bottom,7,125,77);
  } else if(s.safety==Safety::Datc){
    static const char* top[]={"","三相","非常短絡","耐雪ブレーキ","直通予備","定速","駐車ブレーキ"};
    static const char* middle[]={"デジタルATC","ATC","切","ATS電源","パターン低減","非常運転"};
    static const char* bottom[]={"ATC常用","ATC非常","停通防止動作","ATS動作","ATC電源","ATC開放"};
    lampRow(c,s,s.safety,top,7,32,55);lampRow(c,s,s.safety,middle,6,93,55);lampRow(c,s,s.safety,bottom,6,154,55);
  } else {
    static const char* top[]={"過電流","三相","非常短絡","対雪ブレーキ","直通予備","非常運転","ATC開放","定速","駐車ブレーキ"};
    static const char* middle[]={"TASC","TASC制御","TASCブレーキ","ATO","地下鉄","JR","ATC常用","ATC非常","ATC電源"};
    static const char* bottom[]={"ホームドア","","",nullptr,"構内","非設","ATC","ATS動作","ATS電源"};
    lampRow(c,s,s.safety,top,9,32,55);lampRow(c,s,s.safety,middle,9,93,55);lampRow(c,s,s.safety,bottom,9,154,55);
  }
}

static void select(Canvas& c,const State& s) {
  static const char* names[]={"ATS-P / Sn  E233-0・3000","D-ATC  E233-1000","CS-ATC / ATC-10  E233-2000"};
  for(int i=0;i<3;++i){int y=41+i*55; c.rect(12,y,296,46,PANEL);outline(c,12,y,296,46,EDGE);c.text(160,y+13,names[i],WHITE,14,true);}
  (void)s;
}

void render(Canvas& c,const State& state,uint32_t nowMs) {
  common(c,state);
  switch(state.screen){
    case Screen::Home:home(c,state);break;
    case Screen::Speed:speed(c,state);break;
    case Screen::Pressure:pressure(c,state,nowMs);break;
    case Screen::Brake:brake(c,state);break;
    case Screen::Safety:safety(c,state);break;
    case Screen::Select:select(c,state);break;
  }
  status(c,state,nowMs);
}

void tap(State& s,int x,int y) {
  if(x>=287&&y>=211){s.cycleBrightness();return;}
  if(s.screen!=Screen::Home&&x<70&&y<27){s.screen=Screen::Home;return;}
  if(s.screen==Screen::Home){
    if(x>=20&&x<300&&y>=47&&y<185){int col=x>=170?1:0,row=y>=123?1:0;s.screen=Screen(1+row*2+col);}
  } else if(s.screen==Screen::Select&&y>=41&&y<197){
    int row=(y-41)/55;
    if(row>=0&&row<3){s.safety=Safety(row);s.screen=Screen::Home;}
  }
}

} // namespace meter
