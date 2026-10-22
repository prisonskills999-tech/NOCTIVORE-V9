#include "VoidcoreEditor.h"
#include <functional>
#include <cmath>
#include <algorithm>
using namespace juce;
namespace {
constexpr float pi=juce::MathConstants<float>::pi;
const Colour acid=Colour::fromRGB(87,236,155), blood=Colour::fromRGB(247,52,68);
const Colour frost=Colour::fromRGB(213,245,224), grey=Colour::fromRGB(103,151,131);
const Colour voidBlack=Colour::fromRGB(5,11,9);
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
 :label(text),accent(c){
 setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
 setRotaryParameters(pi*1.22f,pi*2.78f,true);
 setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);
 setMouseDragSensitivity(180);
 setTooltip(text+" — drag to adjust, double-click to reset");
 if(auto* param=p.state.getParameter(id))
   setDoubleClickReturnValue(true,param->convertFrom0to1(param->getDefaultValue()));
 link=std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(p.state,id,*this);
}
void VoidcoreEditor::Dial::paint(Graphics& g){
 const auto r=getLocalBounds().toFloat(),w=r.getWidth(),h=r.getHeight();
 const float rad=std::max(9.f,std::min(w*.31f,h*.32f));
 const float cx=r.getCentreX(),cy=r.getY()+h*.36f;
 const float lo=pi*1.22f,hi=pi*2.78f;
 const float norm=getMaximum()>getMinimum()?clamp01((float)((getValue()-getMinimum())/(getMaximum()-getMinimum()))):0.f;
 auto a=lo+(hi-lo)*norm;
 ColourGradient shade(Colour::fromRGB(57,68,58),cx-rad,cy-rad,voidBlack,cx+rad,cy+rad,false);
 g.setGradientFill(shade);g.fillEllipse(cx-rad*.86f,cy-rad*.86f,rad*1.72f,rad*1.72f);
 g.setColour(Colour::fromRGB(9,24,18));g.drawEllipse(cx-rad,cy-rad,rad*2,rad*2,3.f);
 g.setColour(grey.withAlpha(.55f));g.drawEllipse(cx-rad*.78f,cy-rad*.78f,rad*1.56f,rad*1.56f,1.f);
 Path empty;empty.addCentredArc(cx,cy,rad*.95f,rad*.95f,0,lo,hi,true);
 g.setColour(accent.withAlpha(.22f));g.strokePath(empty,PathStrokeType(4.f));
 Path active;active.addCentredArc(cx,cy,rad*.95f,rad*.95f,0,lo,a,true);
 g.setColour(accent);g.strokePath(active,PathStrokeType(3.7f));
 float angle=a-pi*.5f;
 g.setColour(accent.brighter(.3f));g.drawLine(cx,cy,cx+std::cos(angle)*rad*.72f,cy+std::sin(angle)*rad*.72f,2.5f);
 g.fillEllipse(cx-2.f,cy-2.f,4.f,4.f);
 g.setColour(frost);g.setFont(Font(10.5f,Font::bold));
 g.drawText(label,0,roundToInt(h*.73f),roundToInt(w),roundToInt(h*.14f),Justification::centred);
 g.setColour(accent.withAlpha(.86f));g.setFont(Font(9.7f));
 const String v=String(getValue(),(getMaximum()>5?1:2));
 g.drawText(v,0,roundToInt(h*.85f),roundToInt(w),roundToInt(h*.14f),Justification::centred);
}
VoidcoreEditor::Choice::Choice(NoctivoreProcessor& p,const String& id,const StringArray& items){
 addItemList(items,1);
 setJustificationType(Justification::centredLeft);
 setColour(ComboBox::backgroundColourId,Colour::fromRGB(6,18,14));
 setColour(ComboBox::textColourId,frost);
 setColour(ComboBox::outlineColourId,grey.withAlpha(.8f));
 setColour(ComboBox::arrowColourId,acid);
 setColour(PopupMenu::backgroundColourId,voidBlack);
 setColour(PopupMenu::textColourId,frost);
 setColour(PopupMenu::highlightedBackgroundColourId,Colour::fromRGB(20,93,53));
 link=std::make_unique<AudioProcessorValueTreeState::ComboBoxAttachment>(p.state,id,*this);
}
void VoidcoreEditor::Choice::paint(Graphics& g){
 ComboBox::paint(g);
 auto r=getLocalBounds().toFloat().reduced(1.5f);
 g.setColour(acid.withAlpha(.15f));g.drawRoundedRectangle(r,3.0f,1.f);
}
VoidcoreEditor::Click::Click(const String& t,Colour c):TextButton(t),accent(c){
 setClickingTogglesState(false);
 setTooltip(t);
}
void VoidcoreEditor::Click::paintButton(Graphics& g,bool over,bool pressed){
 auto r=getLocalBounds().toFloat().reduced(1.f);
 g.setColour((selected?accent:Colour::fromRGB(9,24,18)).withAlpha(selected?.30f:1.f));
 g.fillRoundedRectangle(r,3.f);
 g.setColour(accent.withAlpha((pressed||over)?1.f:selected?.88f:.35f));
 g.drawRoundedRectangle(r,3.f,(pressed||over)?1.5f:1.f);
 g.setColour((selected||pressed)?frost:(over?accent:grey));
 g.setFont(Font(std::clamp(r.getHeight()*.34f,9.f,13.f),Font::bold));
 g.drawFittedText(getButtonText(),getLocalBounds().reduced(3,1),Justification::centred,1,.75f);
}
VoidcoreEditor::VoidcoreEditor(NoctivoreProcessor& p):AudioProcessorEditor(p),processor(p){
 setOpaque(true);
 // The full-resolution artwork is optional; if present it is the untouched
 // image in the VST3's Resources/GUI/voidcore.png. Real controls stay separate.
 auto art=processor.paintingFile().getSiblingFile("voidcore.png");
 if(art.existsAsFile())reference=ImageFileFormat::loadFrom(art);
 buildCore();buildDeeper();setPage(Core);
 setResizable(true,true);
 setResizeLimits(960,640,1536,1024);
 if(auto* constrainer=getConstrainer())constrainer->setFixedAspectRatio(1.5);
 setSize(1152,768);
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
 ui.push_back({std::move(c),{x-d*.5f,y-d*.43f,d,d*.95f},(int)page_});
}
void VoidcoreEditor::choice(const char* id,StringArray items,float x,float y,float w,float h,Page page_){
 auto c=std::make_unique<Choice>(processor,id,items);
 ui.push_back({std::move(c),{x,y,w,h},(int)page_});
}
void VoidcoreEditor::click(const String& label,float x,float y,float w,float h,Page pg,std::function<void()> fn,int style){
 auto c=std::make_unique<Click>(label,style%3==1?blood:acid);
 c->onClick=std::move(fn);
 ui.push_back({std::move(c),{x,y,w,h},(int)pg});
}
void VoidcoreEditor::allClick(const String& label,float x,float y,float w,float h,std::function<void()> fn,int style){
 auto c=std::make_unique<Click>(label,style%3==1?blood:acid);
 c->onClick=std::move(fn);
 ui.push_back({std::move(c),{x,y,w,h},-1});
}
void VoidcoreEditor::setPage(Page next){
 page=next;
 for(auto& node:ui)node.control->setVisible(node.onPage<0||node.onPage==(int)page);
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
 playing=processor.displayPlaying.load(std::memory_order_relaxed);
 repaint();
}
