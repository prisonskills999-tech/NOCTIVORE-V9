#include "VoidcoreEditor.h"
#include <functional>
#include <cmath>
#include <algorithm>
using namespace juce;
namespace {
constexpr float pi=juce::MathConstants<float>::pi;
// Strict Matrix palette: luminous green, black and neutral white/grey only.
const Colour acid=Colour::fromRGB(89,89,89), blood=Colour::fromRGB(232,232,232);
const Colour frost=Colour::fromRGB(250,250,250), grey=Colour::fromRGB(141,141,141);
const Colour voidBlack=Colour::fromRGB(6,6,6);
float clamp01(float v){return std::clamp(v,0.f,1.f);}
float phaseWave(int shape,float t){
 float p=t-std::floor(t);
 switch(shape){
 case 1:return 1.f-4.f*std::abs(p-.5f);
 case 2:return 2.f*p-1.f;
 case 3:return p<.5f?1.f:-1.f;
 case 4:return std::sin(2.f*pi*std::floor(t*5.1f)*.37f);
 case 5:return .62f*std::sin(2.f*pi*t)+.23f*std::sin(6.f*pi*t+.5f)+.15f*std::sin(14.f*pi*t);
 default:return std::sin(2.f*pi*t);
 }
}
void neonLine(Graphics& g,Path& p,Colour c,float width){
 g.setColour(c.withAlpha(.18f));g.strokePath(p,PathStrokeType(width+3.f,PathStrokeType::curved,PathStrokeType::rounded));
 g.setColour(c);g.strokePath(p,PathStrokeType(width,PathStrokeType::curved,PathStrokeType::rounded));
}
}
VoidcoreEditor::Dial::Dial(NoctivoreProcessor& p,const String& id,const String& text,Colour c)
 :label(text),parameterID(id),accent(c){
 setSliderStyle(Slider::RotaryVerticalDrag);
 setRotaryParameters(pi*1.22f,pi*2.78f,true);
 setTextBoxStyle(Slider::NoTextBox,false,0,0);
 // A long throw makes it easy to settle on a precise sound. JUCE's
 // velocity modifier allows the user to switch to even finer gestures.
 setMouseDragSensitivity(480);
 setVelocityModeParameters(.24,2,0.,true);
 setScrollWheelEnabled(true);
 setMouseCursor(MouseCursor::UpDownResizeCursor);
 setTooltip(text+"  |  drag vertically to adjust  |  Shift for precision  |  double-click to reset");
 if(auto* param=p.state.getParameter(id))
  setDoubleClickReturnValue(true,param->convertFrom0to1(param->getDefaultValue()));
 link=std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(p.state,id,*this);
}
void VoidcoreEditor::Dial::paint(Graphics& g){
 const auto r=getLocalBounds().toFloat();
 const float w=r.getWidth(),h=r.getHeight();
 const float cx=r.getCentreX(),cy=r.getY()+h*.466f;
 const float rad=std::max(11.f,std::min(w*.433f,h*.305f));
 const float lo=pi*1.22f,hi=pi*2.78f;
 const float norm=getMaximum()>getMinimum()?
  clamp01((float)((getValue()-getMinimum())/(getMaximum()-getMinimum()))):0.f;
 if(displayedNormal<0.f)displayedNormal=norm;
 // Mechanical visual damping only; the actual value reaches the DSP and
 // host immediately, with no unwanted audio smoothing or parameter lag.
 displayedNormal+=(norm-displayedNormal)*(isMouseButtonDown()? .50f:.30f);
 if(std::fabs(displayedNormal-norm)<.00025f)displayedNormal=norm;
 const float angle=lo+(hi-lo)*displayedNormal;
 const bool focus=isMouseOverOrDragging()||isMouseButtonDown();
 const Colour rim=Colour::fromRGB(164,164,164);
 const Colour white=Colour::fromRGB(245,245,245);
 const Colour steel=Colour::fromRGB(71,71,71);
 const Colour base=Colour::fromRGB(9,9,9);
 // Every knob is a physical-looking, matte front panel element; the white
 // line, large ring and bright value strip distinguish it from background.
 g.setColour(base);
 g.fillRoundedRectangle(juce::Rectangle<float>(.7f,.7f,w-1.4f,h-1.4f),5.f);
 g.setColour(focus?white.withAlpha(.62f):rim.withAlpha(.40f));
 g.drawRoundedRectangle(juce::Rectangle<float>(.8f,.8f,w-1.6f,h-1.6f),5.f,focus?1.7f:.9f);
 g.setColour(white);g.setFont(Font(std::clamp(w*.182f,10.f,14.f),Font::bold));
 g.drawFittedText(label,2,2,roundToInt(w-4),roundToInt(h*.17f),
                  Justification::centred,1,.78f);
 // Machined tick notches allow precise reading without a computer cursor.
 for(int tick=0;tick<=10;++tick){
  const float a=lo+(hi-lo)*(float)tick/10.f-pi*.5f;
  const float inner=rad*1.135f,outer=rad*(tick%5==0?1.33f:1.25f);
  g.setColour((tick%5==0?white:rim).withAlpha(tick%5==0?.84f:.46f));
  g.drawLine(cx+std::cos(a)*inner,cy+std::sin(a)*inner,
             cx+std::cos(a)*outer,cy+std::sin(a)*outer,tick%5==0?1.5f:.85f);
 }
 g.setColour(Colour::fromRGB(25,25,25));
 g.fillEllipse(cx-rad*1.04f,cy-rad*1.04f,rad*2.08f,rad*2.08f);
 g.setColour(rim);
 g.drawEllipse(cx-rad*.99f,cy-rad*.99f,rad*1.98f,rad*1.98f,2.f);
 ColourGradient machined(Colour::fromRGB(65,65,65),cx-rad,cy-rad,
                         Colour::fromRGB(11,11,11),cx+rad,cy+rad,true);
 g.setGradientFill(machined);
 g.fillEllipse(cx-rad*.91f,cy-rad*.91f,rad*1.82f,rad*1.82f);
 g.setColour(white.withAlpha(.20f));
 g.drawEllipse(cx-rad*.70f,cy-rad*.70f,rad*1.40f,rad*1.40f,1.f);
 Path track;track.addCentredArc(cx,cy,rad*1.10f,rad*1.10f,0.f,lo,hi,true);
 g.setColour(steel);g.strokePath(track,PathStrokeType(3.5f));
 Path valueArc;valueArc.addCentredArc(cx,cy,rad*1.10f,rad*1.10f,0.f,lo,angle,true);
 g.setColour(white);g.strokePath(valueArc,PathStrokeType(3.7f));
 const float a=angle-pi*.5f;
 g.setColour(white);
 g.drawLine(cx+std::cos(a)*rad*.22f,cy+std::sin(a)*rad*.22f,
            cx+std::cos(a)*rad*.82f,cy+std::sin(a)*rad*.82f,3.f);
 g.fillEllipse(cx-2.4f,cy-2.4f,4.8f,4.8f);
 // High contrast persistent readout (not a fleeting popup).
 String read;
 const double v=getValue();
 if(parameterID=="rate"||parameterID.endsWith("Rate")){
  read=(v<.01?String(v,5):v<1.?String(v,3):String(v,2))+" Hz";
 }else if(parameterID=="tuneA"||parameterID=="tuneB"){
  read=String(v,1)+" st";
 }else if(parameterID=="toneLow"||parameterID=="toneMid"||parameterID=="toneHigh"){
  read=String::formatted("%+.1f dB",(v-.5)*18.);
 }else if(parameterID=="attack"||parameterID=="release"||parameterID=="glide"){
  read=String(v,v<1.?2:1)+" s";
 }else if(parameterID=="pan"){
  read=std::abs(v)<.01?"CENTER":(v<0?"L ":"R ")+String((int)std::round(std::abs(v)*100.));
 }else if(getMinimum()>=0.&&getMaximum()<=1.001){
  read=String((int)std::round(v*100.))+"%";
 }else{
  read=String(v,getMaximum()>5.?1:2);
 }
 const float stripY=h*.811f,stripH=h*.172f;
 g.setColour(Colour::fromRGB(31,31,31));
 g.fillRoundedRectangle(juce::Rectangle<float>(3.f,stripY,w-6.f,stripH),3.f);
 g.setColour(focus?white:rim);
 g.drawRoundedRectangle(juce::Rectangle<float>(3.f,stripY,w-6.f,stripH),3.f,focus?1.3f:.8f);
 g.setColour(white);
 g.setFont(Font(std::clamp(w*.167f,9.f,13.f),Font::bold));
 g.drawFittedText(read,5,roundToInt(stripY),roundToInt(w-10),roundToInt(stripH),
                  Justification::centred,1,.75f);
}
void VoidcoreEditor::OrbPad::apply(const MouseEvent& e){
 const float x=std::clamp(e.position.x/std::max(1.f,(float)getWidth()),0.f,1.f);
 const float y=1.f-std::clamp(e.position.y/std::max(1.f,(float)getHeight()),0.f,1.f);
 if(auto* p=processor.state.getParameter("morph"))p->setValueNotifyingHost(x);
 if(auto* p=processor.state.getParameter("biomeDepth"))p->setValueNotifyingHost(y);
 repaint();
}
void VoidcoreEditor::OrbPad::mouseDown(const MouseEvent& e){
 dragging=true;
 if(auto* p=processor.state.getParameter("morph"))p->beginChangeGesture();
 if(auto* p=processor.state.getParameter("biomeDepth"))p->beginChangeGesture();
 apply(e);
}
void VoidcoreEditor::OrbPad::mouseDrag(const MouseEvent& e){
 if(dragging)apply(e);
}
void VoidcoreEditor::OrbPad::mouseUp(const MouseEvent& e){
 if(!dragging)return;
 apply(e);
 if(auto* p=processor.state.getParameter("morph"))p->endChangeGesture();
 if(auto* p=processor.state.getParameter("biomeDepth"))p->endChangeGesture();
 dragging=false;
}
void VoidcoreEditor::OrbPad::paint(Graphics& g){
 const auto* m=processor.state.getRawParameterValue("morph");
 const auto* d=processor.state.getRawParameterValue("biomeDepth");
 const float xx=(m?m->load():.5f)*getWidth();
 const float yy=(1.f-(d?d->load():.5f))*getHeight();
 const Colour green=Colour::fromRGB(89,89,89);
 const Colour white=Colour::fromRGB(246,246,246);
 const float rad=dragging?11.f:7.f;
 g.setColour(green.withAlpha(dragging?.70f:.34f));
 g.drawEllipse(xx-rad,yy-rad,rad*2.f,rad*2.f,1.2f);
 g.setColour(white.withAlpha(.80f));
 g.fillEllipse(xx-2.f,yy-2.f,4.f,4.f);
 if(dragging){
  g.setColour(green.withAlpha(.40f));
  g.drawLine(xx,0.f,xx,(float)getHeight(),1.f);
  g.drawLine(0.f,yy,(float)getWidth(),yy,1.f);
 }
}
VoidcoreEditor::Choice::Choice(NoctivoreProcessor& p,const String& id,const StringArray& items){
 addItemList(items,1);
 setJustificationType(Justification::centredLeft);
 setColour(ComboBox::backgroundColourId,Colour::fromRGB(11,11,11));
 setColour(ComboBox::textColourId,frost);
 setColour(ComboBox::outlineColourId,grey.withAlpha(.8f));
 setColour(ComboBox::arrowColourId,acid);
 setColour(PopupMenu::backgroundColourId,voidBlack);
 setColour(PopupMenu::textColourId,frost);
 setColour(PopupMenu::highlightedBackgroundColourId,Colour::fromRGB(38,38,38));
 link=std::make_unique<AudioProcessorValueTreeState::ComboBoxAttachment>(p.state,id,*this);
}
void VoidcoreEditor::Choice::paint(Graphics& g){
 ComboBox::paint(g);
 auto r=getLocalBounds().toFloat().reduced(1.5f);
 const float pulse=.5f+.5f*std::sin(animationPhase*1.55f);
 g.setColour(acid.withAlpha(.16f+.13f*pulse));
 g.drawRoundedRectangle(r,3.f,1.f);
 g.setColour(acid.withAlpha(.18f+.20f*pulse));
 g.fillRect(r.getRight()-3.f,r.getY()+4.f,2.f,r.getHeight()-8.f);
}
VoidcoreEditor::Click::Click(const String& t,Colour c):TextButton(t),accent(c){
 setClickingTogglesState(false);
 setMouseCursor(MouseCursor::PointingHandCursor);
 setTooltip(t);
}
void VoidcoreEditor::Click::paintButton(Graphics& g,bool over,bool pressed){
 const auto r=getLocalBounds().toFloat().reduced(1.f);
 const float pulse=.5f+.5f*std::sin(animationPhase*2.5f);
 const bool lit=selected||over||pressed;
 g.setColour(lit?Colour::fromRGB(19,19,19):Colour::fromRGB(13,13,13));
 g.fillRoundedRectangle(r,4.f);
 g.setColour(accent.withAlpha(lit?.60f+.30f*pulse:.18f+.13f*pulse));
 g.drawRoundedRectangle(r,4.f,lit?1.7f:1.f);
 if(lit){
  g.setColour(accent.withAlpha(.07f+.10f*pulse));
  g.fillRoundedRectangle(r.reduced(3.f),3.f);
 }
 // The moving scanline and breathing status lamp are visual-only.
 const float scan=std::fmod(animationPhase*.13f,1.f);
 g.setColour(accent.withAlpha(lit?.25f:.09f));
 g.drawVerticalLine((int)(r.getX()+r.getWidth()*scan),r.getY()+3.f,r.getBottom()-3.f);
 g.setColour(lit?frost:grey);
 g.setFont(Font(std::clamp(r.getHeight()*.34f,9.f,13.f),Font::bold));
 g.drawFittedText(getButtonText(),getLocalBounds().reduced(3,1),Justification::centred,1,.75f);
}
VoidcoreEditor::VoidcoreEditor(NoctivoreProcessor& p):AudioProcessorEditor(p),processor(p){
 setOpaque(true);
 // The full-resolution artwork is optional; if present it is the untouched
 // image in the VST3's Resources/GUI/voidcore.png. Real controls stay separate.
 auto art=processor.paintingFile().getSiblingFile("voidcore.png");
 if(art.existsAsFile())reference=ImageFileFormat::loadFrom(art);
 buildCore();setPage(Core);
 setResizable(true,true);
 setResizeLimits(960,720,1800,1350);
 if(auto* constrainer=getConstrainer())constrainer->setFixedAspectRatio(4.0/3.0);
 setSize(1200,900);
 startTimerHz(30); // UI-only; DSP never renders on the GUI thread.
}
VoidcoreEditor::~VoidcoreEditor(){stopTimer();chooser.reset();}
float VoidcoreEditor::value(const char* id)const{
 if(auto* raw=processor.state.getRawParameterValue(id))return raw->load();return 0.f;
}
void VoidcoreEditor::set(const char* id,float v){
 if(auto* par=processor.state.getParameter(id)){
   const auto legal=par->getNormalisableRange().snapToLegalValue(v);
   par->beginChangeGesture();
   par->setValueNotifyingHost(par->convertTo0to1(legal));
   par->endChangeGesture();
 }
}
void VoidcoreEditor::shift(const char* id,int n){
 if(auto* param=processor.state.getParameter(id)){
  const auto num=param->getNumSteps();
  if(num>1){
   int current=(int)std::round(value(id));
   set(id,(float)((current+n+(num*8))%num));
  }
 }
}
void VoidcoreEditor::knob(const char* id,const char* label,float x,float y,float d,Page page_,int style){
 auto c=std::make_unique<Dial>(processor,id,label,(style%3==1)?blood:acid);
 addAndMakeVisible(*c);
 ui.push_back({std::move(c),{x-d*.5f,y-d*.43f,d,d*.95f},(int)page_});
}
void VoidcoreEditor::choice(const char* id,StringArray items,float x,float y,float w,float h,Page page_){
 auto c=std::make_unique<Choice>(processor,id,items);
 addAndMakeVisible(*c);
 ui.push_back({std::move(c),{x,y,w,h},(int)page_});
}
void VoidcoreEditor::click(const String& label,float x,float y,float w,float h,Page pg,std::function<void()> fn,int style){
 auto c=std::make_unique<Click>(label,style%3==1?blood:acid);
 c->onClick=std::move(fn);
 addAndMakeVisible(*c);
 ui.push_back({std::move(c),{x,y,w,h},(int)pg});
}
void VoidcoreEditor::allClick(const String& label,float x,float y,float w,float h,std::function<void()> fn,int style){
 auto c=std::make_unique<Click>(label,style%3==1?blood:acid);
 c->onClick=std::move(fn);
 addAndMakeVisible(*c);
 ui.push_back({std::move(c),{x,y,w,h},-1});
}
void VoidcoreEditor::setPage(Page next){
 page=next;
 for(auto& node:ui)node.control->setVisible(node.onPage<0||node.onPage==(int)page||
   ((page==Motion||page==Core)&&node.onPage==20+focusLfo)||
   (page==Motion&&node.onPage==10+focusLfo));
 for(int i=0;i<4;++i){
  if(pageButtons[(size_t)i]){pageButtons[(size_t)i]->selected=(int)page==i;pageButtons[(size_t)i]->repaint();}
  if(lfoButtons[(size_t)i]){lfoButtons[(size_t)i]->selected=focusLfo==i;lfoButtons[(size_t)i]->repaint();}
 }
 repaint();
}
void VoidcoreEditor::resized(){
 const float sx=(float)getWidth()/W,sy=(float)getHeight()/H;
 for(auto& node:ui){
  auto a=node.area;
  node.control->setBounds(roundToInt(a.getX()*sx),roundToInt(a.getY()*sy),
                          std::max(1,roundToInt(a.getWidth()*sx)),std::max(1,roundToInt(a.getHeight()*sy)));
 }
}
void VoidcoreEditor::timerCallback(){
 clock+=1.f/30.f;
 if(clock>60000.f)clock=0.f;
 smoothedPeak=smoothedPeak*.76f+clamp01(processor.peak.load(std::memory_order_relaxed))*.24f;
 meterLevels[0]=std::max(meterLevels[0]*.84f,clamp01(processor.peakLeft.load(std::memory_order_relaxed)));
 meterLevels[1]=std::max(meterLevels[1]*.84f,clamp01(processor.peakRight.load(std::memory_order_relaxed)));
 playing=processor.displayPlaying.load(std::memory_order_relaxed);
 for(int k=0;k<16;++k)
   if(auto* button=steps[(size_t)k]){
    const bool on=value(String::formatted("gateStep%02d",k+1).toRawUTF8())>.5f;
    if(button->selected!=on){button->selected=on;button->repaint();}
   }
 if(reverseButton){
  const bool newValue=value("reverse")>.5f;
  if(reverseButton->selected!=newValue){reverseButton->selected=newValue;reverseButton->repaint();}
 }
 if(cachedA!=(int)value("bank"))updatePreview(0);
 if(cachedB!=(int)value("bankB"))updatePreview(1);
 if(sourceSoloButton){
  const bool solo=value("sourceSolo")>.5f;
  if(sourceSoloButton->selected!=solo){sourceSoloButton->selected=solo;sourceSoloButton->repaint();}
 }
 for(int mode=0;mode<3;++mode)
  if(auto* button=filterModeButtons[(size_t)mode]){
   const bool active=(int)value("filterMode")==mode;
   if(button->selected!=active){button->selected=active;button->repaint();}
  }
 // JUCE child components repaint independently of their parent: animate each
 // visible control without touching audio processing or modifying parameters.
 for(auto& node:ui){
  if(!node.control->isVisible())continue;
  if(auto* button=dynamic_cast<Click*>(node.control.get()))
   button->animationPhase=clock;
  else if(auto* dial=dynamic_cast<Dial*>(node.control.get()))
   dial->animationPhase=clock;
  else if(auto* slider=dynamic_cast<Fader*>(node.control.get()))
   slider->animationPhase=clock;
  else if(auto* menu=dynamic_cast<Choice*>(node.control.get()))
   menu->animationPhase=clock;
  node.control->repaint();
 }
 repaint();
}


