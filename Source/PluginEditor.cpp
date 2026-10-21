#include "PluginEditor.h"
#include <algorithm>
#include <cmath>
#include <functional>
using namespace juce;
namespace {
 using namespace painted;
 Rectangle<int> rect(float x,float y,float w,float h,float sx,float sy){return {(int)std::round(x*sx),(int)std::round(y*sy),(int)std::round(w*sx),(int)std::round(h*sy)};}
}
NoctivoreEditor::Skin::Skin(){
 setColour(Slider::textBoxBackgroundColourId,painted::soil.withAlpha(.7f));
 setColour(Slider::textBoxOutlineColourId,painted::outline.withAlpha(.3f));
 setColour(Slider::textBoxTextColourId,painted::ivory);
 setColour(ComboBox::backgroundColourId,painted::bark);
 setColour(ComboBox::outlineColourId,painted::outline);
 setColour(ComboBox::textColourId,painted::ivory);
 setColour(ComboBox::arrowColourId,painted::ochre);
 setColour(PopupMenu::backgroundColourId,painted::bark);
 setColour(PopupMenu::textColourId,painted::ivory);
 setColour(PopupMenu::highlightedBackgroundColourId,painted::clay.darker(.45f));
 setColour(TextButton::textColourOffId,painted::ivory);
 setColour(TextButton::textColourOnId,painted::soil);
}
void NoctivoreEditor::Skin::drawRotarySlider(Graphics& g,int x,int y,int w,int h,float value,float start,float end,Slider& slider){painted::rotary(g,x,y,w,h,value,start,end,painted::accents[(size_t)std::clamp((int)slider.getProperties().getWithDefault("tint",0),0,4)]);}
void NoctivoreEditor::Skin::drawButtonBackground(Graphics& g,Button& button,const Colour&,bool hovered,bool down){painted::button(g,button,hovered,down,painted::accents[(size_t)std::clamp((int)button.getProperties().getWithDefault("tint",0),0,4)]);}
void NoctivoreEditor::Skin::drawComboBox(Graphics& g,int w,int h,bool,int,int,int,int,ComboBox&){painted::selector(g,w,h);}

