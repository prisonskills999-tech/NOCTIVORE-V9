#include "VoidcoreEditor.h"
#include <functional>
#include <cmath>
#include <algorithm>
using namespace juce;
namespace {
constexpr float pi=juce::MathConstants<float>::pi;
// Strict Matrix palette: luminous green, black and neutral white/grey only.
const Colour acid=Colour::fromRGB(53,255,105), blood=Colour::fromRGB(229,255,235);
const Colour frost=Colour::fromRGB(248,255,249), grey=Colour::fromRGB(143,161,149);
const Colour voidBlack=Colour::fromRGB(2,5,3);
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
 const auto r=getLocalBounds().toFloat();
 const float w=r.getWidth(),h=r.getHeight();
 const float rad=std::max(9.f,std::min(w*.30f,h*.31f));
 const float cx=r.getCentreX(),cy=r.getY()+h*.34f;
 const float lo=pi*1.22f,hi=pi*2.78f;
 const float norm=getMaximum()>getMinimum()?
    clamp01((float)((getValue()-getMinimum())/(getMaximum()-getMinimum()))):0.f;
 const float angle=lo+(hi-lo)*norm;
 const float pulse=.5f+.5f*std::sin(animationPhase*1.8f);
 // A real rotary JUCE Slider with a black-glass xeno-orb skin.
 g.setColour(accent.withAlpha(.05f+.09f*pulse));
 g.fillEllipse(cx-rad*1.37f,cy-rad*1.37f,2.74f*rad,2.74f*rad);
 g.setColour(frost.withAlpha(.42f));
 g.fillEllipse(cx-rad*1.12f,cy-rad*1.12f,2.24f*rad,2.24f*rad);
 g.setColour(voidBlack);
 g.fillEllipse(cx-rad,cy-rad,2.f*rad,2.f*rad);
 ColourGradient shell(accent.withAlpha(.43f),cx-rad*.45f,cy-rad*.65f,
                      Colour::fromRGB(1,9,4),cx+rad*.7f,cy+rad*.8f,true);
 g.setGradientFill(shell);
 g.fillEllipse(cx-rad*.89f,cy-rad*.89f,1.78f*rad,1.78f*rad);
 g.setColour(frost.withAlpha(.36f));
 g.drawEllipse(cx-rad*.78f,cy-rad*.78f,rad*1.56f,rad*1.56f,1.0f);
 for(int k=0;k<3;++k){
  const float a=animationPhase*.30f+k*2.094f;
  const float py=cy+std::sin(a)*rad*.40f;
  const float px=cx+std::cos(a)*rad*.71f;
  g.setColour(accent.withAlpha(.16f+.13f*pulse));
  g.drawEllipse(cx-rad*.65f,cy-rad*(.17f+.085f*k),
                1.3f*rad,rad*(.34f+.17f*k),1.f);
  g.setColour(frost.withAlpha(.62f));
  g.fillEllipse(px-1.3f,py-1.3f,2.6f,2.6f);
 }
 Path rail;rail.addCentredArc(cx,cy,rad*1.07f,rad*1.07f,0.f,lo,hi,true);
 g.setColour(accent.withAlpha(.22f));g.strokePath(rail,PathStrokeType(2.6f));
 Path active;active.addCentredArc(cx,cy,rad*1.07f,rad*1.07f,0.f,lo,angle,true);
 g.setColour(accent);g.strokePath(active,PathStrokeType(2.7f));
 const float marker=angle-pi*.5f;
 g.setColour(frost);
 g.drawLine(cx+std::cos(marker)*rad*.46f,cy+std::sin(marker)*rad*.46f,
            cx+std::cos(marker)*rad*.83f,cy+std::sin(marker)*rad*.83f,1.8f);
 g.setColour(frost);g.setFont(Font(std::clamp(h*.176f,8.f,11.f),Font::bold));
 g.drawFittedText(label,0,roundToInt(h*.69f),roundToInt(w),roundToInt(h*.18f),Justification::centred,1);
 g.setColour(accent.withAlpha(.95f));g.setFont(Font(std::clamp(h*.147f,8.f,10.f)));
 g.drawFittedText(String(getValue(),getMaximum()>5?1:2),0,roundToInt(h*.85f),
                  roundToInt(w),roundToInt(h*.14f),Justification::centred,1);
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
 g.setColour(lit?Colour::fromRGB(9,44,22):Colour::fromRGB(5,16,9));
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
 buildCore();buildDeeper();setPage(Core);
 setResizable(true,true);
 setResizeLimits(900,600,1500,1000);
 if(auto* constrainer=getConstrainer())constrainer->setFixedAspectRatio(1.5);
 setSize(1050,700);
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
 // Astral Bloom: photographic 3:2 layout, but every rendered control is a real
 // JUCE child. All parameter writes use the host-notifying APVTS path.
 for(int i=0;i<4;++i){
  const Page target=(Page)i;
  allClick(pageNames[(size_t)i],778.f+i*102.f,20.f,94.f,32.f,[this,target]{setPage(target);});
  pageButtons[(size_t)i]=dynamic_cast<Click*>(ui.back().control.get());
 }
 allClick("SAVE",403,20,65,32,[this]{save();});
 allClick("LOAD",475,20,65,32,[this]{load();});
 allClick("INIT",547,20,65,32,[this]{preset(0);});
 allClick("RANDOM",619,20,84,32,[this]{mutation(false);});
 allClick("EVOLVE",709,20,63,32,[this]{mutation(true);});
 StringArray banks;
 for(const auto& bank:noctivore::banks)banks.add(bank.name);
 banks.add("User");
 // The source menu shows the *real* sample bank name. Arrows change the
 // host-automatable selection, never a visual-only picture.
 choice("bank",banks,55,127,135,28,Core);
 choice("bankB",banks,229,127,134,28,Core);
 click("<",28,127,24,28,Core,[this]{shift("bank",-1);});
 click(">",193,127,24,28,Core,[this]{shift("bank",1);});
 click("<",202,215,23,29,Core,[this]{shift("bankB",-1);});
 click(">",364,215,23,29,Core,[this]{shift("bankB",1);});
 // Small sound-sphere knobs follow actual existing DSP parameters.
 knob("start","START",67,293,53,Core);
 knob("xenoFormant","THROAT",131,293,53,Core);
 knob("grain","GRAIN",187,293,53,Core);
 knob("texture","TEXTURE",249,293,53,Core);
 knob("xenoCross","COUPLE",310,293,53,Core);
 knob("symbiosis","BOND",366,293,53,Core);
 fader("morph",62,320,307,26,Core);
 // Source A and B waveforms are decoded from 48k factory WAVs.
 // Filter model, its selectable operating modes, and envelope/saturation.
 choice("filterMode",{"Low Pass","Band Pass","High Pass"},865,130,218,27,Core);
 knob("cutoff","CUTOFF",923,246,99,Core);
 knob("resonance","RES",1032,280,62,Core);
 knob("filterDrive","DRIVE",1116,280,62,Core);
 for(int i=0;i<3;++i){
  const juce::String name=juce::StringArray{"LP","BP","HP"}[i];
  click(name,1104,171.f+i*34.f,68,28,Core,[this,i]{set("filterMode",(float)i);});
  filterModeButtons[(size_t)i]=dynamic_cast<Click*>(ui.back().control.get());
 }
 // LFO1: explicitly bound to real modulation routing, shape and depth.
 choice("wave",{"Sine","Triangle","Saw","Square","Sample & Hold","Fractal"},34,393,160,29,Core);
 choice("lfo1Sync",{"Free","1 bar","1/2 bar","1/4 bar","1/8 bar","1/16 bar"},207,393,161,29,Core);
 knob("rate","RATE",77,522,62,Core);
 knob("depth","DEPTH",170,522,62,Core);
 knob("movement","MOTION",269,522,62,Core);
 // Amplifier envelope visible beneath the real ADSR graph.
 knob("attack","ATTACK",47,695,60,Core);
 knob("decay","DECAY",118,695,60,Core);
 knob("sustain","SUSTAIN",190,695,60,Core);
 knob("release","RELEASE",263,695,60,Core);
 // FX controls are persistent effect-mix parameters, not fake FX switches.
 choice("division",{"1/4","1/8","1/8 dotted","1/16","1/16 dotted","1/32","1/4 triplet","1/8 triplet"},845,376,204,28,Core);
 knob("reverbMix","REV MIX",854,527,56,Core);
 knob("reverbSize","SIZE",933,527,56,Core);
 knob("reverbDecay","DECAY",1009,527,56,Core);
 knob("delayMix","DELAY",1084,527,56,Core);
 // Evolving macro row with genuine host-automatable DSP parameters.
 knob("macroAtmosphere","ATMOS",341,687,67,Core);
 knob("coupledMotion","MOTION",430,687,67,Core);
 knob("spectralTilt","TONE",519,687,67,Core);
 knob("biomeDepth","EVOLUTION",606,681,89,Core);
 knob("stereo","DIMENSION",698,687,67,Core);
 knob("xenoWrithe","TEXTURE",791,687,67,Core);
 knob("clarity","CLARITY",878,687,67,Core);
 // Output controls retain the VST3 parameter IDs for old projects.
 choice("mode",{"Polyphonic","Mono","Legato"},921,608,154,26,Core);
 knob("volume","VOLUME",965,685,84,Core);
 knob("pan","PAN",1144,692,60,Core);
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
 keys->setColour(MidiKeyboardComponent::whiteNoteColourId,Colour::fromRGB(222,240,226));
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
 // Raised smoky-glass surface, chrome trim and animated organic corners.
 const float x=box.getX(),y=box.getY(),w=box.getWidth(),h=box.getHeight();
 g.setColour(Colour::fromRGB(0,3,2).withAlpha(.93f));
 g.fillRoundedRectangle(box,14.f);
 ColourGradient glass(Colour::fromRGB(17,38,27).withAlpha(.80f),x,y,
                      Colour::fromRGB(1,7,4).withAlpha(.93f),x+w*.6f,y+h,true);
 g.setGradientFill(glass);g.fillRoundedRectangle(box.reduced(3.f),12.f);
 g.setColour(frost.withAlpha(.32f));
 g.drawRoundedRectangle(box.reduced(.6f),14.f,1.1f);
 g.setColour(grey.withAlpha(.32f));
 g.drawRoundedRectangle(box.reduced(4.f),11.f,1.f);
 g.setColour(accent.withAlpha(.54f));
 g.drawRoundedRectangle(box.reduced(6.f),9.f,1.1f);
 // Biomechanical ribs avoid buttons: only around panel borders.
 for(int i=0;i<9;++i){
  const float yy=y+12.f+i*(h-24.f)/8.f;
  const float reach=5.f+5.f*std::sin(i*1.9f+clock*.55f);
  Path left;left.startNewSubPath(x+4.f,yy);
  left.quadraticTo(x-3.f,yy+3.f,x+reach,yy+8.f);
  g.setColour((i%3?frost:accent).withAlpha(.12f));
  g.strokePath(left,PathStrokeType(1.1f));
  Path right;right.startNewSubPath(x+w-4.f,yy);
  right.quadraticTo(x+w+3.f,yy-3.f,x+w-reach,yy+8.f);
  g.strokePath(right,PathStrokeType(1.f));
 }
 // Reference-style luminous divider below the section title.
 g.setColour(Colour::fromRGB(3,10,6).withAlpha(.88f));
 g.fillRoundedRectangle(x+8.f,y+7.f,w-16.f,30.f,10.f);
 g.setColour(accent.withAlpha(.67f));
 g.drawLine(x+19.f,y+37.f,x+w-19.f,y+37.f,1.f);
 g.setColour(accent.withAlpha(.77f));
 g.fillEllipse(x+15.f,y+16.f,10.f,10.f);
 g.setColour(frost);
 g.setFont(Font("Consolas",13.f,Font::bold));
 g.drawFittedText(name,(int)(x+34.f),(int)(y+10.f),(int)(w-58.f),22,Justification::centredLeft,1);
}
void VoidcoreEditor::drawBase(Graphics& g){
 g.fillAll(voidBlack);
 ColourGradient nebula(Colour::fromRGB(17,42,24),W*.49f,H*.32f,
                      Colour::fromRGB(0,1,1),W*.78f,H*.91f,true);
 g.setGradientFill(nebula);g.fillRect(0,0,(int)W,(int)H);
 // Distant galactic clouds, sparse to leave readability intact.
 for(int c=0;c<15;++c){
  const float x=60.f+std::fmod(c*173.7f,W-100.f);
  const float y=70.f+std::fmod(c*107.3f,H-180.f);
  const float pulse=.5f+.5f*std::sin(clock*.32f+c*1.6f);
  const float r=45.f+15.f*(c%4);
  ColourGradient vapour(acid.withAlpha(.025f+.018f*pulse),x,y,
                         voidBlack.withAlpha(0.f),x+r,y+r,true);
  g.setGradientFill(vapour);g.fillEllipse(x-r,y-r,2.f*r,2.f*r);
 }
 for(int k=0;k<175;++k){
  const float x=13.f+std::fmod(k*193.47f,W-26.f);
  const float y=8.f+std::fmod(k*79.23f,H-18.f);
  const float twinkle=.5f+.5f*std::sin(clock*(.28f+.023f*(k%5))+k*2.1f);
  g.setColour((k%7==0?frost:acid).withAlpha(.045f+.24f*twinkle));
  const float dot=(k%17==0)?2.4f:1.f;
  g.fillEllipse(x,y,dot,dot);
 }
 // Deep graphite outer bezel and crossing alien filaments.
 g.setColour(Colour::fromRGB(3,9,6));g.drawRoundedRectangle(4.f,4.f,W-8.f,H-8.f,19.f,6.f);
 g.setColour(frost.withAlpha(.48f));g.drawRoundedRectangle(8.f,8.f,W-16.f,H-16.f,17.f,1.3f);
 g.setColour(acid.withAlpha(.41f));g.drawRoundedRectangle(12.f,12.f,W-24.f,H-24.f,14.f,1.2f);
 for(int k=0;k<19;++k){
  const float x=27.f+61.f*k;
  Path rib;
  rib.startNewSubPath(x,-5.f);
  rib.cubicTo(x-36.f,114.f,x+58.f,170.f,x-8.f,330.f);
  g.setColour((k%5==0?frost:acid).withAlpha(.024f));
  g.strokePath(rib,PathStrokeType(1.f));
 }
 // Astral Bloom reference: integrated title plaque and black-glass command bar.
 Rectangle<float> title{18.f,15.f,359.f,51.f};
 g.setColour(Colour::fromRGB(2,7,5));g.fillRoundedRectangle(title,10.f);
 g.setColour(frost.withAlpha(.43f));g.drawRoundedRectangle(title,10.f,1.25f);
 g.setFont(Font("Consolas",32.f,Font::plain));
 g.setColour(frost);g.drawFittedText("V O I D C O R E",26,15,341,38,Justification::centred,1);
 g.setFont(Font(9.f,Font::bold));g.setColour(acid.withAlpha(.82f));
 g.drawFittedText("S Y N T H E S I S   B E Y O N D   M A T T E R",38,50,320,13,Justification::centred,1);
 g.setColour(Colour::fromRGB(2,8,5));
 g.fillRoundedRectangle(387.f,12.f,802.f,51.f,11.f);
 g.setColour(frost.withAlpha(.30f));g.drawRoundedRectangle(387.f,12.f,802.f,51.f,11.f,1.2f);
 g.setColour(acid.withAlpha(.13f));
 g.drawHorizontalLine(65,20.f,1180.f);
 // Working preset operations render as JUCE child buttons atop this bezel.
 g.setColour(acid.withAlpha(.26f));g.drawLine(394.f,63.f,1175.f,63.f,1.1f);
 for(int k=0;k<6;++k){
  const float x=26.f+k*215.f;
  g.setColour(acid.withAlpha(.08f));
  g.drawLine(x,75.f,x+22.f,77.f,1.1f);
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
   g.setColour(Colour::fromRGB(24,45,31));
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
 g.setColour(Colour::fromRGB(2,7,4));
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
 ColourGradient ball(Colour::fromRGB(45,126,66).withAlpha(.90f),
                      x-r*.56f,y-r*.76f,voidBlack,x+r*.78f,y+r*.85f,true);
 g.setGradientFill(ball);g.fillEllipse(x-r,y-r,2.f*r,2.f*r);
 // Dark shadow inside the lower-right hemisphere gives planet curvature.
 ColourGradient edge(Colour::fromRGB(0,3,2).withAlpha(0.f),
                     x-r*.43f,y-r*.55f,
                     Colour::fromRGB(0,1,1).withAlpha(.87f),x+r*.56f,y+r*.71f,true);
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
 g.setColour(Colour::fromRGB(0,2,2).withAlpha(.92f));
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
 g.setColour(Colour::fromRGB(2,13,11));g.fillRoundedRectangle(r,5.f);
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
 g.setColour(voidBlack.withAlpha(.95f));g.fillRoundedRectangle(r,5.f);
 g.setColour(grey.withAlpha(.25f));g.drawRoundedRectangle(r,5.f,1.f);
 const unsigned start=processor.scopeCursor.load(std::memory_order_relaxed);
 Path waveform;
 for(int i=0;i<128;++i){
  const float raw=processor.scope[(size_t)((start+i)%128u)].load(std::memory_order_relaxed);
  const float x=r.getX()+r.getWidth()*i/127.f;
  const float y=r.getCentreY()-raw*r.getHeight()*.62f;
  if(!i)waveform.startNewSubPath(x,y);else waveform.lineTo(x,y);
 }
 neonLine(g,waveform,c.withAlpha(.75f),1.1f);
 (void)mode;
}
void VoidcoreEditor::env(Graphics& g,Rectangle<float> r){
 g.setColour(Colour::fromRGB(3,17,14));g.fillRoundedRectangle(r,5.f);
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
 g.setColour(Colour::fromRGB(2,15,12));g.fillRoundedRectangle(r,5.f);
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
 g.setColour(Colour::fromRGB(2,14,10));g.fillRoundedRectangle(r,5.f);
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
 g.setColour(Colour::fromRGB(3,13,10));g.fillRoundedRectangle(r,5.f);
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
 // Exact reference composition: upper-left twin source organisms, dominant
 // centre orbit, right filter/FX; lower motion/env, macros, output and keyboard.
 panel(g,{17,81,378,271},"OSC  /  SOURCE",acid);
 panel(g,{817,81,369,237},"FILTER",acid);
 panel(g,{17,356,378,198},"MOTION  /  LFO",acid);
 panel(g,{817,325,369,229},"SPACE  /  FX",acid);
 panel(g,{17,562,289,168},"ENVELOPE  /  AMP",acid);
 panel(g,{893,561,293,169},"OUTPUT",acid);
 panel(g,{314,625,576,105},"LIVING  /  MACROS",acid);
 // The core has its own ornamental framing rather than an opaque panel:
 // distant stars remain visible around its living reactor.
 drawOrbitalFrame(g,{600.f,308.f},172.f);
 visualEye(g,600.f,305.f,150.f);
 // Live sampled source previews (no waveform art baked into the controls).
 sourceWave(g,{31,169,166,91},false);
 sourceWave(g,{212,169,166,91},true);
 g.setColour(acid);g.setFont(Font(12.f,Font::bold));
 g.drawText("A",30,117,20,22,Justification::centred);
 g.drawText("B",205,117,20,22,Justification::centred);
 g.setColour(frost.withAlpha(.74f));g.setFont(Font(9.f));
 g.drawFittedText("SOURCE A",40,260,148,18,Justification::centred,1);
 g.drawFittedText("SOURCE B",216,260,148,18,Justification::centred,1);
 g.setColour(frost);g.setFont(Font(10.f,Font::bold));
 g.drawText("A",24,324,28,20,Justification::centred);
 g.drawText("B",370,324,21,20,Justification::centred);
 g.drawText("MORPH",168,322,68,19,Justification::centred);
 // The actual APVTS sliders occupy the entire A--B morph rail.
 // The filter graph uses the selected cutoff/resonance parameters.
 filter(g,{986,176,113,77});
 lfo(g,{38,428,326,76},0);
 env(g,{33,599,255,64});
 effects(g,{836,415,332,78},0);
 // Animated sound-sphere in the FX space visualizer: decorative, not a
 // fake clickable control. The four FX knobs beneath are real sliders.
 soundSphere(g,{1101.f,441.f},31.f,acid,smoothedPeak);
 // The actual processor scope supplies moving audio points, not a loop GIF.
 scope(g,{338,577,547,41},0,acid);
 g.setColour(frost.withAlpha(.77f));g.setFont(Font(10.f,Font::bold));
 g.drawText("LIVE AUDIO  /  XENO SPECTRUM",407,558,408,16,Justification::centred);
 // Physical left/right output meter magnitudes from the audio processor.
 const Rectangle<float> meter{1061.f,642.f,48.f,76.f};
 g.setColour(Colour::fromRGB(2,11,7));g.fillRoundedRectangle(meter,3.f);
 g.setColour(frost.withAlpha(.25f));g.drawRoundedRectangle(meter,3.f,1.f);
 for(int ch=0;ch<2;++ch){
  const float magnitude=std::sqrt(clamp01(meterLevels[(size_t)ch]));
  for(int seg=0;seg<18;++seg){
   const bool active=(float)seg/18.f<=magnitude;
   g.setColour((seg>=15?frost:acid).withAlpha(active?.90f:.10f));
   g.fillRoundedRectangle(1066.f+ch*21.f,708.f-seg*3.4f,13.f,2.6f,1.f);
  }
 }
 g.setColour(frost);g.setFont(Font(9.f,Font::bold));
 g.drawText("L",1066,718,13,10,Justification::centred);
 g.drawText("R",1087,718,13,10,Justification::centred);
 drawKeyboardShell(g);
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
 const float tempo=processor.displayTempo.load(std::memory_order_relaxed);
 g.setColour(acid.withAlpha(.58f));g.setFont(Font(11.f,Font::bold));
 g.drawText(String::formatted("%.1f BPM",tempo),42.f,765.f,135.f,19.f,Justification::centredLeft);
 g.setColour(playing?acid:grey);
 g.drawText(playing?"● LIVE":"● IDLE",200.f,765.f,96.f,19.f,Justification::centredLeft);
 g.setColour(frost.withAlpha(.75f));
 g.drawFittedText("VOIDCORE  /  MATRIX XENO ENGINE  /  LIVE JUCE VST3",675,761,490,24,Justification::centredRight,1);
}
void VoidcoreEditor::paint(Graphics& g){
 Graphics::ScopedSaveState state(g);
 g.addTransform(AffineTransform::scale((float)getWidth()/W,(float)getHeight()/H));
 drawBase(g);
 if(page==Core)drawCore(g);else drawDeep(g);
 paintStatus(g);
}