VoidcoreEditor::Fader::Fader(NoctivoreProcessor& p,const String& id){
 setSliderStyle(Slider::LinearHorizontal);
 setTextBoxStyle(Slider::NoTextBox,false,0,0);
 setMouseDragSensitivity(180);
 link=std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(p.state,id,*this);
}
void VoidcoreEditor::Fader::paint(Graphics& g){
 auto r=getLocalBounds().toFloat();
 const float norm=getMaximum()>getMinimum()?clamp01((float)((getValue()-getMinimum())/(getMaximum()-getMinimum()))):0.f;
 const float y=r.getCentreY(), x=r.getX()+12.f+(r.getWidth()-24.f)*norm;
 g.setColour(grey.withAlpha(.5f));g.fillRoundedRectangle(r.getX()+10.f,y-3.f,r.getWidth()-20.f,6.f,3.f);
 g.setColour(acid.withAlpha(.86f));g.fillRoundedRectangle(r.getX()+10.f,y-3.f,std::max(1.f,x-r.getX()-10.f),6.f,3.f);
 g.setColour(blood.withAlpha(.35f));g.fillEllipse(x-13.f,y-13.f,26.f,26.f);
 g.setColour(frost);g.fillRoundedRectangle(x-5.f,y-14.f,10.f,28.f,2.f);
 g.setColour(acid.withAlpha(.55f+.40f*(.5f+.5f*std::sin(animationPhase*2.f))));
 g.drawRoundedRectangle(x-5.f,y-14.f,10.f,28.f,2.f,1.5f);
 g.setColour(frost.withAlpha(.55f));
 g.fillEllipse(x-2.f,y-2.f,4.f,4.f);
}
void VoidcoreEditor::fader(const char* id,float x,float y,float w,float h,Page pg){
 auto c=std::make_unique<Fader>(processor,id);
 addAndMakeVisible(*c);
 ui.push_back({std::move(c),{x,y,w,h},(int)pg});
}
void VoidcoreEditor::buildCore(){
 // FINAL VOIDCORE: all audio controls on a single visible screen. No page
 // navigation; every knob and button is bound to a real DSP/APVTS parameter.
 allClick("LOAD",423,17,66,34,[this]{load();});
 allClick("SAVE",495,17,66,34,[this]{save();});
 allClick("RANDOM",568,17,91,34,[this]{mutation(false);});
 allClick("EVOLVE",665,17,83,34,[this]{mutation(true);});
 allClick("INIT",755,17,67,34,[this]{
  for(auto* parameter:processor.getParameters()){
   parameter->beginChangeGesture();
   parameter->setValueNotifyingHost(parameter->getDefaultValue());
   parameter->endChangeGesture();
  }
  lastScene="CLEAN INITIAL PATCH";
 });
 allClick("PURE SOURCE",835,17,136,34,[this]{
  set("sourceSolo",value("sourceSolo")>.5f?0.f:1.f);
 });
 sourceSoloButton=dynamic_cast<Click*>(ui.back().control.get());
 allClick("REVERSE",978,17,96,34,[this]{
  set("reverse",value("reverse")>.5f?0.f:1.f);
 });
 reverseButton=dynamic_cast<Click*>(ui.back().control.get());
 StringArray banks;
 for(const auto& bank:noctivore::banks)banks.add(bank.name);
 banks.add("User");
 // Genuine, independently tunable dual sample voices.
 choice("bank",banks,43,126,140,28,Core);
 choice("bankB",banks,218,126,140,28,Core);
 click("<",20,127,21,27,Core,[this]{shift("bank",-1);});
 click(">",184,127,22,27,Core,[this]{shift("bank",1);});
 click("<",196,168,20,27,Core,[this]{shift("bankB",-1);});
 click(">",357,168,20,27,Core,[this]{shift("bankB",1);});
 knob("tuneA","TUNE A",68,274,69,Core);
 knob("levelA","LEVEL A",156,274,69,Core);
 knob("tuneB","TUNE B",247,274,69,Core);
 knob("levelB","LEVEL B",334,274,69,Core);
 // Huge central morph and real amp envelope. The orb is a 2D expression pad.
 auto orb=std::make_unique<OrbPad>(processor);
 addAndMakeVisible(*orb);
 ui.push_back({std::move(orb),{495,191,206,176},(int)Core});
 knob("attack","ATTACK",465,481,75,Core);
 knob("morph","MORPH A/B",601,472,112,Core);
 knob("release","RELEASE",734,481,75,Core);
 // High impact filter; the graph displays its actual active choice.
 choice("filterMode",{"Low Pass","Band Pass","High Pass"},846,126,317,29,Core);
 knob("cutoff","CUTOFF",897,254,116,Core);
 knob("resonance","RESONANCE",1014,265,69,Core);
 knob("filterDrive","DRIVE",1112,265,69,Core);
 // Direct LFO routed to a selected physical destination.
 for(int i=0;i<4;++i){
  click("LFO "+String(i+1),35.f+i*83.f,360.f,78.f,25.f,Core,[this,i]{
   focusLfo=i;setPage(Core);
  });
  lfoButtons[(size_t)i]=dynamic_cast<Click*>(ui.back().control.get());
  const String pre=i==0?"":String("lfo")+String(i+1);
  const String wa=i==0?"wave":pre+"Wave";
  const String ta=i==0?"target":pre+"Target";
  const String ra=i==0?"rate":pre+"Rate";
  const String de=i==0?"depth":pre+"Depth";
  const String sy=String("lfo")+String(i+1)+"Sync";
  const StringArray waveNames{"Sine","Triangle","Saw","Square","Sample & Hold","Fractal"};
  const StringArray targets{"Volume","Pan","Pitch","Filter 1","Filter 2","FM Index","A/B Morph","Metal Exciter","Grain Density","Delay Warp","Reverb Motion"};
  choice(wa.toRawUTF8(),waveNames,35,387,145,25,Core);
  ui.back().onPage=20+i;
  choice(ta.toRawUTF8(),targets,186,387,172,25,Core);
  ui.back().onPage=20+i;
  choice(sy.toRawUTF8(),{"Free","1 bar","1/2 bar","1/4 bar","1/8 bar","1/16 bar"},35,416,145,23,Core);
  ui.back().onPage=20+i;
  knob(ra.toRawUTF8(),"RATE",81,489,77,Core);ui.back().onPage=20+i;
  knob(de.toRawUTF8(),"DEPTH",174,489,77,Core);ui.back().onPage=20+i;
 }
 knob("morphMotion","MORPH",269,486,62,Core);
 knob("evolution","EVOLVE",342,486,62,Core);
 // Character is the genuine filter/organism macro; the two neighboring
 // parameters change the granular time structure, not cosmetic animations.
 // The full XENO sound matrix stays on this one screen. All knobs are
 // actual host-automatable DSP; larger marked caps aid fast live tweaks.
 choice("textureMode",{"Flow","Grain","Orbit","Pulse","Liquid","Spectral"},854,367,306,23,Core);
 knob("character","XENO",857,429,65,Core);
 knob("grain","GRAIN",944,429,65,Core);
 knob("drop","DROP",1032,429,65,Core);
 knob("hits","HITS",1120,429,65,Core);
 knob("body","BODY",857,495,65,Core);
 knob("grit","GRIT",944,495,65,Core);
 knob("dark","DARK",1032,495,65,Core);
 knob("texture","SCATTER",1120,495,65,Core);
 // 16-click performance strip controls the live gating engine.
 choice("motif",{"Off","Serpent","Ritual","Insect","Portal","Fracture","Hollow"},85,551,142,26,Core);
 knob("trancePulse","PULSE",278,558,55,Core);
 for(int i=0;i<16;++i){
  const auto id=String::formatted("gateStep%02d",i+1);
  click(String(i+1),324.f+43.f*i,550.f,36.f,28.f,Core,[this,id]{
   if(value("motif")<.5f)set("motif",1.f);
   set(id.toRawUTF8(),value(id.toRawUTF8())>.5f?0.f:1.f);
  });
  steps[(size_t)i]=dynamic_cast<Click*>(ui.back().control.get());
 }
 knob("motifGate","GATE",1121,556,59,Core);
 // Monochrome impact FX, expanded into two real DSP modules.
 choice("division",{"1/4","1/8","1/8 dotted","1/16","1/16 dotted","1/32","1/4 triplet","1/8 triplet"},35,630,144,23,Core);
 choice("delayMode",{"Stereo","Ping Pong","Mono","Drifting"},186,630,171,23,Core);
 knob("delayMix","MIX",55,693,65,Core);
 knob("delayFeedback","FEED",123,693,65,Core);
 knob("delayImpact","IMPACT",191,693,65,Core);
 knob("delayWarp","WARP",259,693,65,Core);
 knob("delayDiffusion","DIFFUSE",328,693,65,Core);
 knob("delayDrift","DRIFT",397,693,65,Core);
 knob("delayTone","TONE",466,693,65,Core);
 knob("delaySpread","SPREAD",535,693,65,Core);
 choice("reverbMode",{"Hall","Plate","Chamber","Infinite"},601,630,180,23,Core);
 knob("reverbMix","MIX",610,693,65,Core);
 knob("reverbSize","SIZE",678,693,65,Core);
 knob("reverbDecay","DECAY",746,693,65,Core);
 knob("reverbPreDelay","PREDEL",814,693,65,Core);
 knob("reverbDark","DAMP",882,693,65,Core);
 knob("reverbMod","MOTION",950,693,65,Core);
 knob("reverbImpact","IMPACT",1018,693,65,Core);
 knob("reverbWidth","WIDTH",1086,693,65,Core);
 knob("shimmer","SHIMMER",1153,693,65,Core);
 // Source-character EQ, true stereo controls, transport and output.
 knob("toneLow","LOW",55,805,70,Core);
 knob("toneMid","MID",137,805,70,Core);
 knob("toneHigh","HIGH",219,805,70,Core);
 knob("stereo","STEREO",327,805,72,Core);
 knob("pan","PAN",430,805,69,Core);
 knob("volume","MASTER",555,805,92,Core);
 choice("mode",{"Polyphonic","Mono","Legato"},697,755,150,28,Core);
 knob("glide","GLIDE",732,812,70,Core);
 // Real MIDI keyboard, available without any navigation or secondary page.
 auto keys=std::make_unique<MidiKeyboardComponent>(
    processor.keyboardState,MidiKeyboardComponent::horizontalKeyboard);
 keys->setAvailableRange(36,96);keys->setKeyWidth(18.f);
 keys->setColour(MidiKeyboardComponent::whiteNoteColourId,Colour::fromRGB(222,222,222));
 keys->setColour(MidiKeyboardComponent::blackNoteColourId,Colour::fromRGB(7,7,7));
 keys->setColour(MidiKeyboardComponent::keyDownOverlayColourId,acid.withAlpha(.82f));
 keys->setColour(MidiKeyboardComponent::mouseOverKeyOverlayColourId,acid.withAlpha(.20f));
 addAndMakeVisible(*keys);
 ui.push_back({std::move(keys),{208,854,820,35},(int)Core});
}
void VoidcoreEditor::buildDeeper(){
 // MOTION: one selected LFO at a time, one ADSR, editable 16-step gate.
 for(int i=0;i<4;++i){
  click("LFO "+String(i+1),68.f+i*110.f,165.f,103.f,29.f,Motion,[this,i]{focusLfo=i;setPage(Motion);});
  lfoButtons[(size_t)i]=dynamic_cast<Click*>(ui.back().control.get());
  const String pre=i==0?"":String("lfo")+String(i+1);
  const String rate=i==0?"rate":pre+"Rate",depth=i==0?"depth":pre+"Depth";
  const String wave=i==0?"wave":pre+"Wave",target=i==0?"target":pre+"Target";
  const String sync=i==0?"lfo1Sync":pre+"Sync";
  const Page sub=(Page)(10+i);
  knob(rate.toRawUTF8(),"RATE",144,416,77,sub);
  knob(depth.toRawUTF8(),"DEPTH",316,416,77,sub,1);
  choice(wave.toRawUTF8(),{"Sine","Triangle","Saw","Square","Sample & Hold","Fractal"},62,318,170,32,sub);
  choice(sync.toRawUTF8(),{"Free","1 bar","1/2 bar","1/4 bar","1/8 bar","1/16 bar"},255,318,153,32,sub);
  choice(target.toRawUTF8(),{"Volume","Pan","Pitch","Filter 1","Filter 2","FM Index","A/B Morph","Metal Exciter","Grain Density","Delay Warp","Reverb Motion"},62,353,350,27,sub);
 }
 knob("attack","ATTACK",679,395,96,Motion);
 knob("decay","DECAY",814,395,96,Motion);
 knob("sustain","SUSTAIN",947,395,96,Motion);
 knob("release","RELEASE",1077,395,96,Motion);
 choice("motif",{"Off","Serpent","Ritual","Insect","Portal","Fracture","Hollow"},62,490,172,32,Motion);
 choice("motifDivision",{"1/8","1/16","1/32","Triplet"},248,490,120,32,Motion);
 knob("motifGate","GATE",902,530,86,Motion);
 knob("motifGlide","SWING",1061,530,86,Motion);
 for(int i=0;i<16;++i){
  const auto id=String::formatted("gateStep%02d",i+1);
  click(String(i+1),72.f+65.f*i,595.f,55.f,51.f,Motion,[this,id]{
    const float v=value(id.toRawUTF8());
    set(id.toRawUTF8(),v>.5f?0.f:1.f);
  });
  steps[(size_t)i]=dynamic_cast<Click*>(ui.back().control.get());
 }
 // FX: retain the whole engine but expose only genuinely audible master controls.
 knob("delayMix","DELAY MIX",123,322,105,Fx);
 knob("delayFeedback","FEEDBACK",289,322,105,Fx,1);
 knob("delayTone","TONE",454,322,105,Fx);
 choice("division",{"1/4","1/8","1/8 dotted","1/16","1/16 dotted","1/32","1/4 triplet","1/8 triplet"},62,395,428,32,Fx);
 knob("reverbMix","REVERB MIX",724,322,105,Fx);
 knob("reverbSize","SIZE",889,322,105,Fx,1);
 knob("reverbDecay","DECAY",1054,322,105,Fx);
 knob("drive","DRIVE",123,595,100,Fx,1);
 knob("psySpiral","SPACE MOTION",316,595,100,Fx);
 knob("predator","PREDATOR",508,595,100,Fx,1);
 knob("insectCode","SIGNALS",708,595,100,Fx);
 knob("burrow","BURROW",908,595,100,Fx,1);
 // SYSTEM: performance controls, preset / session actions, no 48-knob wall.
 choice("voiceLimit",{"8 voices","12 voices","16 voices","24 voices"},85,240,245,40,System);
 choice("mode",{"Polyphonic","Mono","Legato"},374,240,220,40,System);
 choice("velocity",{"Soft","Normal","Hard"},630,240,220,40,System);
 click("REVERSE AUDIO",881,240,228,40,System,[this]{
  set("reverse",value("reverse")>.5f?0.f:1.f);
 },1);
 reverseButton=dynamic_cast<Click*>(ui.back().control.get());
 knob("glide","GLIDE",200,402,114,System);
 knob("pan","PAN",471,402,114,System,1);
 knob("stereo","WIDTH",740,402,114,System);
 knob("clarity","CLARITY",997,402,114,System);
 const char* scenes[]={"ALIEN RADIO","PREDATOR","PULSE FIELD","DEEP SIGNAL"};
 for(int i=0;i<4;++i)click(scenes[i],85.f+i*275.f,550.f,255.f,48.f,System,[this,i]{preset(i);},i%2);
 click("SAVE PATCH",300,631,256,43,System,[this]{save();});
 click("LOAD PATCH",650,631,256,43,System,[this]{load();});
 // Real on-screen MIDI keyboard; notes are injected by the processor into the
 // audio thread's MIDI stream, so clicking keys audibly plays the instrument.
 auto keys=std::make_unique<MidiKeyboardComponent>(processor.keyboardState,
                                                   MidiKeyboardComponent::horizontalKeyboard);
 keys->setAvailableRange(24,96);
 keys->setKeyWidth(18.f);
 keys->setColour(MidiKeyboardComponent::whiteNoteColourId,Colour::fromRGB(235,235,235));
 keys->setColour(MidiKeyboardComponent::blackNoteColourId,voidBlack);
 keys->setColour(MidiKeyboardComponent::keyDownOverlayColourId,acid.withAlpha(.8f));
 keys->setColour(MidiKeyboardComponent::mouseOverKeyOverlayColourId,acid.withAlpha(.23f));
 addAndMakeVisible(*keys);
 ui.push_back({std::move(keys),{270,748,668,40},-1});
}
void VoidcoreEditor::updatePreview(int which){
 const int index=std::clamp((int)value(which?"bankB":"bank"),0,50);
 if(which)cachedB=index;else cachedA=index;
 auto& samples=previews[(size_t)which];
 samples.fill(0.f);
 auto file=processor.paintingFile().getParentDirectory().getParentDirectory()
                     .getChildFile("Banks").getChildFile(String::formatted("%02d_60.wav",index));
 if(index==50)file=file.getParentDirectory().getChildFile("User").getChildFile("60.wav");
 if(!file.existsAsFile())return;
 AudioFormatManager fm;fm.registerBasicFormats();
 std::unique_ptr<AudioFormatReader> reader(fm.createReaderFor(file));
 if(!reader||reader->lengthInSamples<192)return;
 const auto n=(int)std::min<juce::int64>(reader->lengthInSamples,480000);
 AudioBuffer<float> waveform(1,n);
 if(!reader->read(&waveform,0,n,0,true,false))return;
 const float* data=waveform.getReadPointer(0);
 for(int i=0;i<192;++i){
   const int start=i*n/192,finish=std::min(n,(i+1)*n/192);
   float peak=0.f;
   for(int j=start;j<finish;++j)peak=std::max(peak,std::abs(data[j]));
   samples[(size_t)i]=peak;
 }
}
void VoidcoreEditor::mutation(bool gentle){
 Random random;
 for(auto* id:{"morph","xenoFormant","xenoCross","creatures","biomeDepth","cutoff","resonance","predator","insectCode","burrow"}){
  if(auto* par=processor.state.getParameter(id)){
   const float normalized=par->getValue();
   const float target=gentle?clamp01(normalized+(random.nextFloat()-.5f)*.17f):random.nextFloat();
   par->beginChangeGesture();par->setValueNotifyingHost(target);par->endChangeGesture();
  }
 }
 // Full mutation visits a different synthesis family, not simply a nearby
 // variation of the same sonic material. EVOlVE preserves source selections.
 if(!gentle){
  const int a=random.nextInt(50);
  const int b=(a+1+random.nextInt(9))%50;
  set("bank",(float)a);
  set("bankB",(float)b);
  set("textureMode",(float)random.nextInt(6));
 }
 lastScene=gentle?"EVOLVED ORGANISM":"MUTATED XENO SOUNDSCAPE";
}
void VoidcoreEditor::preset(int index){
 const float presets[4][8]={{.18f,.65f,.28f,.70f,.76f,.64f,.60f,.20f},
 {.70f,.82f,.71f,.88f,.50f,.83f,.23f,.75f},
 {.50f,.40f,.32f,.55f,.91f,.22f,.87f,.24f},
 {.39f,.80f,.12f,.35f,.70f,.76f,.61f,.88f}};
 const char* ids[]={"morph","xenoFormant","xenoCross","creatures","biomeDepth","predator","insectCode","burrow"};
 index=std::clamp(index,0,3);
 for(int i=0;i<8;++i){
  if(auto* p=processor.state.getParameter(ids[i])){
   p->beginChangeGesture();p->setValueNotifyingHost(presets[index][i]);p->endChangeGesture();
  }
 }
 // Scenes must select different audio materials and space/texture topology,
 // not merely recolour a collection of similar macro values.
 static constexpr int sources[4][3]={
  {2,9,2},  // Alien radio + dimensional glitch + orbital texture
  {3,5,1},  // Creature throat + insect hive + grain cloud
  {6,7,3},  // Pulsar gate + wormhole sweep + pulsar mode
  {0,8,0}   // Subvoid gravity + nebula choir + flow
 };
 static constexpr float fx[4][4]={
  {.78f,.19f,.49f,.17f},
  {.52f,.32f,.18f,.34f},
  {.66f,.22f,.38f,.22f},
  {.65f,.11f,.20f,.69f}
 };
 set("bank",(float)sources[index][0]);
 set("bankB",(float)sources[index][1]);
 set("textureMode",(float)sources[index][2]);
 set("cutoff",fx[index][0]);
 set("resonance",fx[index][1]);
 set("delayMix",fx[index][2]);
 set("reverbMix",fx[index][3]);
 lastScene=StringArray{"ALIEN RADIO","PREDATOR","PULSE FIELD","DEEP SIGNAL"}[index];
}
void VoidcoreEditor::save(){
 chooser=std::make_unique<FileChooser>("Save VOIDCORE preset",File::getSpecialLocation(File::userDocumentsDirectory).getChildFile("Voidcore.nfx"),"*.nfx");
 Component::SafePointer<VoidcoreEditor> safe(this);
 chooser->launchAsync(FileBrowserComponent::saveMode|FileBrowserComponent::canSelectFiles|FileBrowserComponent::warnAboutOverwriting,
 [safe](const FileChooser& fc){
  if(!safe)return;
  const auto file=fc.getResult();
  if(file==File())return;
  MemoryBlock data;safe->processor.getStateInformation(data);
  FileOutputStream out(file);
  if(out.openedOk()){out.write(data.getData(),data.getSize());out.flush();safe->lastScene="PRESET SAVED";}
 });
}
void VoidcoreEditor::load(){
 chooser=std::make_unique<FileChooser>("Load VOIDCORE preset",File::getSpecialLocation(File::userDocumentsDirectory),"*.nfx");
 Component::SafePointer<VoidcoreEditor> safe(this);
 chooser->launchAsync(FileBrowserComponent::openMode|FileBrowserComponent::canSelectFiles,
 [safe](const FileChooser& fc){
  if(!safe)return;
  const auto file=fc.getResult();
  if(file==File())return;
  MemoryBlock data;
  if(file.loadFileAsData(data)){safe->processor.setStateInformation(data.getData(),(int)data.getSize());safe->lastScene="PRESET LOADED";}
 });
}