NoctivoreEditor::ArtKnob::ArtKnob(AudioProcessorValueTreeState& st,const String& id,const String& label){
 setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
 setTextBoxStyle(Slider::NoTextBox,false,0,0);
 setMouseDragSensitivity(250);
 setRotaryParameters(MathConstants<float>::pi*1.18f,MathConstants<float>::pi*2.82f,true);
 setTooltip(label+" — drag to edit");
 if(auto* p=st.getParameter(id))setDoubleClickReturnValue(true,p->convertFrom0to1(p->getDefaultValue()));
 attachment=std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(st,id,*this);
}
void NoctivoreEditor::ArtKnob::paint(Graphics& g){
 auto r=getLocalBounds().toFloat();float cx=r.getCentreX(),cy=r.getCentreY(),rad=.44f*std::min(r.getWidth(),r.getHeight());
 float norm=getMaximum()>getMinimum()?(float)((getValue()-getMinimum())/(getMaximum()-getMinimum())):0.f;
 norm=std::clamp(norm,0.f,1.f);
 float first=MathConstants<float>::pi*1.18f,last=MathConstants<float>::pi*2.82f;
 Path p;p.addCentredArc(cx,cy,rad,rad,0,first,first+norm*(last-first),true);
 g.setColour(Colour::fromRGB(242,197,105).withAlpha(isMouseOverOrDragging()?.92f:.34f));
 g.strokePath(p,PathStrokeType(isMouseOverOrDragging()?2.3f:1.1f));
 if(isMouseOverOrDragging()){
   g.setColour(Colour::fromRGB(20,23,17).withAlpha(.92f));
   g.fillRoundedRectangle(0.f,0.f,r.getWidth(),16.f,3.f);
   g.setColour(Colour::fromRGB(235,218,174));g.setFont(Font(10.f,Font::bold));
   g.drawText(String(getValue(),2),0,0,getWidth(),16,Justification::centred);
 }
}
NoctivoreEditor::ArtMenu::ArtMenu(AudioProcessorValueTreeState& st,const String& id,const StringArray& names){
 for(auto name:names)addItem(name,getNumItems()+1);
 attachment=std::make_unique<AudioProcessorValueTreeState::ComboBoxAttachment>(st,id,*this);
}
NoctivoreEditor::Dial::Dial(AudioProcessorValueTreeState& apvts,const String& id,const String& text,int colour){
 knob.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
 knob.setRotaryParameters(MathConstants<float>::pi*1.2f,MathConstants<float>::pi*2.8f,true);
 knob.setTextBoxStyle(Slider::TextBoxBelow,false,85,17);
 knob.setMouseDragSensitivity(240);
 knob.getProperties().set("tint",colour);
 auto* parameter=apvts.getParameter(id);
 if(parameter)knob.setDoubleClickReturnValue(true,parameter->convertFrom0to1(parameter->getDefaultValue()));
 addAndMakeVisible(knob);
 name.setText(text,dontSendNotification);
 name.setJustificationType(Justification::centred);name.setFont(Font(10.6f,Font::bold));
 name.setColour(Label::textColourId,soft);addAndMakeVisible(name);
 attachment=std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(apvts,id,knob);
}
void NoctivoreEditor::Dial::resized(){auto r=getLocalBounds();name.setBounds(r.removeFromTop(17));knob.setBounds(r);}
NoctivoreEditor::Selector::Selector(AudioProcessorValueTreeState& apvts,const String& id,const String& text,const StringArray& names){
 name.setText(text,dontSendNotification);name.setJustificationType(Justification::centredLeft);
 name.setFont(Font(10.5f,Font::bold));name.setColour(Label::textColourId,soft);
 addAndMakeVisible(name);
 for(auto item:names)box.addItem(item,box.getNumItems()+1);
 addAndMakeVisible(box);
 attachment=std::make_unique<AudioProcessorValueTreeState::ComboBoxAttachment>(apvts,id,box);
}
void NoctivoreEditor::Selector::resized(){auto r=getLocalBounds().reduced(4,1);name.setBounds(r.removeFromTop(17));box.setBounds(r.reduced(0,2));}
NoctivoreEditor::Section& NoctivoreEditor::section(Page p,const String& name,Colour colour,int columns){
 sections.push_back({p,name,colour,columns,{}, {}});
 return sections.back();
}
void NoctivoreEditor::dial(Section& s,const char* id,const char* name,int colour){
 auto c=std::make_unique<Dial>(processor.state,id,name,colour);
 s.children.push_back(c.get());addAndMakeVisible(*c);controls.push_back(std::move(c));
}
void NoctivoreEditor::select(Section& s,const char* id,const char* label,StringArray opts){
 auto c=std::make_unique<Selector>(processor.state,id,label,opts);
 s.children.push_back(c.get());addAndMakeVisible(*c);controls.push_back(std::move(c));
}
NoctivoreEditor::NoctivoreEditor(NoctivoreProcessor& p)
 :AudioProcessorEditor(&p),processor(p),keyboard(p.keyboardState,MidiKeyboardComponent::horizontalKeyboard){
 setLookAndFeel(&skin);
 originalPainting=ImageFileFormat::loadFrom(processor.paintingFile());
 // No resizing until EVERY child and attachment is constructed: essential
 // for Ableton Live, which may request editor layout immediately on creation.
 addAndMakeVisible(keyboard);keyboard.setAvailableRange(24,96);
 keyboard.setKeyWidth(21.f);keyboard.setScrollButtonsVisible(false);
 StringArray bankNames;for(auto& b:noctivore::banks)bankNames.add(b.name);bankNames.add("User Bank");
 auto& a=section(Atlas,"01 / SPORE ATLAS",moss,3);
 select(a,"bank","SOURCE A — MATERIAL",bankNames);
 select(a,"bankB","SOURCE B — COUNTERPOINT",bankNames);
 select(a,"textureMode","TEXTURE ENGINE",{"Flow","Grain Cloud","Orbital","Pulsar","Fracture","Time Freeze"});
 dial(a,"morph","A / B MORPH",0);dial(a,"morphMotion","AUTO MORPH",0);dial(a,"morphRate","MORPH SPEED",0);
 dial(a,"texture","TEXTURE",0);dial(a,"grain","GRAIN CLOUD",0);dial(a,"scatter","SCATTER",0);
 auto& b=section(Atlas,"02 / LIVING BIOME",ochre,3);
 dial(b,"organic","ORGANIC DRIFT",2);dial(b,"layers","LIVING LAYERS",2);dial(b,"movement","MICRO MOTION",2);
 dial(b,"biomeDepth","BIOME LIFE",2);dial(b,"biomeRate","BIOME SPEED",2);
 dial(b,"biomeChaos","BIOME COMPLEXITY",2);
 dial(b,"sub","SUB BODY",2);dial(b,"pulseRate","PULSE BEATS",2);dial(b,"spectralTilt","DARK / BRIGHT",2);
 dial(b,"stereo","WIDTH",2);dial(b,"drive","SATURATION",2);dial(b,"volume","OUTPUT",2);
 auto& x=section(Atlas,"03 / CREATURE MORPHOLOGY",slate,3);
 dial(x,"xenoFormant","ALIEN VOCAL TRACT",3);
 dial(x,"xenoCross","ORGANISM COUPLING",3);
 dial(x,"xenoWrithe","BODY MUTATION",3);
 dial(x,"lowRegisterLife","DEEP LIFE",3);
 dial(x,"coupledMotion","LINKED MOTION",3);
 auto& c=section(Atlas,"04 / FOREST RHYTHMS",clay,3);
 select(c,"motif","GATE PATTERN",{"Off","Underbrush","Offbeat","Tribal","Broken","Breathing","Shadow"});
 select(c,"motifDivision","SYNC GRID",{"1/8","1/16","1/32","Triplet"});
 dial(c,"motifPitch","GROOVE VARIATION",1);
 dial(c,"motifGate","RHYTHMIC GATE",1);dial(c,"motifAccent","ODD ACCENTS",1);
 dial(c,"motifGlide","GATE SWING",1);dial(c,"motifFX","RESONANT EVENTS",1);dial(c,"mudCut","MUD CONTROL",1);
 dial(c,"wetLowCut","FX LOW CUT",1);dial(c,"focus","STEREO FOCUS",1);
 // Option B: deep metal/FM/filter sound design grouped in three sculpted tiles.
 auto& f=section(Forge,"01 / ALIEN FM + RESONANCE",clay,3);
 dial(f,"fmMix","FM LAYER",1);dial(f,"fmRatio","RATIO",1);dial(f,"fmIndex","INDEX",1);
 dial(f,"fmFeedback","FEEDBACK",1);dial(f,"fmInharmonic","INHARMONIC",1);
 dial(f,"metalMix","RESONATOR",1);dial(f,"metalTune","RESONANCE TUNE",1);
 dial(f,"metalDecay","RING TIME",1);dial(f,"metalTone","MATERIAL",1);
 auto& fi=section(Forge,"02 / DUAL FILTER NETWORK",moss,3);
 select(fi,"filterMode","FILTER 1",{"Low Pass","Band Pass","High Pass"});
 select(fi,"filter2Mode","FILTER 2",{"Off","Low Pass","Band Pass","High Pass","Notch"});
 select(fi,"filterRoute","ROUTING",{"Serial","Parallel","Split"});
 dial(fi,"cutoff","CUTOFF A",0);dial(fi,"resonance","RESONANCE A",0);
 dial(fi,"cutoff2","CUTOFF B",0);dial(fi,"resonance2","RESONANCE B",0);
 dial(fi,"filterBlend","A / B BLEND",0);dial(fi,"filterDrive","FILTER DRIVE",0);
 dial(fi,"keytrack","KEY TRACK",0);
 auto& e=section(Forge,"03 / SHAPING ENVELOPES",ochre,3);
 dial(e,"filterEnvAttack","F. ATTACK",2);dial(e,"filterEnvDecay","F. DECAY",2);
 dial(e,"filterEnvSustain","F. SUSTAIN",2);dial(e,"filterEnvRelease","F. RELEASE",2);
 dial(e,"filterEnvDepth","F. AMOUNT",2);
 dial(e,"motionEnvAttack","M. ATTACK",2);dial(e,"motionEnvRelease","M. RELEASE",2);
 dial(e,"motionEnvDepth","M. AMOUNT",2);
 StringArray waves{"Sine","Triangle","Saw","Square","Sample & Hold","Fractal"};
 StringArray targets{"Volume","Pan","Pitch","Filter 1","Filter 2","FM Index","A/B Morph","Metal Exciter","Grain Density","Delay Warp","Reverb Motion"};
 StringArray syncs{"Free","1 bar","1/2 bar","1/4 bar","1/8 bar","1/16 bar","2 bars","4 bars","8 bars","16 bars","32 bars"};
 for(int i=1;i<=4;++i){
  auto& l=section(Motion,"0"+String(i)+" / MODULATOR",accents[(size_t)((i+1)%5)],2);
  String prefix=i==1?"":("lfo"+String(i));
  String rate=i==1?"rate":prefix+"Rate",depth=i==1?"depth":prefix+"Depth";
  String wave=i==1?"wave":prefix+"Wave",target=i==1?"target":prefix+"Target";
  String sync="lfo"+String(i)+"Sync";
  dial(l,rate.toRawUTF8(),"RATE / ultra slow",(i+1)%5);
  dial(l,depth.toRawUTF8(),"FINE DEPTH",(i+1)%5);
  select(l,wave.toRawUTF8(),"WAVEFORM",waves);
  select(l,target.toRawUTF8(),"DESTINATION",targets);
  select(l,sync.toRawUTF8(),"TEMPO GRID",syncs);
 }
 auto& de=section(Motion,"05 / MOTION ENVELOPE",ochre,3);
 dial(de,"motionEnvAttack","RISE",2);dial(de,"motionEnvRelease","FALL",2);dial(de,"motionEnvDepth","DEPTH",2);
 auto& psy=section(Space,"00 / HYPNOTIC MOVEMENT",ochre,3);
 dial(psy,"trancePulse","TRANCE PULSE",2);
 dial(psy,"tranceRate","PULSE DIVISION",2);
 dial(psy,"psySpiral","STEREO SPIRAL",2);
 dial(psy,"psySpiralRate","SPIRAL SPEED",2);
 dial(psy,"hiFiAir","AIR / DETAIL",2);
 dial(psy,"clarity","HI-FI CLARITY",2);
 dial(psy,"wetLowCut","FX HIGH PASS",2);
 auto& d=section(Space,"01 / CONSTELLATION DELAY",clay,4);
 select(d,"division","HOST SYNC",{"1/4","1/8","1/8 dotted","1/16","1/16 dotted","1/32","1/4 triplet","1/8 triplet"});
 dial(d,"delayMix","DELAY WET",1);dial(d,"delayFeedback","FEEDBACK",1);
 dial(d,"delayTone","TONE",1);dial(d,"delayWarp","WARP",1);
 dial(d,"delaySpread","PING PONG",1);dial(d,"delayDiffusion","SMEAR",1);
 auto& rv=section(Space,"02 / PSYCHEDELIC REVERB",moss,4);
 dial(rv,"reverbMix","REVERB WET",0);dial(rv,"reverbSize","CHAMBER",0);
 dial(rv,"reverbDecay","DECAY",0);dial(rv,"reverbDark","DARKNESS",0);
 dial(rv,"reverbMod","MUTATION",0);dial(rv,"reverbPreDelay","PRE-DELAY",0);
 dial(rv,"shimmer","BLOOM",0);dial(rv,"duck","FX DUCK",0);
 auto& sy=section(System,"01 / VOICE CONFIGURATION",slate,3);
 select(sy,"mode","VOICE MODE",{"Polyphonic","Mono","Legato"});
 select(sy,"velocity","VELOCITY",{"Soft","Normal","Hard"});
 select(sy,"quality","RESAMPLING",{"Linear","Cubic"});
 dial(sy,"start","START OFFSET",3);dial(sy,"glide","GLIDE",3);
 select(sy,"reverse","SAMPLE PLAYBACK",{"Forward","Reverse"});
 auto& sy2=section(System,"02 / TUNING + CONTROL",ochre,3);
 dial(sy2,"bendRange","BEND RANGE",2);dial(sy2,"bend","PITCH BEND",2);
 dial(sy2,"pan","PAN",2);dial(sy2,"stereo","STEREO WIDTH",2);
 dial(sy2,"space","LEGACY SPACE",2);dial(sy2,"volume","OUTPUT",2);
 auto& sy3=section(System,"03 / AMPLITUDE ENVELOPE",ash,3);
 dial(sy3,"attack","ATTACK",4);dial(sy3,"decay","DECAY",4);dial(sy3,"sustain","SUSTAIN",4);
 dial(sy3,"release","RELEASE",4);dial(sy3,"sub","SUB BODY",4);dial(sy3,"drive","DRIVE",4);
 const char* labels[]={"ATLAS","MORPHOLOGY","4 LFOs","PSY SPACE","UTILITY"};
 for(size_t i=0;i<tabs.size();++i){addAndMakeVisible(tabs[i]);tabs[i].setButtonText(labels[i]);
  tabs[i].getProperties().set("tint",(int)i);tabs[i].setClickingTogglesState(false);
  tabs[i].onClick=[this,i]{showPage((Page)i);};}
 const char* actionLabels[]={"MUTATE","EVOLVE","GROOVE"};
 for(size_t i=0;i<actions.size();++i){addAndMakeVisible(actions[i]);actions[i].setButtonText(actionLabels[i]);
  actions[i].getProperties().set("tint",(int)(i+1));}
 actions[0].onClick=[this]{mutate(false);};actions[1].onClick=[this]{mutate(true);};
 actions[2].onClick=[this]{setParameter("motif",3.f);setParameter("motifDivision",1.f);
   setParameter("motifPitch",.36f);setParameter("motifGate",.72f);
   setParameter("motifAccent",.86f);setParameter("motifFX",0.f);setParameter("wetLowCut",.76f);
   setParameter("reverbMix",.30f);setParameter("morphMotion",.12f);showPage(Atlas);};
 sceneLabel.setText("FOREST SIGNALS",dontSendNotification);sceneLabel.setFont(Font(10.f,Font::bold));
 sceneLabel.setColour(Label::textColourId,soft);addAndMakeVisible(sceneLabel);
 const char* presets[]={"CHOOSE SCENE","FIREFLY BREATH","POND MEMORY","SPORE TELEPATHY","ROOT RADIO","MUSHROOM CHOIR","LIQUID SWARM","FOREST BLEEP","GENTLE CREEP","ALIEN SAP","WET NIGHT SIGNAL","MYCELIUM ECHO","HOLLOW GARDEN"};
 addAndMakeVisible(scenes);for(int i=0;i<13;++i)scenes.addItem(presets[i],i+1);
 scenes.setSelectedId(1,dontSendNotification);
 scenes.onChange=[this]{if(scenes.getSelectedId()>1)applyScene(scenes.getSelectedId()-2);};
 // Always-visible macros. Their controls are real host-automatable parameters.
 auto atm=std::make_unique<Dial>(processor.state,"macroAtmosphere","ATMOSPHERE",0);
 atm->setName("PERMANENT_ATM");addAndMakeVisible(*atm);controls.push_back(std::move(atm));
 auto fury=std::make_unique<Dial>(processor.state,"macroAggression","AGGRESSION",1);
 fury->setName("PERMANENT_FURY");addAndMakeVisible(*fury);controls.push_back(std::move(fury));

 // This is the exact screenshot, not a procedural imitation: all hotspots
 // are positioned in its original 1491x1055 coordinate system.
 auto artDial=[this](const char* id,const char* label,float x,float y,float d=54.f){
   ArtDialSpot a;a.x=x;a.y=y;a.d=d;
   a.knob=std::make_unique<ArtKnob>(processor.state,id,label);
   addAndMakeVisible(*a.knob);artDials.push_back(std::move(a));
 };
 auto artMenu=[this](const char* id,StringArray opts,float x,float y,float w,float h){
   ArtMenuSpot a;a.x=x;a.y=y;a.w=w;a.h=h;
   a.menu=std::make_unique<ArtMenu>(processor.state,id,opts);
   addAndMakeVisible(*a.menu);artMenus.push_back(std::move(a));
 };
 auto artButton=[this](const char* label,float x,float y,float w,float h,std::function<void()> fn){
   ArtButtonSpot a;a.x=x;a.y=y;a.w=w;a.h=h;
   a.button=std::make_unique<ArtHotspot>();a.button->setTooltip(label);
   a.button->onClick=std::move(fn);
   addAndMakeVisible(*a.button);artButtons.push_back(std::move(a));
 };
 StringArray names;for(const auto& b:noctivore::banks)names.add(b.name);names.add("User Bank");
 artMenu("bank",names,390,155,160,33);artMenu("bankB",names,964,155,156,33);
 artMenu("motif",{"Off","Underbrush","Offbeat","Tribal","Broken","Breathing","Shadow"},1164,421,220,28);
 artMenu("filterMode",{"Low Pass","Band Pass","High Pass"},187,796,243,30);
 artMenu("filter2Mode",{"Off","Low Pass","Band Pass","High Pass","Notch"},1004,796,280,30);
 artMenu("division",{"1/4","1/8","1/8 dotted","1/16","1/16 dotted","1/32","1/4 triplet","1/8 triplet"},623,797,238,30);
 artDial("morph","Bank A/B Morph",725,164,66);
 artDial("lowRegisterLife","Deep Life Growth",70,343);
 artDial("sub","Deep Life Depth",137,343);
 artDial("movement","Deep Life Movement",204,343);
 artDial("grain","Deep Life Texture",274,343);
 artDial("morphMotion","Motion Sync",369,341);
 artDial("biomeRate","Motion Rate",432,341);
 artDial("psySpiralRate","Motion Phase",496,341);
 artDial("coupledMotion","Linked Motion",558,341);
 artDial("layers","Biome Density",877,344);
 artDial("texture","Biome Size",944,344);
 artDial("stereo","Biome Spread",1012,344);
 artDial("biomeDepth","Biome Life",1083,344);
 artDial("tranceRate","Gate Rate",1178,340);
 artDial("motifPitch","Gate Length",1247,340);
 artDial("motifGlide","Gate Swing",1315,340);
 artDial("motifGate","Rhythmic Gate",1380,340);
 artDial("xenoFormant","Alien Vocal Formants",135,526);
 artDial("xenoWrithe","Talk",201,526);
 artDial("fmIndex","Harmonics",269,526);
 artDial("xenoCross","Wet",337,526);
 artDial("xenoCross","Coupling",414,606);
 artDial("morph","Organism Transfer",514,606);
 artDial("morphMotion","Variation",588,606);
 artDial("spectralTilt","Body Shape",666,574);
 artDial("grain","Body Stretch",738,574);
 artDial("scatter","Disrupt",808,574);
 artDial("motionEnvRelease","Regenerate",879,574);
 artDial("fmRatio","FM Ratio",969,530);
 artDial("fmIndex","FM Index",1029,530);
 artDial("fmMix","FM Amount",1089,530);
 artDial("fmInharmonic","FM Blend",1146,530);
 artDial("drive","Drive",1320,540);
 artDial("macroAggression","Bite",1390,540);
 artDial("filterDrive","Fuzz",1320,603);
 artDial("spectralTilt","Tone",1392,603);
 artDial("cutoff","Cutoff 1",198,749);
 artDial("resonance","Resonance 1",267,749);
 artDial("filterEnvDepth","Filter 1 Envelope",341,749);
 artDial("filterDrive","Filter 1 Drive",414,749);
 artDial("reverbSize","Atmosphere Size",626,751);
 artDial("reverbDark","Damping",693,751);
 artDial("stereo","Width",762,751);
 artDial("reverbMix","Reverb Mix",830,751);
 artDial("cutoff2","Cutoff 2",1050,752);
 artDial("resonance2","Resonance 2",1118,752);
 artDial("filterBlend","Filter Blend",1191,752);
 artDial("filterDrive","Filter Tone",1263,752);
 const float tx[]={244,445,626,801,1024},tw[]={170,180,170,181,171};
 for(int i=0;i<5;++i)artButton("Switch page",tx[i],852,tw[i],40,[this,i]{showPage((Page)i);});
 artButton("Mutate",876,201,45,42,[this]{mutate(true);});
 artButton("Evolve",830,201,45,42,[this]{mutate(false);});
 showPage(Atlas);
 setResizable(true,true);setResizeLimits(1010,715,1600,1132);
 setSize(1192,844);
 startTimerHz(15);
}
NoctivoreEditor::~NoctivoreEditor(){stopTimer();setLookAndFeel(nullptr);}
void NoctivoreEditor::setParameter(const char* id,float value){
 if(auto* param=processor.state.getParameter(id)){
  param->beginChangeGesture();param->setValueNotifyingHost(param->convertTo0to1(value));param->endChangeGesture();}
}
void NoctivoreEditor::mutate(bool subtle){
 auto& r=Random::getSystemRandom();
 if(!subtle){
  auto a=r.nextInt(50);int b=r.nextInt(49);if(b>=a)++b;
  setParameter("bank",(float)a);setParameter("bankB",(float)b);
  setParameter("textureMode",(float)r.nextInt(6));
  setParameter("fmRatio",.3f+r.nextFloat()*10.f);
  setParameter("fmInharmonic",r.nextFloat());
  setParameter("filter2Mode",(float)(1+r.nextInt(4)));
  setParameter("filterRoute",(float)r.nextInt(3));
 }
 setParameter("motif",(float)(1+r.nextInt(6)));
 setParameter("motifDivision",(float)r.nextInt(4));
 setParameter("motifPitch",.15f+r.nextFloat()*.66f);
 setParameter("motifGate",.22f+r.nextFloat()*.65f);
 setParameter("motifAccent",.25f+r.nextFloat()*.7f);
 setParameter("motifFX",r.nextFloat()*.025f);
 setParameter("wetLowCut",.49f+r.nextFloat()*.45f);
 setParameter("mudCut",.30f+r.nextFloat()*.60f);
 setParameter("reverbMix",.17f+r.nextFloat()*.25f);
 setParameter("morph",.15f+r.nextFloat()*.7f);
 setParameter("morphMotion",.12f+r.nextFloat()*.7f);
 setParameter("morphRate",.007f+r.nextFloat()*.13f);
 setParameter("fmMix",.005f+r.nextFloat()*.065f);
 setParameter("metalMix",r.nextFloat()*.012f);
 setParameter("metalDecay",.32f+r.nextFloat()*.63f);
 setParameter("trancePulse",.14f+r.nextFloat()*.55f);
 setParameter("psySpiral",.14f+r.nextFloat()*.60f);
 setParameter("reverbMod",.2f+r.nextFloat()*.75f);
 setParameter("macroAtmosphere",.2f+r.nextFloat()*.7f);
 setParameter("macroAggression",.03f+r.nextFloat()*.35f);
 for(int i=1;i<=4;++i){
  String base=i==1?"":("lfo"+String(i));
  String rate=i==1?"rate":base+"Rate",depth=i==1?"depth":base+"Depth";
  String target=i==1?"target":base+"Target";
  setParameter(rate.toRawUTF8(),.004f+r.nextFloat()*.38f);
  setParameter(depth.toRawUTF8(),.05f+r.nextFloat()*.58f);
  setParameter(target.toRawUTF8(),(float)(3+r.nextInt(8)));
 }
}
void NoctivoreEditor::applyScene(int which){
 struct Scene {int a,b,algorithm;float fm,metal,index,morph,atm,fury,filter2;};
 static const std::array<Scene,12> data{{
   {0,5,0,.05f,.01f,1.2f,.42f,.82f,.10f,.52f},   // root / insect
   {2,25,2,.06f,.01f,1.2f,.60f,.78f,.12f,.68f}, // vowel / forest
   {7,33,3,.09f,.02f,1.4f,.50f,.76f,.17f,.41f}, // membrane / spectral
   {11,14,2,.06f,.01f,1.5f,.48f,.80f,.15f,.55f},// hollow / radio
   {16,31,4,.08f,.02f,1.7f,.64f,.77f,.17f,.49f},// swarms / current
   {21,38,1,.05f,.01f,1.1f,.50f,.90f,.10f,.66f},// pad / motion
   {24,46,5,.06f,.01f,1.5f,.52f,.84f,.10f,.36f},// scrape / vocal
   {28,43,4,.07f,.03f,1.7f,.62f,.75f,.15f,.47f},// noise / swarm
   {9,1,2,.09f,.03f,2.0f,.35f,.78f,.20f,.61f}, // FM / woods
   {32,8,3,.07f,.01f,1.4f,.46f,.83f,.11f,.55f}, // choir / stone
   {20,2,1,.10f,.04f,1.6f,.47f,.82f,.12f,.62f},// glass / voice
   {41,49,3,.12f,.02f,1.4f,.59f,.80f,.16f,.40f},// cosmic / dark
 }};
 if(which<0||which>=(int)data.size())return;
 const auto& s=data[(size_t)which];
 setParameter("bank",(float)s.a);setParameter("bankB",(float)s.b);
 setParameter("textureMode",(float)s.algorithm);
 setParameter("fmMix",s.fm*.4f);setParameter("metalMix",s.metal*.06f);
 setParameter("fmIndex",s.index);setParameter("morph",s.morph);
 setParameter("macroAtmosphere",s.atm);setParameter("macroAggression",s.fury);
 setParameter("filter2Mode",2.f);setParameter("filterRoute",(float)(which%3));
 setParameter("filterBlend",s.filter2);setParameter("attack",.20f+.05f*(float)(which%4));
 setParameter("release",4.1f+.5f*(float)(which%5));setParameter("reverbMix",.24f+.025f*(float)(which%5));
 setParameter("reverbSize",.85f);setParameter("metalDecay",.60f);
 setParameter("motif",(float)(1+which%6));
 setParameter("motifDivision",(float)(which%4));
 setParameter("motifPitch",.19f+.067f*(float)(which%8));
 setParameter("motifGate",.25f+.065f*(float)(which%7));
 setParameter("motifAccent",.45f+.07f*(float)(which%6));
 setParameter("motifGlide",.08f+.06f*(float)(which%5));
 setParameter("motifFX",0.f);
 setParameter("mudCut",.45f+.04f*(float)(which%6));
 setParameter("wetLowCut",.55f+.05f*(float)(which%7));
 setParameter("focus",.58f+.04f*(float)(which%6));
 setParameter("trancePulse",.06f+.025f*(float)(which%7));
 setParameter("psySpiral",.20f+.042f*(float)(which%6));
 setParameter("clarity",.93f);setParameter("hiFiAir",.33f);
 setParameter("biomeDepth",.54f+.05f*(float)(which%7));
 setParameter("biomeRate",.012f+.004f*(float)(which%7));
 setParameter("biomeChaos",.28f+.11f*(float)(which%6));
 setParameter("lowRegisterLife",.58f+.05f*(float)(which%7));
 setParameter("coupledMotion",.38f+.08f*(float)(which%6));
 for(int i=2;i<=4;++i){auto prefix="lfo"+String(i);
  setParameter((prefix+"Depth").toRawUTF8(),.10f+.04f*(float)i);
  setParameter((prefix+"Rate").toRawUTF8(),.001f+.0022f*(float)i);
 }
 showPage(Atlas);
}
void NoctivoreEditor::showPage(Page next){
 page=next;
 bool art=paintedPage();
 for(auto& s:sections)for(auto* c:s.children)c->setVisible(s.page==page && !art);
 for(auto& a:artDials)a.knob->setVisible(art);
 for(auto& a:artMenus)a.menu->setVisible(art);
 for(auto& a:artButtons)a.button->setVisible(art);
 for(size_t i=0;i<tabs.size();++i){
  tabs[i].setVisible(!art);
  tabs[i].setToggleState((int)i==(int)page,dontSendNotification);
 }
 for(auto& a:actions)a.setVisible(!art);
 scenes.setVisible(!art);sceneLabel.setVisible(!art);
 for(auto& c:controls)if(c->getName().startsWith("PERMANENT_"))c->setVisible(!art);
 keyboard.setAlpha(art?.26f:1.f);
 if(getWidth()>0 && getHeight()>0)resized();
 repaint();
}
void NoctivoreEditor::layout(Section& section_,Rectangle<int> area){
 section_.bounds=area;
 auto inset=area.reduced(9,13);
 inset.removeFromTop(25);
 int count=(int)section_.children.size();
 int cols=std::max(1,section_.columns),rows=std::max(1,(count+cols-1)/cols);
 const int cw=std::max(15,inset.getWidth()/cols),ch=std::max(22,inset.getHeight()/rows);
 for(int i=0;i<count;++i){auto x=inset.getX()+(i%cols)*cw,y=inset.getY()+(i/cols)*ch;
  section_.children[(size_t)i]->setBounds(x,y,cw,ch);}
}
void NoctivoreEditor::resized(){
 if(paintedPage()){
  float sx=(float)getWidth()/1491.f,sy=(float)getHeight()/1055.f;
  auto R=[&](float x,float y,float w,float h){
   return Rectangle<int>(roundToInt(x*sx),roundToInt(y*sy),roundToInt(w*sx),roundToInt(h*sy));};
  for(auto& a:artDials)a.knob->setBounds(R(a.x-a.d*.5f,a.y-a.d*.5f,a.d,a.d));
  for(auto& a:artMenus)a.menu->setBounds(R(a.x,a.y,a.w,a.h));
  for(auto& a:artButtons)a.button->setBounds(R(a.x,a.y,a.w,a.h));
  keyboard.setBounds(R(214,907,1024,94));
  return;
 }

 float sx=getWidth()/1160.f,sy=getHeight()/790.f;
 auto R=[&](float x,float y,float w,float h){return rect(x,y,w,h,sx,sy);};
 for(size_t i=0;i<tabs.size();++i)tabs[i].setBounds(R(18.f+(float)i*143.f,174,137,36));
 actions[0].setBounds(R(778,174,120,36));
 actions[1].setBounds(R(905,174,115,36));
 actions[2].setBounds(R(1026,174,119,36));
 sceneLabel.setBounds(R(27,98,135,18));scenes.setBounds(R(27,116,270,34));
 for(auto& c:controls){
  if(c->getName()=="PERMANENT_ATM")c->setBounds(R(826,79,115,86));
  if(c->getName()=="PERMANENT_FURY")c->setBounds(R(956,79,115,86));
 }
 for(auto& s:sections){
  if(s.page!=page){s.bounds={};continue;}
  int order=0;for(const auto& test:sections){if(&test==&s)break;if(test.page==page)++order;}
  if(page==Atlas){
   int col=order%2,row=order/2;
   layout(s,R(18.f+col*570.f,224.f+row*223.f,554.f,214.f));
  }else if(page==Motion){
   if(order<4)layout(s,R(18.f+order*285.f,224.f,275.f,356.f));
   else layout(s,R(18,590,1124,80));
  }else{
   layout(s,R(18.f+order*379.f,224.f,370.f,442.f));
  }
 }
 keyboard.setBounds(R(26,701,1108,51));
}
void NoctivoreEditor::timerCallback(){repaint();}
void NoctivoreEditor::paint(Graphics& g){
 if(paintedPage()){
  g.drawImage(originalPainting,getLocalBounds().toFloat());
  return;
 }

 g.fillAll(painted::soil);
 const float sx=getWidth()/1160.f,sy=getHeight()/790.f;
 Graphics::ScopedSaveState scope(g);
 g.addTransform(AffineTransform::scale(sx,sy));
 painted::forestBackdrop(g);
 g.setColour(painted::ivory);g.setFont(Font(35.f,Font::bold));
 g.drawText("NOCTIVORE FX",22,14,560,45,Justification::left);
 g.setColour(painted::ochre);g.setFont(Font(12.5f,Font::bold));
 g.drawText("XENOBIOME  /  THE PAINTED FOREST",26,60,460,18,Justification::left);
 painted::spiral(g,567,41,23,painted::ochre.withAlpha(.65f),2.f);
 painted::creature(g,750,117,.52f);
 painted::mushroom(g,1090,81,.82f,painted::clay);
 painted::mushroom(g,1112,82,.54f,painted::moss);
 g.setColour(painted::bark);g.fillRoundedRectangle({18.f,92.f,790.f,72.f},13.f);
 g.setColour(painted::outline);g.drawRoundedRectangle({18.f,92.f,790.f,72.f},13.f,1.f);
 g.setColour(painted::ochre.withAlpha(.5f));g.drawLine(20,168,1140,168,1.f);
 for(size_t i=0;i<sections.size();++i){
  auto& s=sections[i];if(s.page!=page)continue;
  auto r=s.bounds.toFloat();r.setX(r.getX()/sx);r.setY(r.getY()/sy);
  r.setWidth(r.getWidth()/sx);r.setHeight(r.getHeight()/sy);
  painted::panel(g,r,s.accent,(int)i);
  g.setColour(s.accent);g.setFont(Font(12.2f,Font::bold));
  g.drawText(s.title,r.getX()+23,r.getY()+7,r.getWidth()-50,23,Justification::left);
 }
 g.setColour(painted::bark);g.fillRoundedRectangle({18.f,685.f,1124.f,74.f},10.f);
 g.setColour(painted::outline);g.drawRoundedRectangle({18.f,685.f,1124.f,74.f},10.f,1.f);
 const float peak=std::clamp(processor.peak.load(std::memory_order_relaxed),0.f,1.f);
 g.setColour(painted::stone);g.fillRoundedRectangle({1114.f,105.f,13.f,55.f},3.f);
 g.setColour(painted::moss);g.fillRoundedRectangle({1117.f,158.f-peak*50.f,7.f,peak*50.f},2.f);
 g.setColour(painted::soft);g.setFont(Font(10.f));
 g.drawText("OUT",1085,86,65,16,Justification::centred);
 if(processor.libraryLoaded){
  g.drawText("FOREST ATLAS  /  200 ORIGINAL SAMPLES  /  FOUR ULTRA-SLOW LFOs",27,764,940,18,Justification::left);
 }else{
  g.setColour(painted::clay);
  g.drawText("SOUND BANKS MISSING — check VST3 Contents/Resources/Banks",27,764,980,18,Justification::left);
 }
 g.setColour(painted::ochre);
 g.drawText("PAINTED FOREST 10.2",978,764,166,18,Justification::right);
}
