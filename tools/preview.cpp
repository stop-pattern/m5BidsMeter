#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <direct.h>

#include "render.h"

#include <stdio.h>

using meter::Color;

static COLORREF rgb(Color value) { return RGB((value>>16)&255,(value>>8)&255,value&255); }

class GdiCanvas : public meter::Canvas {
 public:
  GdiCanvas() {
    BITMAPINFO info={}; info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth=320; info.bmiHeader.biHeight=-240;
    info.bmiHeader.biPlanes=1; info.bmiHeader.biBitCount=32; info.bmiHeader.biCompression=BI_RGB;
    dc_=CreateCompatibleDC(nullptr);
    bitmap_=CreateDIBSection(dc_,&info,DIB_RGB_COLORS,&pixels_,nullptr,0);
    old_=SelectObject(dc_,bitmap_); SetBkMode(dc_,TRANSPARENT);
  }
  ~GdiCanvas() override { SelectObject(dc_,old_); DeleteObject(bitmap_); DeleteDC(dc_); }
  void rect(int x,int y,int w,int h,Color color) override {
    if(w<=0||h<=0)return;
    RECT r={x,y,x+w,y+h}; HBRUSH brush=CreateSolidBrush(rgb(color));FillRect(dc_,&r,brush);DeleteObject(brush);
  }
  void line(int x1,int y1,int x2,int y2,Color color,int width=1) override {
    HPEN pen=CreatePen(PS_SOLID,width,rgb(color));HGDIOBJ prev=SelectObject(dc_,pen);
    MoveToEx(dc_,x1,y1,nullptr);LineTo(dc_,x2,y2);SelectObject(dc_,prev);DeleteObject(pen);
  }
  void text(int x,int y,const char* value,Color color,int size,bool center=false) override {
    wchar_t wide[128]={}; MultiByteToWideChar(CP_UTF8,0,value,-1,wide,128);
    HFONT font=CreateFontW(-size,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,
      OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Yu Gothic UI");
    HGDIOBJ prev=SelectObject(dc_,font);SetTextColor(dc_,rgb(color));
    const int length=lstrlenW(wide);SIZE extent={};GetTextExtentPoint32W(dc_,wide,length,&extent);
    TextOutW(dc_,center?x-extent.cx/2:x,y,wide,length);
    SelectObject(dc_,prev);DeleteObject(font);
  }
  void circle(int x,int y,int r,Color color) override {
    HPEN pen=CreatePen(PS_SOLID,1,rgb(color));HBRUSH brush=CreateSolidBrush(rgb(color));
    HGDIOBJ oldPen=SelectObject(dc_,pen),oldBrush=SelectObject(dc_,brush);
    Ellipse(dc_,x-r,y-r,x+r+1,y+r+1);
    SelectObject(dc_,oldPen);SelectObject(dc_,oldBrush);DeleteObject(pen);DeleteObject(brush);
  }
  bool save(const char* path) {
    FILE* out=fopen(path,"wb");if(!out)return false;
    BITMAPFILEHEADER file={};BITMAPINFOHEADER info={};
    info.biSize=sizeof(info);info.biWidth=320;info.biHeight=-240;info.biPlanes=1;
    info.biBitCount=32;info.biCompression=BI_RGB;info.biSizeImage=320*240*4;
    file.bfType=0x4D42;file.bfOffBits=sizeof(file)+sizeof(info);file.bfSize=file.bfOffBits+info.biSizeImage;
    fwrite(&file,sizeof(file),1,out);fwrite(&info,sizeof(info),1,out);fwrite(pixels_,info.biSizeImage,1,out);
    fclose(out);return true;
  }
 private:
  HDC dc_={};HBITMAP bitmap_={};HGDIOBJ old_={};void* pixels_={};
};

int main() {
  _mkdir("preview");
  meter::State s;
  s.speed=72.5f;s.bc=330;s.mr=860;s.brake=6;s.power=-1;s.received=true;s.lastResponseMs=1000;
  struct Scene{const char* name;meter::Screen screen;meter::Safety safety;bool warning;};
  const Scene scenes[]={
    {"home",meter::Screen::Home,meter::Safety::Ats,false},
    {"speed_ats",meter::Screen::Speed,meter::Safety::Ats,false},
    {"speed_csatc",meter::Screen::Speed,meter::Safety::Csatc,false},
    {"pressure",meter::Screen::Pressure,meter::Safety::Ats,false},
    {"pressure_warning",meter::Screen::Pressure,meter::Safety::Ats,true},
    {"brake",meter::Screen::Brake,meter::Safety::Ats,false},
    {"safety_ats",meter::Screen::Safety,meter::Safety::Ats,false},
    {"safety_datc",meter::Screen::Safety,meter::Safety::Datc,false},
    {"safety_csatc",meter::Screen::Safety,meter::Safety::Csatc,false},
    {"select",meter::Screen::Select,meter::Safety::Ats,false},
  };
  for(const auto& scene:scenes){
    s.screen=scene.screen;s.safety=scene.safety;
    s.speed=scene.warning?0:72.5f;s.bc=scene.warning?150:330;
    GdiCanvas canvas;meter::render(canvas,s,2000);
    char path[96];snprintf(path,sizeof(path),"preview/%s.bmp",scene.name);
    if(!canvas.save(path))return 1;
  }
  return 0;
}
