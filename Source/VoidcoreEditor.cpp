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
 for(auto& node:ui)node.control->setVisible(node.onPage<0||node.onPage==(int)page||
   (page==Motion&&node.onPage==10+focusLfo));
 for(int i=0;i<4;++i){
  if(pageButtons[(size_t)i])pageButtons[(size_t)i]->selected=(int)page==i;
  if(lfoButtons[(size_t)i])lfoButtons[(size_t)i]->selected=focusLfo==i;
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
   if(steps[(size_t)k])steps[(size_t)k]->selected=value(String::formatted("gateStep%02d",k+1).toRawUTF8())>.5f;
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
  knob(rate.toRawUTF8(),"RATE",144,394,110,sub);
  knob(depth.toRawUTF8(),"DEPTH",316,394,110,sub,1);
  choice(wave.toRawUTF8(),{"Sine","Triangle","Saw","Square","Sample & Hold","Fractal"},62,318,170,32,sub);
  choice(sync.toRawUTF8(),{"Free","1 bar","1/2 bar","1/4 bar","1/8 bar","1/16 bar"},255,318,153,32,sub);
  choice(target.toRawUTF8(),{"Volume","Pan","Pitch","Filter 1","Filter 2","FM Index","A/B Morph","Metal Exciter","Grain Density","Delay Warp","Reverb Motion"},62,354,350,30,sub);
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
 choice("quality",{"Linear","Cubic"},630,240,220,40,System);
 choice("reverse",{"Forward","Reverse"},881,240,228,40,System);
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
