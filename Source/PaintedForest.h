#pragma once
#include <JuceHeader.h>
#include <algorithm>
#include <cmath>

// Original vector artwork inspired by whimsical children's forest paintings:
// mushrooms, rough-hewn earth, imperfect spirals, ritual creatures.
// All GUI elements are rendered live: no preprinted faux knobs or labels.
namespace painted {
using namespace juce;
inline const Colour soil=Colour::fromRGB(17,20,15), bark=Colour::fromRGB(33,34,25);
inline const Colour stone=Colour::fromRGB(63,56,40), outline=Colour::fromRGB(109,87,61);
inline const Colour ivory=Colour::fromRGB(237,216,173), soft=Colour::fromRGB(199,181,143);
inline const Colour moss=Colour::fromRGB(147,166,104), clay=Colour::fromRGB(185,102,66);
inline const Colour ochre=Colour::fromRGB(221,175,97), slate=Colour::fromRGB(126,153,119);
inline const Colour ash=Colour::fromRGB(180,130,105);
inline const std::array<Colour,5> accents{{moss,clay,ochre,slate,ash}};
inline void spiral(Graphics& g,float x,float y,float radius,Colour tint,float weight=1.5f){
 Path p;
 for(int i=0;i<72;++i) {
  float t=(float)i/71.f,angle=2.f*MathConstants<float>::pi*2.9f*t,rad=radius*t;
  float xx=x+std::cos(angle)*rad, yy=y+std::sin(angle)*rad;
  if(i==0)p.startNewSubPath(xx,yy);else p.lineTo(xx,yy);
 }
 g.setColour(tint);g.strokePath(p,PathStrokeType(weight,PathStrokeType::curved,PathStrokeType::rounded));
}
inline void leaf(Graphics& g,float x,float y,float size,Colour col){
 Path p;p.startNewSubPath(x,y);p.quadraticTo(x-size*.9f,y-size*.8f,x,y-size*1.8f);
 p.quadraticTo(x+size*.9f,y-size*.8f,x,y);p.closeSubPath();
 g.setColour(col);g.fillPath(p);
 g.setColour(ochre.withAlpha(.23f));g.drawLine(x,y,x,y-size*1.5f,.7f);
}
inline void mushroom(Graphics& g,float x,float y,float s,Colour colour){
 g.setColour(ash.withAlpha(.65f));g.fillRoundedRectangle(x-3.f*s,y-14.f*s,6.f*s,15.f*s,2.f*s);
 Path cap;cap.startNewSubPath(x-12.f*s,y-12.f*s);
 cap.cubicTo(x-13.f*s,y-30.f*s,x+13.f*s,y-30.f*s,x+12.f*s,y-12.f*s);
 cap.quadraticTo(x,y-7.f*s,x-12.f*s,y-12.f*s);cap.closeSubPath();
 g.setColour(colour);g.fillPath(cap);
 g.setColour(ochre.withAlpha(.54f));g.strokePath(cap,PathStrokeType(.9f*s));
 for(int i=0;i<3;++i) {
  float xx=x+(i-1)*5.f*s;
  g.setColour(ivory.withAlpha(.4f));g.fillEllipse(xx-1.3f*s,y-(19.f+i%2*3.f)*s,2.7f*s,2.2f*s);
 }
}
inline void creature(Graphics& g,float x,float y,float s){
 Path body;body.startNewSubPath(x,y+24*s);
 body.cubicTo(x-32*s,y+19*s,x-34*s,y-13*s,x-17*s,y-24*s);
 body.cubicTo(x-9*s,y-45*s,x+9*s,y-45*s,x+17*s,y-24*s);
 body.cubicTo(x+34*s,y-13*s,x+32*s,y+19*s,x,y+24*s);body.closeSubPath();
 g.setColour(moss.withAlpha(.32f));g.fillPath(body);
 g.setColour(ochre.withAlpha(.53f));g.strokePath(body,PathStrokeType(2.f*s));
 for(int side=-1;side<=1;side+=2){
  Path horn;horn.startNewSubPath(x+side*18*s,y-19*s);
  horn.cubicTo(x+side*22*s,y-39*s,x+side*39*s,y-35*s,x+side*45*s,y-49*s);
  g.setColour(clay.withAlpha(.67f));g.strokePath(horn,PathStrokeType(2.f*s));
  g.setColour(ochre);g.fillEllipse(x+(side*12-4)*s,y-13*s,8*s,7*s);
  g.setColour(soil);g.fillEllipse(x+(side*12-1)*s,y-12*s,3*s,4*s);
 }
 spiral(g,x,y+8*s,11*s,ochre.withAlpha(.8f),1.8f*s);
}
inline void forestBackdrop(Graphics& g){
 ColourGradient bg(Colour::fromRGB(32,39,27),0.f,0.f,soil,1030.f,790.f,false);
 g.setGradientFill(bg);g.fillRect(0,0,1160,790);
 // Hand-drawn crayon-like topographical lines instead of distracting animated noise.
 for(int n=0;n<18;++n) {
  float y=37.f+n*41.f;
  Path p;p.startNewSubPath(0.f,y);
  p.cubicTo(240.f,y-23.f,680.f,y+23.f,1160.f,y-12.f);
  g.setColour((n%3==0?moss:ochre).withAlpha(.029f));
  g.strokePath(p,PathStrokeType((n%3==0)?1.4f:.8f));
 }
 for(int side=0;side<2;++side){
  float x=side==0?8.f:1152.f,sgn=side==0?1.f:-1.f;
  for(int n=0;n<4;++n){
   float yy=230.f+n*146.f;
   Path trunk;trunk.startNewSubPath(x,yy+48.f);
   trunk.cubicTo(x+sgn*22.f,yy+27.f,x+sgn*8.f,yy-26.f,x+sgn*(18.f+n*2.f),yy-50.f);
   g.setColour(outline.withAlpha(.23f));g.strokePath(trunk,PathStrokeType(3.3f));
   leaf(g,x+sgn*22.f,yy-25.f,8.f,moss.withAlpha(.22f));
  }
 }
 // Forest-dwelling life around the bottom edge, never on top of controls.
 mushroom(g,29.f,676.f,1.35f,clay.withAlpha(.7f));
 mushroom(g,49.f,671.f,.74f,ochre.withAlpha(.66f));
 mushroom(g,1124.f,678.f,1.3f,clay.withAlpha(.7f));
 mushroom(g,1103.f,673.f,.8f,moss.withAlpha(.64f));
}
inline void panel(Graphics& g,Rectangle<float> r,Colour colour,int index){
 ColourGradient fill(bark.brighter(.11f),r.getX(),r.getY(),soil.brighter(.14f),r.getRight(),r.getBottom(),false);
 g.setGradientFill(fill);g.fillRoundedRectangle(r,13.f);
 g.setColour(outline.withAlpha(.78f));g.drawRoundedRectangle(r,13.f,1.3f);
 // Clay strokes are never perfectly straight. Tiny painted nodes denote each section.
 g.setColour(colour.withAlpha(.84f));
 g.fillEllipse(r.getX()+12.f,r.getY()+14.f,5.5f,5.5f);
 g.drawLine(r.getX()+24.f,r.getY()+16.8f,r.getX()+55.f,r.getY()+16.8f,1.2f);
 g.setColour(ochre.withAlpha(.17f));
 g.drawLine(r.getX()+13.f,r.getBottom()-10.f,r.getRight()-13.f,r.getBottom()-10.f,.8f);
 for(int n=0;n<2;++n){
  float x=r.getRight()-29.f+n*11.f,y=r.getY()+42.f;
  leaf(g,x,y,4.1f,colour.withAlpha(.36f));
 }
 if(index%3==0)spiral(g,r.getRight()-16.f,r.getBottom()-19.f,8.f,colour.withAlpha(.23f),1.f);
}
inline void rotary(Graphics& g,int x,int y,int w,int h,float value,float start,float end,Colour tint){
 const float rad=std::max(9.f,std::min((float)w*.27f,(float)h*.30f));
 const float cx=x+(float)w*.5f,cy=y+(float)h*.43f;
 Path rim;rim.addCentredArc(cx,cy,rad,rad,0,start,end,true);
 g.setColour(soil.brighter(.43f));g.strokePath(rim,PathStrokeType(6.f,PathStrokeType::curved,PathStrokeType::rounded));
 g.setColour(outline);g.strokePath(rim,PathStrokeType(3.7f,PathStrokeType::curved,PathStrokeType::rounded));
 float a=start+(end-start)*value;
 Path p;p.addCentredArc(cx,cy,rad,rad,0,start,a,true);
 g.setColour(tint.brighter(.21f));g.strokePath(p,PathStrokeType(4.f,PathStrokeType::curved,PathStrokeType::rounded));
 ColourGradient grad(stone.brighter(.33f),cx-rad,cy-rad,bark.darker(.22f),cx+rad,cy+rad,false);
 g.setGradientFill(grad);g.fillEllipse(cx-rad*.78f,cy-rad*.78f,rad*1.56f,rad*1.56f);
 g.setColour(ochre.withAlpha(.3f));g.drawEllipse(cx-rad*.77f,cy-rad*.75f,rad*1.54f,rad*1.5f,1.1f);
 for(int i=0;i<5;++i){
  float aa=(i*.85f+.24f)*MathConstants<float>::pi;
  g.setColour((i%2?tint:ash).withAlpha(.42f));
  g.fillEllipse(cx+std::cos(aa)*rad*.49f-1.1f,cy+std::sin(aa)*rad*.42f-1.1f,2.2f,2.2f);
 }
 a-=MathConstants<float>::halfPi;
 g.setColour(tint.brighter(.48f));
 g.drawLine(cx,cy,cx+std::cos(a)*rad*.66f,cy+std::sin(a)*rad*.66f,2.3f);
 g.setColour(ivory.withAlpha(.76f));g.fillEllipse(cx-1.6f,cy-1.6f,3.2f,3.2f);
}
inline void button(Graphics& g,Button& b,bool hovered,bool pressed,Colour tint){
 auto r=b.getLocalBounds().toFloat().reduced(1.5f);
 const bool active=b.getToggleState();
 g.setColour(active?tint.darker(.46f):pressed?stone.brighter(.15f):hovered?stone:bark);
 g.fillRoundedRectangle(r,9.f);
 g.setColour(active?tint.brighter(.3f):outline);
 g.drawRoundedRectangle(r,9.f,active?1.6f:1.0f);
 g.setColour(tint.withAlpha(.68f));g.fillEllipse(r.getX()+7.f,r.getCentreY()-2.5f,5.f,5.f);
 g.setColour(ochre.withAlpha(.18f));
 g.drawLine(r.getX()+18.f,r.getBottom()-5.f,r.getRight()-6.f,r.getBottom()-5.f,.8f);
}
inline void selector(Graphics& g,int w,int h){
 auto r=Rectangle<float>(1.f,1.f,(float)w-2.f,(float)h-2.f);
 g.setColour(soil.brighter(.2f));g.fillRoundedRectangle(r,6.f);
 g.setColour(outline.withAlpha(.83f));g.drawRoundedRectangle(r,6.f,1.f);
 Path p;float x=(float)w-15.f,y=(float)h*.53f;
 p.startNewSubPath(x-3.5f,y-1.f);p.lineTo(x,y+2.5f);p.lineTo(x+3.5f,y-1.f);
 g.setColour(ochre);g.strokePath(p,PathStrokeType(1.5f));
}
}
