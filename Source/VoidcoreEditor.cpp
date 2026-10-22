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
 const auto r=getLocalBounds().toFloat();
 const float w=r.getWidth(),h=r.getHeight();
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
 g.setColour(blood);g.drawRoundedRectangle(x-5.f,y-14.f,10.f,28.f,2.f,1.5f);
}
void VoidcoreEditor::fader(const char* id,float x,float y,float w,float h,Page pg){
 auto c=std::make_unique<Fader>(processor,id);
 addAndMakeVisible(*c);
 ui.push_back({std::move(c),{x,y,w,h},(int)pg});
}
void VoidcoreEditor::buildCore(){
 for(int i=0;i<4;++i){
  const Page target=(Page)i;
  allClick(pageNames[(size_t)i],667.f+i*120.f,29.f,110.f,33.f,[this,target]{setPage(target);});
  pageButtons[(size_t)i]=dynamic_cast<Click*>(ui.back().control.get());
 }
 allClick("SAVE",437,73,75,28,[this]{save();});
 allClick("LOAD",522,73,75,28,[this]{load();});
 allClick("MUTATE",607,73,88,28,[this]{mutation(false);},1);
 allClick("EVOLVE",705,73,86,28,[this]{mutation(true);});
 // Main page: only the controls that actually shape the instrument.
 StringArray banks;
 for(const auto& bank:noctivore::banks)banks.add(bank.name);
 banks.add("User");
 choice("bank",banks,58,177,305,33,Core);
 choice("bankB",banks,838,177,305,33,Core);
 knob("start","START",112,404,95,Core);
 knob("xenoFormant","THROAT",285,404,95,Core,1);
 knob("texture","TEXTURE",915,404,95,Core);
 knob("xenoCross","COUPLE",1084,404,95,Core,1);
 fader("morph",463,444,274,42,Core);
 const char* ids[]={"cutoff","resonance","creatures","biomeDepth","reverbMix","volume"};
 const char* labels[]={"FILTER","RES","CREATURE","EVOLVE","SPACE","OUTPUT"};
 for(int i=0;i<6;++i)knob(ids[i],labels[i],142.f+182.f*i,600.f,109.f,Core,i==1||i==5);
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
 lastScene=gentle?"EVOLVED ORGANISM":"MUTATED ORGANISM";
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


void VoidcoreEditor::panel(Graphics& g,Rectangle<float> r,const String& name,Colour accent){
 g.setColour(Colour::fromRGB(3,13,10).withAlpha(.96f));
 g.fillRoundedRectangle(r,9.f);
 ColourGradient shine(accent.withAlpha(.21f),r.getX(),r.getY(),Colour::fromRGB(0,10,6).withAlpha(0.f),r.getRight(),r.getBottom(),false);
 g.setGradientFill(shine);
 g.fillRoundedRectangle(r.removeFromTop(37.f),9.f);
 g.setColour(accent.withAlpha(.53f));g.drawRoundedRectangle(r.getUnion({r.getX(),r.getY()-37.f,r.getWidth(),37.f}),9.f,1.1f);
 g.setColour(accent.withAlpha(.95f));
 g.setFont(Font(14.f,Font::bold));
 g.drawText(name,r.getX()+17.f,r.getY()-29.f,r.getWidth()-34.f,19.f,Justification::centredLeft);
}
void VoidcoreEditor::drawBase(Graphics& g){
 g.fillAll(voidBlack);
 ColourGradient bg(Colour::fromRGB(10,34,23),0.f,80.f,Colour::fromRGB(4,7,7),W,H,false);
 g.setGradientFill(bg);g.fillRect(0,0,(int)W,(int)H);
 // Uneven bio-mechanical ribbing; procedurally drawn to avoid phantom knobs
 // and ensure every real control aligns with the hit-test geometry.
 for(int k=0;k<30;++k){
  const float t=(float)k/30.f,alpha=.024f+.02f*std::sin(k*1.67f);
  Path a;a.startNewSubPath(0.f,120.f+24.f*k);
  a.cubicTo(130.f,142.f+20.f*k,405.f,88.f+25.f*k,W,126.f+22.f*k);
  g.setColour((k%4==0?blood:acid).withAlpha(alpha));g.strokePath(a,PathStrokeType(k%3==0?2.1f:1.1f));
 }
 for(int k=0;k<23;++k){
  const float x=15.f+52.f*k;
  Path a;a.startNewSubPath(x,140.f);a.cubicTo(x-35.f,270.f,x+49.f,410.f,x-12.f,725.f);
  g.setColour((k%3==0?blood:grey).withAlpha(.065f));
  g.strokePath(a,PathStrokeType(1.15f));
 }
 // Title plaque, engraved sci-fi typography; icy rather than whimsical.
 g.setColour(Colour::fromRGB(2,9,7));g.fillRoundedRectangle(23.f,16.f,375.f,106.f,9.f);
 g.setColour(acid.withAlpha(.55f));g.drawRoundedRectangle(23.f,16.f,375.f,106.f,9.f,1.2f);
 g.setColour(frost);
 g.setFont(Font("Consolas",41.f,Font::bold));
 g.drawFittedText("V O I D C O R E",41,23,350,50,Justification::centredLeft,1);
 g.setColour(acid);g.setFont(Font(12.f,Font::bold));
 g.drawText("S P E C T R A L   O R G A N I S M",43,80,340,18,Justification::centredLeft);
 g.setColour(grey.withAlpha(.64f));
 g.drawLine(405.f,60.f,633.f,60.f,1.f);
 g.setColour(blood.withAlpha(.43f));g.drawLine(405.f,112.f,1167.f,112.f,1.5f);
 g.setColour(frost);g.setFont(Font(12.f));
 g.drawFittedText(lastScene,407,28,218,24,Justification::centred,1);
 // Thin corner claws shift slowly but never cover click areas.
 for(int k=0;k<12;++k){
  const float a=(float)k*pi/6.f+clock*.022f;
  const float radius=12.f+5.f*std::sin(clock*.75f+k);
  g.setColour((k%2?acid:blood).withAlpha(.25f));
  const float cx=k%2?1185.f:14.f,cy=150.f+(k/2)*106.f;
  g.drawLine(cx,cy,cx+radius*std::cos(a),cy+radius*std::sin(a),1.5f);
 }
}
void VoidcoreEditor::visualEye(Graphics& g,float x,float y,float radius){
 const float pulse=.5f+.5f*std::sin(clock*1.3f);
 const float audio=std::sqrt(clamp01(smoothedPeak*2.5f));
 const float outer=radius*(1.f+.03f*pulse+.07f*audio);
 for(int k=0;k<7;++k){
  const float r=outer*(.31f+.11f*k);
  g.setColour((k%3==0?blood:acid).withAlpha(.08f+.11f*pulse));
  g.drawEllipse(x-r,y-r,r*2.f,r*2.f,k%2?1.f:2.4f);
 }
 for(int k=0;k<20;++k){
  const float theta=(float)k*2.f*pi/20.f+clock*.10f*(k%2?1.f:-1.f);
  const float r0=outer*.47f,r1=outer*(.8f+.16f*std::sin(k*1.43f+clock*.51f));
  g.setColour((k%3==0?blood:acid).withAlpha(.25f+.15f*audio));
  g.drawLine(x+r0*std::cos(theta),y+r0*std::sin(theta),x+r1*std::cos(theta+.23f),y+r1*std::sin(theta+.23f),1.1f);
 }
 Path almond;
 almond.startNewSubPath(x-outer*.78f,y);
 almond.cubicTo(x-outer*.25f,y-outer*.77f,x+outer*.38f,y-outer*.83f,x+outer*.78f,y);
 almond.cubicTo(x+outer*.38f,y+outer*.83f,x-outer*.25f,y+outer*.77f,x-outer*.78f,y);
 almond.closeSubPath();
 ColourGradient skin(blood.withAlpha(.28f),x,y-outer,Colour::fromRGB(25,6,11),x,y+outer,false);
 g.setGradientFill(skin);g.fillPath(almond);
 g.setColour(blood.withAlpha(.91f));g.strokePath(almond,PathStrokeType(2.f));
 const float iris=outer*(.19f+.035f*pulse+.04f*audio);
 g.setColour(blood.withAlpha(.48f));g.fillEllipse(x-iris*1.6f,y-iris*1.6f,iris*3.2f,iris*3.2f);
 g.setColour(Colour::fromRGB(6,12,10));g.fillEllipse(x-iris,y-iris,iris*2.f,iris*2.f);
 g.setColour(acid.withAlpha(.78f));g.drawEllipse(x-iris,y-iris,iris*2.f,iris*2.f,1.8f);
 const float pupilW=iris*.28f;
 g.setColour(blood);g.fillEllipse(x-pupilW,y-iris*1.55f,2*pupilW,iris*3.1f);
 g.setColour(Colour::fromRGB(255,194,174));g.drawLine(x,y-iris*1.20f,x,y+iris*1.2f,2.f);
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
 panel(g,{34,142,350,321},"SOURCE A  /  ORGANISM",acid);
 panel(g,{816,142,350,321},"SOURCE B  /  MATERIAL",blood);
 panel(g,{401,142,398,365},"NEURAL MORPH",acid);
 sourceWave(g,{57,230,305,131},false);
 sourceWave(g,{837,230,306,131},true);
 visualEye(g,600.f,297.f,135.f);
 g.setColour(frost);g.setFont(Font(14.f,Font::bold));
 g.drawText("A",444,448,24,32,Justification::centred);
 g.drawText("B",733,448,24,32,Justification::centred);
 panel(g,{35,510,1131,161},"PERFORMANCE MACROS",blood);
 scope(g,{42,689,1068,57},0,acid);
 const float peak=std::sqrt(clamp01(smoothedPeak));
 g.setColour(grey.withAlpha(.7f));g.drawRoundedRectangle(1120.f,688.f,39.f,59.f,4.f,1.f);
 for(int i=0;i<15;++i){
  const bool lit=(float)i<peak*15.f;
  g.setColour((i>11?blood:acid).withAlpha(lit?.96f:.10f));
  g.fillRoundedRectangle(1127.f,738.f-i*3.3f,25.f,2.8f,1.f);
 }
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
 g.drawFittedText("VOIDCORE  /  REAL-TIME ORGANISM ENGINE",745,761,420,24,Justification::centredRight,1);
}
void VoidcoreEditor::paint(Graphics& g){
 Graphics::ScopedSaveState state(g);
 g.addTransform(AffineTransform::scale((float)getWidth()/W,(float)getHeight()/H));
 drawBase(g);
 if(page==Core)drawCore(g);else drawDeep(g);
 paintStatus(g);
}