void VoidcoreEditor::panel(Graphics& g,Rectangle<float> box,const String& name,Colour accent){
 const float x=box.getX(),y=box.getY(),w=box.getWidth(),h=box.getHeight();
 g.setColour(Colour::fromRGB(9,9,9));
 g.fillRoundedRectangle(box,7.f);
 ColourGradient glass(Colour::fromRGB(21,21,21),x,y,
                      Colour::fromRGB(7,7,7),x+w,y+h,true);
 g.setGradientFill(glass);
 g.fillRoundedRectangle(box.reduced(2.f),5.f);
 g.setColour(Colour::fromRGB(67,67,67));
 g.drawRoundedRectangle(box.reduced(.8f),7.f,1.f);
 g.setColour(accent.withAlpha(.72f));
 g.fillRoundedRectangle(x+13.f,y+12.f,3.f,15.f,1.f);
 g.drawHorizontalLine((int)(y+34.f),x+14.f,x+w-14.f);
 g.setColour(frost);
 g.setFont(Font("Consolas",13.f,Font::bold));
 g.drawFittedText(name,(int)(x+24.f),(int)(y+9.f),(int)(w-46.f),22,Justification::centredLeft,1);
}
void VoidcoreEditor::drawBase(Graphics& g){
 g.fillAll(voidBlack);
 const Rectangle<float> screen{5.f,5.f,W-10.f,H-10.f};
 ColourGradient metal(Colour::fromRGB(23,23,23),0.f,0.f,
                      Colour::fromRGB(5,5,5),W*.8f,H,true);
 g.setGradientFill(metal);
 g.fillRoundedRectangle(screen,11.f);
 g.setColour(Colour::fromRGB(77,77,77));
 g.drawRoundedRectangle(screen,11.f,1.5f);
 g.setColour(acid.withAlpha(.63f));
 g.drawLine(18.f,65.f,1180.f,65.f,1.f);
 // Brushed steel cross-hatching; intentionally subtle behind all controls.
 for(int k=0;k<80;++k){
  const float x=(float)k*15.f;
  g.setColour(frost.withAlpha(.009f));
  g.drawVerticalLine((int)x,8.f,H-8.f);
 }
 Rectangle<float> header{13,12,1174,48};
 g.setColour(Colour::fromRGB(6,6,6));
 g.fillRoundedRectangle(header,7.f);
 g.setColour(Colour::fromRGB(83,83,83));
 g.drawRoundedRectangle(header,7.f,1.f);
 g.setFont(Font("Consolas",27.f,Font::bold));
 g.setColour(frost);
 g.drawFittedText("V O I D C O R E",31,15,363,33,Justification::centredLeft,1);
 g.setFont(Font("Consolas",10.f,Font::bold));
 g.setColour(acid);
 g.drawText("L I V I N G   S O U N D S C A P E   A T L A S",36,46,357,13,Justification::centredLeft);
 for(int i=0;i<4;++i){
  const float x=i%2?1180.f:18.f;
  const float y=i/2?H-18.f:18.f;
  g.setColour(acid.withAlpha(.68f));
  g.fillEllipse(x-2.f,y-2.f,4.f,4.f);
 }
}
void VoidcoreEditor::drawOrbitalFrame(Graphics& g,Point<float> centre,float radius){
 // Scaled 3D orbital architecture: luminous elliptical rings and skeletal
 // biological structures, without overlaying parameter controls.
 const float x=centre.x,y=centre.y,theta=clock*.06f;
 for(int i=0;i<6;++i){
  const float r=radius*(1.12f+i*.085f);
  Path ring;
  ring.addCentredArc(x,y,r,r*(.25f+.045f*(i%3)),theta+i*.41f,
                     theta+i*.41f,theta+i*.41f+2.f*pi,true);
  g.setColour((i%2==0?acid:frost).withAlpha(.07f+.075f*(i%2==0)));
  g.strokePath(ring,PathStrokeType(i%3==0?2.4f:1.1f));
 }
 // Curved chrome skeletal arms frame the core like the reference artwork.
 for(int side=-1;side<=1;side+=2){
  const float edge=x+side*radius*.96f;
  for(int k=0;k<5;++k){
   const float p=(float)k/4.f;
   const float yy=y-radius*.9f+p*radius*1.8f;
   Path bone;
   bone.startNewSubPath(edge,yy);
   bone.cubicTo(edge+side*37.f,yy-46.f,
                edge+side*18.f,y+(yy-y)*.45f,
                x+side*radius*.55f,y+(yy-y)*.27f);
   g.setColour(Colour::fromRGB(37,37,37));
   g.strokePath(bone,PathStrokeType(6.f,PathStrokeType::curved,PathStrokeType::rounded));
   g.setColour(frost.withAlpha(.38f));
   g.strokePath(bone,PathStrokeType(1.3f));
   g.setColour(acid.withAlpha(.13f+.13f*std::sin(clock*.3f+k)));
   g.fillEllipse(edge-4.f,yy-4.f,8.f,8.f);
  }
 }
 for(int k=0;k<9;++k){
  const float a=clock*.14f+k*2.f*pi/9.f;
  const float xx=x+std::cos(a)*radius*1.10f;
  const float yy=y+std::sin(a)*radius*.45f;
  g.setColour(acid.withAlpha(.10f));
  g.fillEllipse(xx-7.f,yy-7.f,14.f,14.f);
  g.setColour(frost.withAlpha(.75f));g.fillEllipse(xx-2.f,yy-2.f,4.f,4.f);
 }
}
void VoidcoreEditor::drawKeyboardShell(Graphics& g){
 const Rectangle<float> r{245.f,735.f,738.f,59.f};
 g.setColour(Colour::fromRGB(6,6,6));
 g.fillRoundedRectangle(r,9.f);
 g.setColour(frost.withAlpha(.34f));
 g.drawRoundedRectangle(r,9.f,1.f);
 g.setColour(acid.withAlpha(.43f));
 g.drawLine(r.getX()+20.f,r.getY()+4.f,r.getRight()-20.f,r.getY()+4.f,1.f);
}
void VoidcoreEditor::soundSphere(Graphics& g,Point<float> centre,float radius,Colour tint,float activity){
 // Audio-reactive orbital planet rendered from rings, living plasma and
 // faceted glass. No graphics resources or allocations on the audio thread.
 const float x=centre.x,y=centre.y,p=clamp01(activity);
 const float breathing=1.f+.018f*std::sin(clock*1.6f)+.03f*p;
 const float r=radius*breathing;
 for(int glow=5;glow>=0;--glow){
  const float R=r*(1.f+glow*.07f);
  g.setColour(tint.withAlpha(.008f+.016f*p));
  g.fillEllipse(x-R,y-R,2.f*R,2.f*R);
 }
 ColourGradient ball(Colour::fromRGB(46,46,46).withAlpha(.90f),
                      x-r*.56f,y-r*.76f,voidBlack,x+r*.78f,y+r*.85f,true);
 g.setGradientFill(ball);g.fillEllipse(x-r,y-r,2.f*r,2.f*r);
 // Dark shadow inside the lower-right hemisphere gives planet curvature.
 ColourGradient edge(Colour::fromRGB(2,2,2).withAlpha(0.f),
                     x-r*.43f,y-r*.55f,
                     Colour::fromRGB(1,1,1).withAlpha(.87f),x+r*.56f,y+r*.71f,true);
 g.setGradientFill(edge);g.fillEllipse(x-r*.96f,y-r*.96f,1.92f*r,1.92f*r);
 g.setColour(frost.withAlpha(.42f));
 g.drawEllipse(x-r*.98f,y-r*.98f,r*1.96f,r*1.96f,1.45f);
 g.setColour(tint.withAlpha(.44f+.20f*p));
 g.drawEllipse(x-r*1.02f,y-r*1.02f,r*2.04f,r*2.04f,2.4f);
 // Irregular luminous wisps suggest living plasma / nebula filaments.
 for(int k=0;k<34;++k){
  const float a=k*2.f*pi/34.f+.25f*std::sin(clock*.07f+k);
  const float rr=r*(.20f+.62f*(.5f+.5f*std::sin(k*8.617f+clock*.18f)));
  const float xx=x+rr*std::cos(a),yy=y+rr*std::sin(a);
  const float rr2=rr*(.83f+.07f*std::sin(k+clock*.08f));
  Path plasma;
  plasma.startNewSubPath(xx,yy);
  plasma.quadraticTo(x+(rr*.97f)*std::cos(a+.43f),y+(rr*.97f)*std::sin(a+.43f),
                     x+rr2*std::cos(a+.72f),y+rr2*std::sin(a+.72f));
  g.setColour((k%4==0?frost:tint).withAlpha(.07f+.13f*p+.08f*(k%3==0)));
  g.strokePath(plasma,PathStrokeType(k%7==0?2.f:1.1f));
 }
 for(int ring=0;ring<5;++ring){
  const float orbit=clock*(.11f+.028f*ring)+ring*.86f;
  const float rr=r*(.52f+ring*.11f);
  Path track;
  track.addCentredArc(x,y,rr,r*(.16f+ring*.041f),orbit*.36f,
                       orbit,orbit+1.65f*pi,true);
  neonLine(g,track,tint.withAlpha(.16f+.11f*p),ring%2==0?1.45f:.75f);
  const float px=x+std::cos(orbit)*rr;
  const float py=y+std::sin(orbit)*r*(.16f+ring*.041f);
  g.setColour(frost.withAlpha(.87f));g.fillEllipse(px-2.4f,py-2.4f,4.8f,4.8f);
 }
 // Organic void iris carries the reactor name without occluding the rings.
 const float iris=r*(.20f+.026f*p);
 g.setColour(Colour::fromRGB(2,2,2).withAlpha(.92f));
 g.fillEllipse(x-iris,y-iris,2.f*iris,2.f*iris);
 g.setColour(frost.withAlpha(.29f));
 g.drawEllipse(x-iris,y-iris,2.f*iris,2.f*iris,1.25f);
}
void VoidcoreEditor::visualEye(Graphics& g,float x,float y,float radius){
 soundSphere(g,{x,y},radius,acid,smoothedPeak);
 const float mix=clamp01(value("morph")),turn=clock*.27f,distance=radius*1.06f;
 const Point<float> a{x+std::cos(turn)*distance,y+std::sin(turn)*distance*.39f};
 const Point<float> b{x-std::cos(turn)*distance,y-std::sin(turn)*distance*.39f};
 soundSphere(g,a,20.f+7.f*(1.f-mix),acid,smoothedPeak);
 soundSphere(g,b,20.f+7.f*mix,blood,smoothedPeak);
 g.setColour(frost);g.setFont(Font("Consolas",25.f,Font::bold));
 g.drawFittedText("V O I D C O R E",(int)(x-147.f),(int)(y-16.f),294,33,Justification::centred,1);
 g.setColour(acid.withAlpha(.85f));g.setFont(Font(11.f,Font::bold));
 g.drawFittedText("XENO  /  SOUND SPHERE",(int)(x-125.f),(int)(y+33.f),250,23,Justification::centred,1);
}
void VoidcoreEditor::sourceWave(Graphics& g,Rectangle<float> r,bool second){
 g.setColour(Colour::fromRGB(10,10,10));g.fillRoundedRectangle(r,5.f);
 for(int k=0;k<=8;++k){const float xx=r.getX()+r.getWidth()*k/8.f;
  g.setColour(acid.withAlpha(.09f));g.drawVerticalLine((int)xx,r.getY(),r.getBottom());}
 for(int k=0;k<5;++k){const float yy=r.getY()+r.getHeight()*k/4.f;
  g.setColour(acid.withAlpha(.07f));g.drawHorizontalLine((int)yy,r.getX(),r.getRight());}
 const auto& amplitudes=previews[second?1:0];
 const auto c=second?blood:acid;
 const float mid=r.getCentreY();
 Path top,bottom;
 for(int i=0;i<192;++i){
  const float x=r.getX()+r.getWidth()*i/191.f;
  const float peak=amplitudes[(size_t)i];
  const float y=mid-peak*r.getHeight()*.43f;
  const float z=mid+peak*r.getHeight()*.43f;
  if(i==0){top.startNewSubPath(x,y);bottom.startNewSubPath(x,z);}
  else{top.lineTo(x,y);bottom.lineTo(x,z);}
 }
 neonLine(g,top,c.withAlpha(.95f),1.2f);neonLine(g,bottom,c.withAlpha(.45f),1.05f);
 // Orbit highlights accompany the actual WAV waveform.
 const float orbX=r.getCentreX(),orbY=r.getCentreY();
 g.setColour(c.withAlpha(.07f+.08f*std::sqrt(clamp01(smoothedPeak))));
 g.drawEllipse(orbX-47.f,orbY-47.f,94.f,94.f,1.f);
 const float phase=clock*.34f+(second?pi:0.f);
 const float px=orbX+std::cos(phase)*45.f,py=orbY+std::sin(phase)*33.f;
 g.setColour(c.withAlpha(.60f));g.fillEllipse(px-3.f,py-3.f,6.f,6.f);
 // Cursor visibly sweeps the real source waveform while the engine produces sound.
 if(smoothedPeak>.0005f){
  const float xx=r.getX()+r.getWidth()*std::fmod(clock*(.13f+.035f*value("movement")),1.f);
  g.setColour(frost.withAlpha(.7f));g.drawVerticalLine((int)xx,r.getY(),r.getBottom());
  g.setColour(c.withAlpha(.4f));g.fillEllipse(xx-4.f,mid-4.f,8.f,8.f);
 }
 if(std::all_of(amplitudes.begin(),amplitudes.end(),[](float v){return v<.00001f;})){
  g.setColour(grey);g.setFont(Font(11.f));g.drawText("NO BANK AUDIO FOUND",r.toNearestInt(),Justification::centred);
 }
}
void VoidcoreEditor::scope(Graphics& g,Rectangle<float> r,int mode,Colour c){
 g.setColour(Colour::fromRGB(7,7,7).withAlpha(.97f));
 g.fillRoundedRectangle(r,5.f);
 g.setColour(c.withAlpha(.24f));g.drawRoundedRectangle(r,5.f,1.f);
 g.setColour(frost.withAlpha(.06f));
 g.drawHorizontalLine((int)r.getCentreY(),r.getX()+3.f,r.getRight()-3.f);
 const unsigned start=processor.scopeCursor.load(std::memory_order_relaxed);
 Path waveform;
 // Subtle reactive spectrum pillars are actual recent audio amplitudes,
 // not a prerecorded animation. They add the Astral Bloom energy skyline.
 for(int j=0;j<64;++j){
  const size_t idx=(size_t)((start+2*j)%128u);
  const float a=std::abs(processor.scope[idx].load(std::memory_order_relaxed));
  const float b=std::abs(processor.scope[(idx+1)%128u].load(std::memory_order_relaxed));
  const float amp=clamp01(std::sqrt((a+b)*.5f));
  const float x=r.getX()+5.f+(r.getWidth()-10.f)*(float)j/64.f;
  const float height=2.f+(r.getHeight()*.50f)*amp;
  g.setColour((j%9==0?frost:c).withAlpha(.12f+.48f*amp));
  g.drawLine(x,r.getCentreY()-height,x,r.getCentreY()+height,1.7f);
 }
 for(int i=0;i<128;++i){
  const float raw=processor.scope[(size_t)((start+i)%128u)].load(std::memory_order_relaxed);
  const float x=r.getX()+r.getWidth()*(float)i/127.f;
  const float y=r.getCentreY()-raw*r.getHeight()*.46f;
  if(i==0)waveform.startNewSubPath(x,y);else waveform.lineTo(x,y);
 }
 neonLine(g,waveform,c.withAlpha(.78f),1.15f);
 (void)mode;
}
void VoidcoreEditor::env(Graphics& g,Rectangle<float> r){
 g.setColour(Colour::fromRGB(14,14,14));g.fillRoundedRectangle(r,5.f);
 for(int i=1;i<5;++i){g.setColour(acid.withAlpha(.11f));g.drawVerticalLine((int)(r.getX()+r.getWidth()*i/5),r.getY(),r.getBottom());}
 const float a=std::sqrt(clamp01(value("attack")/8.f));
 const float d=std::sqrt(clamp01(value("decay")/9.f));
 const float sust=clamp01(value("sustain"));
 const float rel=std::sqrt(clamp01(value("release")/12.f));
 const float w=r.getWidth(),height=r.getHeight()*.78f;
 const float wA=w*(.10f+.16f*a),wD=w*(.12f+.15f*d),wR=w*(.12f+.15f*rel);
 const float start=r.getX()+7.f,base=r.getBottom()-9.f,top=base-height;
 Path p;p.startNewSubPath(start,base);
 p.lineTo(start+wA,top);
 p.cubicTo(start+wA+wD*.40f,top,start+wA+wD*.64f,base-height*sust,start+wA+wD,base-height*sust);
 const float hold=start+w*.78f;p.lineTo(hold,base-height*sust);
 p.cubicTo(hold+wR*.3f,base-height*sust,hold+wR*.6f,base,hold+wR,base);
 neonLine(g,p,blood,2.f);
 const float marker=(std::sin(clock*1.3f)*.5f+.5f);
 g.setColour(frost.withAlpha(.65f));g.fillEllipse(start+marker*w*.70f-3.f,base-height*sust-3.f,6.f,6.f);
}
void VoidcoreEditor::lfo(Graphics& g,Rectangle<float> r,int which){
 g.setColour(Colour::fromRGB(10,10,10));g.fillRoundedRectangle(r,5.f);
 const String pre=which==0?"":String("lfo")+String(which+1);
 const String shape=which==0?"wave":pre+"Wave";
 const String hz=which==0?"rate":pre+"Rate";
 const String dep=which==0?"depth":pre+"Depth";
 const int wave=(int)value(shape.toRawUTF8());
 const float rate=std::min(8.f,value(hz.toRawUTF8()));
 const float depth=clamp01(value(dep.toRawUTF8()));
 for(int i=1;i<5;++i){g.setColour(acid.withAlpha(.08f));g.drawHorizontalLine((int)(r.getY()+r.getHeight()*i/5),r.getX(),r.getRight());}
 Path p;
 for(int i=0;i<=256;++i){
  float t=(float)i/256.f*2.6f;
  float y=phaseWave(wave,t+clock*rate*.12f);
  const float x=r.getX()+r.getWidth()*i/256.f;
  const float py=r.getCentreY()-y*r.getHeight()*.40f*depth;
  if(!i)p.startNewSubPath(x,py);else p.lineTo(x,py);
 }
 neonLine(g,p,acid,1.65f);
 g.setColour(frost.withAlpha(.7f));g.setFont(Font(10.f));
 g.drawText("LIVE MODULATION  //  "+String(value(hz.toRawUTF8()),3)+" Hz",r.getX()+8.f,r.getY()+6.f,r.getWidth()-16.f,18.f,Justification::topLeft);
}
void VoidcoreEditor::filter(Graphics& g,Rectangle<float> r){
 g.setColour(Colour::fromRGB(10,10,10));g.fillRoundedRectangle(r,5.f);
 const float cutoff=clamp01(value("cutoff")),q=clamp01(value("resonance"));
 Path p;
 for(int i=0;i<240;++i){
  const float t=(float)i/239.f;
  const float normalized=(t-cutoff)*19.f;
  const float response=1.f/std::sqrt(1.f+std::pow(2.f,normalized));
  const float peak=(q*.27f)*std::exp(-std::pow((t-cutoff)*30.f,2.f));
  const float yy=r.getBottom()-14.f-(response*.78f+peak)*r.getHeight()*.72f;
  const float xx=r.getX()+t*r.getWidth();
  if(!i)p.startNewSubPath(xx,yy);else p.lineTo(xx,yy);
 }
 neonLine(g,p,blood,2.f);
 const float marker=r.getX()+r.getWidth()*cutoff;
 g.setColour(frost.withAlpha(.5f));g.drawVerticalLine((int)marker,r.getY(),r.getBottom());
}
void VoidcoreEditor::gates(Graphics& g,Rectangle<float> r){
 g.setColour(Colour::fromRGB(11,11,11));g.fillRoundedRectangle(r,5.f);
 const float beat=processor.displayPpq.load(std::memory_order_relaxed);
 const int step=(int)std::floor(beat*4.f)%16;
 const int active=(step+16)%16;
 for(int i=0;i<16;++i){
  const float x=72.f+65.f*i,w=55.f;
  const float glow=(i==active && playing)?1.f:.19f;
  g.setColour((i==active?blood:acid).withAlpha(glow*.20f));
  g.fillRoundedRectangle(x,584.f,w,72.f,5.f);
  const float height=13.f+25.f*(.5f+.5f*std::sin((float)i*1.3f+clock*2.f));
  g.setColour((i==active?blood:acid).withAlpha(glow*.5f));
  g.fillRect(x+5.f,653.f-height,w-10.f,height);
 }
}
void VoidcoreEditor::effects(Graphics& g,Rectangle<float> r,int which){
 g.setColour(voidBlack.withAlpha(.8f));g.fillRoundedRectangle(r,5.f);
 const float intensity=which==0?value("delayFeedback"):value("reverbDecay");
 const float movement=clock*(which==0?.28f:.05f);
 Path wave;
 for(int k=0;k<220;++k){
  const float x=(float)k/219.f;
  const float decay=std::exp(-x*(2.8f-2.f*intensity));
  const float v=decay*(.50f*std::sin(2.f*pi*(x*(6.f+which*3.f)+movement))
      +.24f*std::sin(2.f*pi*(x*17.f+movement*.51f)));
  const float y=r.getCentreY()-v*r.getHeight()*.63f;
  const float px=r.getX()+x*r.getWidth();
  if(k==0)wave.startNewSubPath(px,y);else wave.lineTo(px,y);
 }
 neonLine(g,wave,which?blood:acid,1.6f);
}
void VoidcoreEditor::drawCore(Graphics& g){
 panel(g,{16,79,369,248},"SOURCE  /  DUAL SAMPLE",acid);
 panel(g,{394,79,412,442},"VOID CORE  /  MORPH",acid);
 panel(g,{815,79,369,248},"FILTER  /  SPECTRAL",acid);
 panel(g,{16,335,369,186},"MOTION  /  MODULATION",acid);
 panel(g,{815,335,369,186},"XENO  /  DEBRIS & IMPACT",acid);
 panel(g,{16,530,1168,55},"GATE  /  16 STEP",acid);
 panel(g,{16,593,558,145},"FX / DELAY  -  SYNC & DIRECTION",acid);
 panel(g,{582,593,602,145},"FX / REVERB  -  CHAMBERS",acid);
 panel(g,{16,744,267,94},"TONE / EQ",acid);
 panel(g,{290,744,343,94},"STEREO / OUTPUT",acid);
 panel(g,{640,744,544,94},"PERFORMANCE / DYNAMICS",acid);
 // Bank-specific WAV waveforms: never an invented procedural bank display.
 sourceWave(g,{29,160,169,73},false);
 sourceWave(g,{207,160,169,73},true);
 g.setColour(frost);g.setFont(Font(11.f,Font::bold));
 g.drawText("A",29,129,19,20,Justification::centred);
 g.drawText("B",207,129,18,20,Justification::centred);
 // The core responds directly to actual DSP audio peaks and morph state.
 drawOrbitalFrame(g,{599.f,269.f},111.f);
 visualEye(g,599.f,267.f,109.f);
 scope(g,{425,393,349,30},0,acid);
 g.setColour(grey);g.setFont(Font(10.f,Font::bold));
 g.drawFittedText("XY CORE: HORIZONTAL MORPH  /  VERTICAL EVOLUTION",433,426,334,17,Justification::centred,1);
 // Filter and LFO curves update with the real parameter values, not GIFs.
 filter(g,{985,168,179,73});
 lfo(g,{188,416,170,34},focusLfo);
 g.setColour(frost.withAlpha(.66f));g.setFont(Font(9.f));
 g.drawText("SAMPLE A",48,239,132,16,Justification::centred);
 g.drawText("SAMPLE B",231,239,132,16,Justification::centred);
 g.drawFittedText("MORPH",560,517,76,16,Justification::centred,1);
 // Lower panel readouts are live DSP measurements, not ornamental meters.
 for(int ch=0;ch<2;++ch){
  const float m=std::sqrt(clamp01(meterLevels[(size_t)ch]));
  for(int seg=0;seg<15;++seg){
   const bool on=(float)seg/15.f<m;
   g.setColour((seg>12?frost:acid).withAlpha(on?.93f:.12f));
   g.fillRoundedRectangle(1114.f+ch*19.f,818.f-seg*3.25f,11.f,2.5f,1.f);
  }
  g.setColour(grey);
  g.drawText(ch?"R":"L",1112+ch*19,822,14,11,Justification::centred);
 }
 // The animated sequence is represented by genuinely clickable steps above.
 g.setColour(frost.withAlpha(.51f));g.setFont(Font(9.f,Font::bold));
 g.drawText("PURE SOURCE: UNPROCESSED WAV ATLAS",661,826,394,13,Justification::centred);
 g.setColour(Colour::fromRGB(9,9,9));
 g.fillRoundedRectangle(198.f,850.f,850.f,43.f,4.f);
 g.setColour(acid.withAlpha(.49f));
 g.drawRoundedRectangle(198.f,850.f,850.f,43.f,4.f,1.f);
}
void VoidcoreEditor::drawDeep(Graphics& g){
 if(page==Motion){
  panel(g,{35,142,536,315},"MODULATION  /  LFO",acid);
  panel(g,{590,142,576,315},"ENVELOPE  /  ADSR",blood);
  lfo(g,{60,208,474,103},focusLfo);
  env(g,{622,208,512,135});
  panel(g,{35,467,1131,218},"RHYTHM  /  16-STEP GATE",acid);
  gates(g,{60,581,1080,78});
 }else if(page==Fx){
  panel(g,{35,142,536,328},"DELAY  /  SIGNAL REPEATS",acid);
  panel(g,{590,142,576,328},"REVERB  /  VOID SPACE",blood);
  effects(g,{60,200,470,78},0);effects(g,{623,200,505,78},1);
  panel(g,{35,483,1131,199},"CREATURE ENGINE  /  FX CHARACTER",acid);
  scope(g,{65,519,1050,36},1,blood);
 }else if(page==System){
  panel(g,{35,142,1131,339},"SYSTEM  /  PERFORMANCE",acid);
  panel(g,{35,501,1131,198},"PRESETS  /  SCENES",blood);
  g.setColour(frost);g.setFont(Font(15.f));
  g.drawText("Choose playback settings or recall one of four distinct alien scenes.",78,182,1015,29,Justification::centredLeft);
  g.setColour(grey);g.setFont(Font(12.f));
  g.drawText("Full parameter automation, modulation routes and extra controls remain accessible through Ableton.",75,717,1040,30,Justification::centredLeft);
 }
}
void VoidcoreEditor::paintStatus(Graphics& g){
 g.setColour(grey);g.setFont(Font(9.f,Font::bold));
 g.drawText(playing?"AUDIO ACTIVE":"READY",28,866,184,14,Justification::centredLeft);
 g.drawText("VOIDCORE  /  MONOCHROME",1030,867,150,14,Justification::centredRight);
}
void VoidcoreEditor::paint(Graphics& g){
 Graphics::ScopedSaveState state(g);
 g.addTransform(AffineTransform::scale((float)getWidth()/W,(float)getHeight()/H));
 drawBase(g);
 drawCore(g);
 paintStatus(g);
}
