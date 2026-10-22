#include "PluginEditor.h"
#include <algorithm>
#include <cmath>
#include <functional>
#include <array>
#include <utility>
using namespace juce;
namespace {
 using namespace painted;
 Rectangle<int> rect(float x,float y,float w,float h,float sx,float sy){return {(int)std::round(x*sx),(int)std::round(y*sy),(int)std::round(w*sx),(int)std::round(h*sy)};}
}
NoctivoreEditor::Skin::Skin(){
 setColour(Slider::textBoxBackgroundColourId,painted::soil.withAlpha(.94f));
 setColour(Slider::textBoxOutlineColourId,painted::outline.withAlpha(.19f));
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
void NoctivoreEditor::Skin::drawComboBox(Graphics& g,int w,int h,bool,int,int,int,int,ComboBox&){
 painted::selector(g,w,h);
 g.setColour(painted::ochre.withAlpha(.29f));
 g.drawLine(12.f,(float)h-5.f,(float)w-24.f,(float)h-5.f,1.f);
}

// Custom painted button renderer: required for the FULL_GUI edition.
// The declaration in PluginEditor.h must have this out-of-line definition.
void NoctivoreEditor::ArtHotspot::paintButton(Graphics& g,bool hovered,bool down){
 const bool lit=hovered||down||selected;
 if(!lit)return;
 const auto bounds=getLocalBounds().toFloat().reduced(1.0f);
 const auto ochre=Colour::fromRGB(238,185,90);
 const auto moss=Colour::fromRGB(144,168,109);
 g.setColour((selected?moss:ochre).withAlpha(down?.31f:selected?.24f:.13f));
 g.fillRoundedRectangle(bounds,5.5f);
 g.setColour(ochre.withAlpha(down?.96f:selected?.76f:.58f));
 g.drawRoundedRectangle(bounds,5.5f,1.25f);
}

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
 const auto r=getLocalBounds().toFloat();
 const float cx=r.getCentreX(),cy=r.getCentreY();
 const float rad=.44f*std::min(r.getWidth(),r.getHeight());
 const float norm=getMaximum()>getMinimum()?std::clamp((float)((getValue()-getMinimum())/(getMaximum()-getMinimum())),0.f,1.f):0.f;
 const float begin=juce::MathConstants<float>::pi*1.18f,end=juce::MathConstants<float>::pi*2.82f;
 const float theta=begin+norm*(end-begin);
 const auto gold=juce::Colour::fromRGB(242,190,94);
 const float inner=rad*.78f;
 // Opaque clay face hides the baked-in, immovable pointer of the screenshot.
 juce::ColourGradient face(juce::Colour::fromRGB(121,88,49),cx-inner,cy-inner,juce::Colour::fromRGB(25,28,18),cx+inner,cy+inner,false);
 g.setGradientFill(face);g.fillEllipse(cx-inner,cy-inner,inner*2.f,inner*2.f);
 g.setColour(juce::Colour::fromRGB(12,16,10));g.drawEllipse(cx-inner,cy-inner,2.f*inner,2.f*inner,2.5f);
 g.setColour(gold.withAlpha(.55f));g.drawEllipse(cx-inner+2,cy-inner+2,inner*2.f-4,inner*2.f-4,1.f);
 juce::Path arc;arc.addCentredArc(cx,cy,rad*.93f,rad*.93f,0,begin,theta,true);
 g.setColour(gold.withAlpha(.9f));g.strokePath(arc,juce::PathStrokeType(3.2f));
 const float a=theta-juce::MathConstants<float>::halfPi;
 g.setColour(juce::Colour::fromRGB(255,221,138));g.drawLine(cx,cy,cx+std::cos(a)*inner*.83f,cy+std::sin(a)*inner*.83f,3.f);
 g.fillEllipse(cx-2,cy-2,4.f,4.f);
 if(isMouseOverOrDragging()){
  g.setColour(juce::Colour::fromRGB(18,22,16).withAlpha(.97f));g.fillRoundedRectangle(0.f,0.f,r.getWidth(),17.f,3.f);
  g.setColour(juce::Colour::fromRGB(254,217,160));g.setFont(juce::Font(10.f,juce::Font::bold));
  g.drawText(juce::String(getValue(),2),0,0,getWidth(),17,juce::Justification::centred);
 }
}
NoctivoreEditor::ArtFader::ArtFader(AudioProcessorValueTreeState& st,const String& id,const String& text){
 setSliderStyle(Slider::LinearVertical);
 setTextBoxStyle(Slider::NoTextBox,false,0,0);
 setMouseDragSensitivity(150);
 setTooltip(text+" — drag vertically");
 attachment=std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(st,id,*this);
}
void NoctivoreEditor::ArtFader::paint(Graphics& g){
 auto b=getLocalBounds().toFloat().reduced(9.f,5.f);
 const float norm=juce::jlimit(0.f,1.f,(float)((getValue()-getMinimum())/(getMaximum()-getMinimum()+1.e-12)));
 const float y=b.getBottom()-(norm*b.getHeight());
 g.setColour(juce::Colour::fromRGB(244,189,95).withAlpha(.22f));
 g.fillRoundedRectangle(b,3.f);
 g.setColour(juce::Colour::fromRGB(242,184,92).withAlpha(.80f));
 g.fillRoundedRectangle(b.getX(),y,b.getWidth(),b.getBottom()-y,2.f);
 g.setColour(juce::Colour::fromRGB(241,221,178));
 g.fillRoundedRectangle(b.getX()-3.f,y-3.f,b.getWidth()+6.f,5.f,2.f);
}
NoctivoreEditor::ArtMenu::ArtMenu(AudioProcessorValueTreeState& st,const String& id,const StringArray& names){
 for(auto name:names)addItem(name,getNumItems()+1);
 attachment=std::make_unique<AudioProcessorValueTreeState::ComboBoxAttachment>(st,id,*this);
}
NoctivoreEditor::Dial::Dial(AudioProcessorValueTreeState& apvts,const String& id,const String& text,int colour)
  :knob(apvts,id,text){
 knob.getProperties().set("tint",colour);
 addAndMakeVisible(knob);
 name.setText(text,dontSendNotification);
 name.setJustificationType(Justification::centred);
 name.setFont(Font(10.6f,Font::bold));
 name.setColour(Label::textColourId,painted::ivory);
 addAndMakeVisible(name);
}
void NoctivoreEditor::Dial::resized(){auto r=getLocalBounds();name.setBounds(r.removeFromTop(17));knob.setBounds(r);}
void NoctivoreEditor::Dial::paint(Graphics& g){
 auto r=getLocalBounds().toFloat().reduced(1.5f);
 g.setColour(painted::soil.withAlpha(.25f));
 g.fillRoundedRectangle(r,9.f);
 g.setColour(painted::outline.withAlpha(.23f));
 g.drawRoundedRectangle(r,9.f,.85f);
 g.setColour(painted::ochre.withAlpha(.21f));
 g.drawLine(10.f,r.getBottom()-5.f,r.getRight()-10.f,r.getBottom()-5.f,.9f);
 // A painted fleck and a tiny moss spore on every parameter tablet.
 g.setColour(painted::moss.withAlpha(.42f));g.fillEllipse(r.getRight()-9.f,8.f,3.f,3.f);
}

NoctivoreEditor::Selector::Selector(AudioProcessorValueTreeState& apvts,const String& id,const String& text,const StringArray& names){
 name.setText(text,dontSendNotification);name.setJustificationType(Justification::centredLeft);
 name.setFont(Font(10.5f,Font::bold));name.setColour(Label::textColourId,soft);
 addAndMakeVisible(name);
 for(auto item:names)box.addItem(item,box.getNumItems()+1);
 addAndMakeVisible(box);
 attachment=std::make_unique<AudioProcessorValueTreeState::ComboBoxAttachment>(apvts,id,box);
}
void NoctivoreEditor::Selector::resized(){auto r=getLocalBounds().reduced(4,1);name.setBounds(r.removeFromTop(17));box.setBounds(r.reduced(0,2));}
void NoctivoreEditor::Selector::paint(Graphics& g){
 auto r=getLocalBounds().toFloat().reduced(2.f,1.f);
 g.setColour(painted::stone.withAlpha(.3f));g.fillRoundedRectangle(r,7.f);
 g.setColour(painted::ochre.withAlpha(.28f));g.drawRoundedRectangle(r,7.f,1.f);
 g.setColour(painted::moss.withAlpha(.48f));g.fillEllipse(r.getRight()-8.f,7.f,3.2f,3.2f);
}

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
 dial(f,"colony","MUTANT COLONY",3);dial(f,"creatures","CREATURE DENSITY",3);
 dial(f,"psyFold","PSY SPECTRAL FOLD",1);dial(f,"symbiosis","SOURCE SYMBIOSIS",0);
 dial(f,"predator","PREDATOR THROAT",1);dial(f,"insectCode","SPORE CODE",0);dial(f,"burrow","BURROW INTERFERENCE",3);
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
  select(sy,"voiceLimit","CPU / VOICES",{"8 voices","12 voices","16 voices","24 voices"});
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
 auto artFader=[this](const char* id,const char* label,float x,float y,float w,float h){
   ArtFaderSpot f;f.x=x;f.y=y;f.w=w;f.h=h;
   f.fader=std::make_unique<ArtFader>(processor.state,id,label);
   addAndMakeVisible(*f.fader);artFaders.push_back(std::move(f));
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
 artMenu("bank",names,390,155,160,33);artMenu("bankB",names,984,155,156,33);
 artMenu("motif",{"Off","Underbrush","Offbeat","Tribal","Broken","Breathing","Shadow"},1232,421,188,28);
 artMenu("filterMode",{"Low Pass","Band Pass","High Pass"},218,796,215,30);
 artMenu("filter2Mode",{"Off","Low Pass","Band Pass","High Pass","Notch"},1057,796,265,30);
 artMenu("division",{"1/4","1/8","1/8 dotted","1/16","1/16 dotted","1/32","1/4 triplet","1/8 triplet"},645,797,218,30);
 artDial("morph","Bank A/B Morph",725,164,66);
 artDial("lowRegisterLife","Deep Life Growth",70,343);
 artDial("sub","Deep Life Depth",137,343);
 artDial("movement","Deep Life Movement",204,343);
 artDial("grain","Deep Life Texture",274,343);
 artDial("morphMotion","Motion Sync",369,341);
 artDial("biomeRate","Motion Rate",432,341);
 artDial("psySpiralRate","Motion Phase",496,341);
 artDial("coupledMotion","Linked Motion",558,341);
 artDial("layers","Biome Density",902,350);
 artDial("texture","Biome Size",969,350);
 artDial("stereo","Biome Spread",1037,350);
 artDial("biomeDepth","Biome Life",1108,350);
 artDial("tranceRate","Gate Rate",1209,346);
 artDial("motifPitch","Gate Length",1278,346);
 artDial("motifGlide","Gate Swing",1346,346);
 artDial("motifGate","Rhythmic Gate",1411,346);
 artDial("xenoFormant","Alien Vocal Formants",135,526);
 artDial("predator","Predator Talk",201,526);
 artDial("fmIndex","Harmonics",269,526);
 artDial("xenoCross","Wet",337,526);
 artDial("xenoCross","Coupling",414,606);
 artDial("morph","Organism Transfer",514,606);
 artDial("morphMotion","Variation",588,606);
 artDial("burrow","Burrowing Body Shape",683,574);
 artDial("grain","Body Stretch",755,574);
 artDial("insectCode","Spore Disruption",825,574);
 artDial("motionEnvRelease","Regenerate",896,574);
 artDial("fmRatio","FM Ratio",1004,530);
 artDial("fmIndex","FM Index",1064,530);
 artDial("fmMix","FM Amount",1124,530);
 artDial("fmInharmonic","FM Blend",1181,530);
 artDial("drive","Drive",1361,546);
 artDial("macroAggression","Bite",1431,546);
 artDial("filterDrive","Fuzz",1361,614);
 artDial("spectralTilt","Tone",1433,614);
 artDial("cutoff","Cutoff 1",198,749);
 artDial("resonance","Resonance 1",267,749);
 artDial("filterEnvDepth","Filter 1 Envelope",341,749);
 artDial("filterDrive","Filter 1 Drive",414,749);
 artDial("reverbSize","Atmosphere Size",647,759);
 artDial("reverbDark","Damping",714,759);
 artDial("stereo","Width",783,759);
 artDial("reverbMix","Reverb Mix",851,759);
 artDial("cutoff2","Cutoff 2",1081,760);
 artDial("resonance2","Resonance 2",1149,760);
 artDial("filterBlend","Filter Blend",1222,760);
 artDial("filterDrive","Filter Tone",1294,760);
 artDial("volume","Master Output",1360,938,68);
 artFader("bend","Pitch Bend",51,916,43,84);
 artFader("performanceMod","Performance Modulation",109,916,43,84);
 const float tx[]={244,445,626,801,1024},tw[]={170,180,170,181,171};
 for(int i=0;i<5;++i)artButton("Switch page",tx[i],873,tw[i],36,[this,i]{showPage((Page)i);});
 artButton("Previous preset",528,200,32,42,[this]{stepScene(-1);});
 artButton("Next preset",804,200,32,42,[this]{stepScene(1);});
 artButton("Save patch",844,201,37,42,[this]{savePreset();});
 artButton("Mutate organism",884,201,38,42,[this]{mutate(false);});
 artButton("Subtle evolution",928,201,38,42,[this]{mutate(true);});
 artButton("Previous source A",363,152,25,38,[this]{stepChoice("bank",-1,51);});
 artButton("Next source A",545,152,25,38,[this]{stepChoice("bank",1,51);});
 artButton("Previous source B",955,152,24,38,[this]{stepChoice("bankB",-1,51);});
 artButton("Next source B",1143,152,24,38,[this]{stepChoice("bankB",1,51);});
 artButton("Load patch",970,201,35,42,[this]{loadPreset();});
 // Small symbols painted under Linked Motion select LFO 1 wave shapes.
 for(int i=0;i<4;++i){
  const int index=i;
  artButton("LFO 1 wave shape",375.f+i*50.f,396.f,42.f,42.f,
   [this,index]{setParameter("wave",(float)index);});
 }
 // Biome organism glyphs select contrasting texture engines.
 for(int i=0;i<4;++i){
  const int alg[]={0,1,3,4};
  const int index=i;
  artButton("Organism texture mode",856.f+i*68.f,396.f,51.f,47.f,
   [this,index,alg]{setParameter("textureMode",(float)alg[index]);});
 }
 // Entire painted 16-step gate is user-programmable, automatable and saved.
 for(int i=0;i<16;++i){
  const auto key=juce::String::formatted("gateStep%02d",i+1);
  artButton((juce::String("Gate step ")+juce::String(i+1)).toRawUTF8(),1151.f+i*14.2f,381.f,13.6f,44.f,
   [this,key]{
     if(auto* v=processor.state.getRawParameterValue(key))
       setParameter(key.toRawUTF8(),v->load()>.5f?0.f:1.f);
   });
 }

 showPage(Atlas);
 setResizable(true,true);
 setResizeLimits(919,650,1491,1055);
 if(auto* c=getConstrainer())c->setFixedAspectRatio(1491.0/1055.0);
 setSize(1044,739);
 startTimerHz(24);
}
NoctivoreEditor::~NoctivoreEditor(){stopTimer();presetChooser.reset();setLookAndFeel(nullptr);}

void NoctivoreEditor::stepChoice(const char* id,int amount,int count){
 if(count<=0)return;
 if(auto* raw=processor.state.getRawParameterValue(id)){
  const int current=juce::roundToInt(raw->load());
  setParameter(id,(float)((current+count+amount%count)%count));
 }
}
void NoctivoreEditor::stepScene(int direction){
 const int count=scenes.getNumItems()-1;
 if(count<=0)return;
 const int current=scenes.getSelectedId()<=1?0:scenes.getSelectedId()-2;
 const int next=(current+count+direction%count)%count;
 scenes.setSelectedId(next+2,juce::sendNotificationSync);
}
void NoctivoreEditor::savePreset(){
 const auto folder=juce::File::getSpecialLocation(juce::File::userDocumentsDirectory);
 presetChooser=std::make_unique<juce::FileChooser>("Save NOCTIVORE patch",
   folder.getChildFile("NOCTIVORE Forest.nvpreset"),"*.nvpreset");
 juce::Component::SafePointer<NoctivoreEditor> alive(this);
 presetChooser->launchAsync(juce::FileBrowserComponent::saveMode |
    juce::FileBrowserComponent::canSelectFiles |
    juce::FileBrowserComponent::warnAboutOverwriting,
    [alive](const juce::FileChooser& chooser){
      if(alive==nullptr)return;
      auto file=chooser.getResult();
      if(file.getFullPathName().isEmpty())return;
      file=file.withFileExtension(".nvpreset");
      juce::MemoryBlock data;
      alive->processor.getStateInformation(data);
      file.replaceWithData(data.getData(),data.getSize());
    });
}
void NoctivoreEditor::loadPreset(){
 const auto folder=juce::File::getSpecialLocation(juce::File::userDocumentsDirectory);
 presetChooser=std::make_unique<juce::FileChooser>("Load NOCTIVORE patch",folder,"*.nvpreset");
 juce::Component::SafePointer<NoctivoreEditor> alive(this);
 presetChooser->launchAsync(juce::FileBrowserComponent::openMode |
    juce::FileBrowserComponent::canSelectFiles,
    [alive](const juce::FileChooser& chooser){
      if(alive==nullptr)return;
      auto file=chooser.getResult();
      if(!file.existsAsFile())return;
      juce::MemoryBlock data;
      if(file.loadFileAsData(data) && data.getSize()>0)
        alive->processor.setStateInformation(data.getData(),(int)data.getSize());
      alive->hasCustomPatch=true;
    });
}

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
 setParameter("colony",.2f+r.nextFloat()*.75f);
 setParameter("predator",.12f+r.nextFloat()*.86f);
 setParameter("insectCode",.10f+r.nextFloat()*.90f);
 setParameter("burrow",.15f+r.nextFloat()*.80f);
 setParameter("creatures",.1f+r.nextFloat()*.85f);
 setParameter("psyFold",.04f+r.nextFloat()*.65f);
 setParameter("symbiosis",.1f+r.nextFloat()*.75f);
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
 // Contrast between hollow vocal/creature, coded insect and dark FM scenes.
 setParameter("fmMix",s.fm*(which%3==0?1.75f:1.10f));
 setParameter("metalMix",s.metal*.045f);
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
 setParameter("biomeRate",.018f+.012f*(float)(which%7));
 setParameter("biomeChaos",.28f+.11f*(float)(which%6));
 setParameter("lowRegisterLife",.58f+.05f*(float)(which%7));
 setParameter("coupledMotion",.38f+.08f*(float)(which%6));
 setParameter("morphMotion",.23f+.045f*(float)(which%6));
 setParameter("colony",.25f+.12f*(float)(which%6));
 setParameter("creatures",.25f+.105f*(float)(which%6));
 setParameter("psyFold",.22f+.12f*(float)(which%5));
 setParameter("symbiosis",.17f+.12f*(float)(which%6));
 // Distinct scenes: spores, predator, burrow and spectral warps are not uniform.
 static constexpr float tribes[12][3]={
  {.18f,.88f,.18f},{.78f,.18f,.72f},{.20f,.94f,.64f},
  {.88f,.36f,.80f},{.67f,.85f,.17f},{.82f,.24f,.66f},
  {.23f,.96f,.28f},{.92f,.36f,.90f},{.30f,.72f,.73f},
  {.55f,.93f,.41f},{.79f,.25f,.88f},{.62f,.67f,.91f}};
 setParameter("predator",tribes[which%12][0]);
 setParameter("insectCode",tribes[which%12][1]);
 setParameter("burrow",tribes[which%12][2]);
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
 for(auto& a:artFaders)a.fader->setVisible(art);
 for(size_t i=0;i<artButtons.size();++i){
  artButtons[i].button->setVisible(art);
  artButtons[i].button->selected=art && i<5 && i==(size_t)page;
 }
 for(size_t i=0;i<tabs.size();++i){
  tabs[i].setVisible(!art);
  tabs[i].setToggleState((int)i==(int)page,dontSendNotification);
 }
 for(auto& a:actions)a.setVisible(!art);
 scenes.setVisible(true);sceneLabel.setVisible(!art);
 for(auto& c:controls)if(c->getName().startsWith("PERMANENT_"))c->setVisible(!art);
 // Atlas uses the painted keys as the visible surface; MIDI mouse handling
 // remains active but the generic piano overlay no longer ghosts through.
 keyboard.setAlpha(art?.015f:1.f);
 if(getWidth()>0 && getHeight()>0)resized();
 repaint();
}
void NoctivoreEditor::layout(Section& section_,Rectangle<int> area){
 section_.bounds=area;
 auto inset=area.reduced(9,13);
 inset.removeFromTop(31);
 if(page==Motion && section_.children.size()==5)
  inset.removeFromBottom(std::max(26,juce::roundToInt(48.f*(float)getHeight()/1055.f)));
 int count=(int)section_.children.size();
 int cols=std::max(1,section_.columns),rows=std::max(1,(count+cols-1)/cols);
 const int cw=std::max(15,inset.getWidth()/cols),ch=std::max(22,inset.getHeight()/rows);
 for(int i=0;i<count;++i){auto x=inset.getX()+(i%cols)*cw,y=inset.getY()+(i/cols)*ch;
  section_.children[(size_t)i]->setBounds(x+3,y+2,std::max(8,cw-6),std::max(16,ch-4));}
}
void NoctivoreEditor::resized(){
 const float sx=(float)getWidth()/1491.f,sy=(float)getHeight()/1055.f;
 auto R=[&](float x,float y,float w,float h){
  return juce::Rectangle<int>(roundToInt(x*sx),roundToInt(y*sy),roundToInt(w*sx),roundToInt(h*sy));};
 const bool atlas=paintedPage();
 for(auto& a:artDials)a.knob->setBounds(R(a.x-a.d*.5f,a.y-a.d*.5f,a.d,a.d));
 for(auto& a:artMenus)a.menu->setBounds(R(a.x,a.y,a.w,a.h));
 for(auto& a:artFaders)a.fader->setBounds(R(a.x,a.y,a.w,a.h));
 for(auto& a:artButtons)a.button->setBounds(R(a.x,a.y,a.w,a.h));
 // In every mode the painted keyboard occupies the same exact place.
 keyboard.setBounds(R(214,907,1024,94));
 scenes.setBounds(atlas?R(548,201,275,41):R(530,181,365,39));
 sceneLabel.setBounds(R(538,159,210,21));
 for(size_t i=0;i<tabs.size();++i)tabs[i].setBounds(R(244.f+(float)i*204.f,862.f,190.f,42.f));
 actions[0].setBounds(R(1114,174,103,44));
 actions[1].setBounds(R(1224,174,103,44));
 actions[2].setBounds(R(1334,174,123,44));
 for(auto& c:controls){
  if(c->getName()=="PERMANENT_ATM")c->setBounds(R(1013,109,115,89));
  if(c->getName()=="PERMANENT_FURY")c->setBounds(R(1359,109,115,89));
 }
 for(auto& section_:sections){
  if(section_.page!=page){section_.bounds={};continue;}
  int index=0;
  for(const auto& test:sections){if(&test==&section_)break;if(test.page==page)++index;}
  if(atlas){
   section_.bounds={}; // No additional widgets laid over the illustrated front page.
  }else if(page==Motion){
   if(index<4)layout(section_,R(35.f+index*359.f,249.f,345.f,430.f));
   else layout(section_,R(36.f,695.f,1420.f,133.f));
  }else{
   layout(section_,R(36.f+index*477.f,259.f,460.f,559.f));
  }
 }
}
void NoctivoreEditor::timerCallback(){
 uiSeconds+=1.f/24.f;
 if(uiSeconds>72000.f)uiSeconds=0.f;
 smoothedPeak=.78f*smoothedPeak+.22f*std::clamp(processor.peak.load(std::memory_order_relaxed),0.f,1.f);
 const float rate=processor.displayTempo.load(std::memory_order_relaxed)/60.f;
 const float pos=processor.displayPlaying.load(std::memory_order_relaxed)?
   processor.displayPpq.load(std::memory_order_relaxed):uiSeconds*rate;
 const float divisions[]={2.f,4.f,8.f,3.f};
 int div=std::clamp((int)processor.state.getRawParameterValue("motifDivision")->load(),0,3);
 int next=(int)std::floor(pos*divisions[div]);
 currentGateStep=((next%16)+16)%16;
 // Skip unrelated hotspots; the extra 16 gate keys are appended last.
 if(artButtons.size()>=16){
  const size_t start=artButtons.size()-16;
  for(size_t i=0;i<16;++i){
    const auto key=juce::String::formatted("gateStep%02d",(int)i+1);
    artButtons[start+i].button->selected=processor.state.getRawParameterValue(key)->load()>.5f;
  }
 }
 repaint();
}

void NoctivoreEditor::drawAnimation(Graphics& g){
 juce::Graphics::ScopedSaveState save(g);
 g.addTransform(juce::AffineTransform::scale((float)getWidth()/1491.f,(float)getHeight()/1055.f));
 auto val=[this](const char* id)->float{
  if(auto* p=processor.state.getRawParameterValue(id))return p->load();return 0.f;};
 const float life=std::clamp(val("biomeDepth"),0.f,1.f);
 const float coupled=std::clamp(val("coupledMotion"),0.f,1.f);
 const float peak=std::clamp(smoothedPeak,0.f,1.f);
 const float breathing=.5f+.5f*std::sin(uiSeconds*(1.0f+.75f*life));
 const float pulse=.4f+.35f*breathing+.25f*std::sqrt(peak);
 const auto gold=juce::Colour::fromRGB(251,191,92);
 const auto green=juce::Colour::fromRGB(166,213,115);
 const float cx=720.f,cy=424.f;
 // Visible moving center: mask printed static art below with opaque organic disk.
 g.setColour(juce::Colour::fromRGB(23,28,17).withAlpha(.98f));
 g.fillEllipse(cx-33.f,cy-33.f,66.f,66.f);
 for(int n=0;n<3;++n){
  const float rad=23.f+n*10.f+7.f*pulse;
  g.setColour((n%2?green:gold).withAlpha(.55f+.12f*pulse-.09f*n));
  g.drawEllipse(cx-rad,cy-rad,2.f*rad,2.f*rad,1.8f);
 }
 juce::Path swirl;
 for(int i=0;i<130;++i){
  const float t=(float)i/129.f;
  const float angle=uiSeconds*(.85f+.65f*coupled)+(4.5f+t)*juce::MathConstants<float>::pi*t;
  const float rr=(8.f+25.f*pulse)*t;
  const float x=cx+std::cos(angle)*rr,y=cy+std::sin(angle)*rr;
  if(i==0)swirl.startNewSubPath(x,y);else swirl.lineTo(x,y);
 }
 g.setColour(gold.withAlpha(.94f));g.strokePath(swirl,juce::PathStrokeType(2.8f));
 // Moving spore orbit, breath-driven even at ultra-slow musical LFO rates.
 for(int i=0;i<13;++i){
  const float phase=(float)i*1.37f+uiSeconds*(.13f+.07f*(i%4))*(1.f+.5f*coupled);
  const float radius=58.f+14.f*(i%4)+11.f*breathing;
  const float x=cx+radius*std::cos(phase),y=cy+radius*.64f*std::sin(phase*1.13f);
  const float size=2.2f+1.6f*(.5f+.5f*std::sin(uiSeconds+i));
  g.setColour((i%2?gold:green).withAlpha(.45f+.39f*breathing));
  g.fillEllipse(x-size,y-size,size*2.f,size*2.f);
 }
 for(int i=0;i<2;++i){
  const float x=i?741.f:698.f,y=350.f;
  const float r=7.f+10.f*pulse;
  g.setColour(gold.withAlpha(.32f+.30f*pulse));g.fillEllipse(x-r,y-r,r*2.f,r*2.f);
  g.setColour(juce::Colour::fromRGB(255,232,144));g.fillEllipse(x-4.f,y-4.f,8.f,8.f);
 }
 if(val("motif")>.5f){
  for(int i=0;i<16;++i){
   const float x=1156.f+(float)i*13.95f;
   auto* v=processor.state.getRawParameterValue(juce::String::formatted("gateStep%02d",i+1));
   const bool on=v==nullptr||v->load()>.5f,active=i==currentGateStep;
   const float h=19.f+9.f*std::sin(i*.77f+uiSeconds*.2f);
   g.setColour((active?gold:green).withAlpha(on?(active?.98f:.34f):.06f));
   g.fillRoundedRectangle(x,415.f-h,9.4f,h,2.f);
   if(active)g.fillEllipse(x+1,373,8,8);
  }
 }
 const float meter=std::sqrt(peak);
 for(int i=0;i<2;++i){
  g.setColour(green.withAlpha(.95f));
  g.fillRoundedRectangle(1291.f+i*22.f,996.f-76.f*meter,11.f,76.f*meter,2.f);
 }
}
void NoctivoreEditor::drawSubpage(Graphics& g){
 // Completely opaque separate backdrop; never draw the old Atlas screenshot.
 g.fillAll(juce::Colour::fromRGB(15,20,14));
 {
  juce::Graphics::ScopedSaveState frame(g);
  g.addTransform(juce::AffineTransform::scale((float)getWidth()/1491.f,(float)getHeight()/1055.f));
  juce::ColourGradient earth(juce::Colour::fromRGB(36,42,27),0.f,0.f,juce::Colour::fromRGB(12,17,11),1491.f,1055.f,false);
  g.setGradientFill(earth);g.fillRect(0,0,1491,1055);
  for(int k=0;k<18;++k){
   float y=32.f+58.f*k;juce::Path p;p.startNewSubPath(0.f,y);
   p.cubicTo(380.f,y-28.f,800.f,y+24.f,1491.f,y-11.f);
   g.setColour((k%2?painted::moss:painted::ochre).withAlpha(.049f));
   g.strokePath(p,juce::PathStrokeType(1.1f));
  }
  painted::mushroom(g,1418.f,158.f,1.5f,painted::clay);
  painted::mushroom(g,67.f,832.f,1.1f,painted::moss);
  painted::creature(g,1131.f,123.f,.55f);
 }

 // Cropped untouched tree/mushroom art: no printed Atlas labels or knobs.
 if(originalPainting.isValid()){
  juce::Graphics::ScopedSaveState artwork(g);
  g.addTransform(juce::AffineTransform::scale((float)getWidth()/1491.f,(float)getHeight()/1055.f));
  g.drawImage(originalPainting,0.f,0.f,310.f,246.f,0,0,310,246);
  g.drawImage(originalPainting,1185.f,0.f,306.f,246.f,1185,0,306,246);
 }
 juce::Graphics::ScopedSaveState state(g);
 const float sx=(float)getWidth()/1491.f,sy=(float)getHeight()/1055.f;
 g.addTransform(juce::AffineTransform::scale(sx,sy));
 const juce::Colour ochre=juce::Colour::fromRGB(221,175,97);
 const juce::Colour ivory=juce::Colour::fromRGB(237,216,173);
 const juce::Colour moss=juce::Colour::fromRGB(147,166,104);
 const char* names[]={"ATLAS","MORPHOLOGY","4 LFOs","PSY SPACE","UTILITY"};
 g.setColour(ivory);g.setFont(juce::Font(34.f,juce::Font::bold));
 g.drawText("NOCTIVORE FX",34,34,570,50,juce::Justification::left);
 g.setColour(ochre);g.setFont(juce::Font(17.f,juce::Font::bold));
 g.drawText(juce::String("XENOBIOME / ")+names[(int)page],37,88,710,26,juce::Justification::left);
 painted::spiral(g,965.f,77.f,40.f,ochre.withAlpha(.42f),2.3f);
 painted::mushroom(g,1423.f,107.f,1.35f,painted::clay.withAlpha(.9f));
 painted::mushroom(g,1381.f,111.f,.87f,moss.withAlpha(.73f));
 g.setColour(juce::Colour::fromRGB(44,44,30));g.fillRoundedRectangle({25.f,155.f,1440.f,78.f},14.f);
 g.setColour(ochre.withAlpha(.49f));g.drawRoundedRectangle({25.f,155.f,1440.f,78.f},14.f,1.f);
 g.setColour(ivory.withAlpha(.5f));g.setFont(juce::Font(13.f));
 g.drawText("FACTORY SCENE",400,178,128,27,juce::Justification::left);
 for(size_t index=0;index<sections.size();++index){
  const auto& s=sections[index];if(s.page!=page)continue;
  auto r=s.bounds.toFloat();
  if(r.isEmpty())continue;
  r.setX(r.getX()/sx);r.setY(r.getY()/sy);
  r.setWidth(r.getWidth()/sx);r.setHeight(r.getHeight()/sy);
  painted::panel(g,r,s.accent,(int)index);
  for(int d=0;d<5;++d){
    g.setColour((d%2?painted::clay:painted::moss).withAlpha(.24f));
    g.fillEllipse(r.getX()+24.f+d*11.f,r.getBottom()-19.f+2.f*std::sin(d*1.7f),3.5f,3.5f);
  }
  g.setColour(s.accent);
  g.setFont(juce::Font(17.f,juce::Font::bold));
  g.drawText(s.title,r.getX()+29.f,r.getY()+10.f,r.getWidth()-54.f,29.f,juce::Justification::left);
 }
 if(page==Motion){
  // Visualization of each LFO's waveform / rate and depth. The actual modulation
  // remains in the DSP; this is an informative, parameter-driven phase preview.
  for(int i=0;i<4;++i){
   const juce::String prefix=i==0?"":("lfo"+juce::String(i+1));
   const auto rateId=i==0?juce::String("rate"):prefix+"Rate";
   const auto depthId=i==0?juce::String("depth"):prefix+"Depth";
   const auto waveId=i==0?juce::String("wave"):prefix+"Wave";
   const auto syncId="lfo"+juce::String(i+1)+"Sync";
   auto raw=[this](const juce::String& key)->float{
     if(auto* v=processor.state.getRawParameterValue(key))return v->load();
     return 0.f;
   };
   float hz=std::max(.0002f,raw(rateId));
   const int sync=std::clamp((int)raw(syncId),0,10);
   if(sync!=0){
    constexpr float beats[]={4.f,2.f,1.f,.5f,.25f,8.f,16.f,32.f,64.f,128.f};
    hz=(processor.displayTempo.load(std::memory_order_relaxed)/60.f)/beats[sync-1];
   }
   const int wave=(int)raw(waveId);
   const float depth=std::clamp(raw(depthId),0.f,1.f);
   const float x=57.f+i*359.f,y=643.f;
   juce::Path graph;
   for(int k=0;k<=88;++k){
    const float t=(float)k/88.f;
    const float phase=std::fmod(t*1.5f+uiSeconds*hz,1.f);
    float v=std::sin(juce::MathConstants<float>::twoPi*phase);
    if(wave==1)v=1.f-4.f*std::abs(phase-.5f);
    else if(wave==2)v=phase*2.f-1.f;
    else if(wave==3)v=phase<.5f?1.f:-1.f;
    else if(wave==4)v=std::sin(6.283185f*std::floor(t*6.f+uiSeconds*hz));
    else if(wave==5)v*=std::sin(18.84955f*phase);
    const float yy=y-13.f*v*depth;
    if(k==0)graph.startNewSubPath(x,yy);else graph.lineTo(x+t*298.f,yy);
   }
   g.setColour(moss.withAlpha(.22f));g.drawLine(x,y,x+298.f,y,1.f);
   g.setColour(ochre.withAlpha(.85f));g.strokePath(graph,juce::PathStrokeType(2.f));
  }
 }
 // Painted navigation and playable keyboard remain in the same locations as
 // the main illustration, providing consistent visual continuity on every page.
 g.setColour(juce::Colour::fromRGB(35,34,23));
 g.fillRoundedRectangle({25.f,842.f,1440.f,57.f},11.f);
 g.setColour(ochre.withAlpha(.36f));
 g.drawRoundedRectangle({25.f,842.f,1440.f,57.f},11.f,1.f);
 g.setColour(juce::Colour::fromRGB(36,35,25));
 g.fillRoundedRectangle({200.f,901.f,1060.f,108.f},12.f);
 const float level=std::clamp(smoothedPeak,0.f,1.f);
 g.setColour(moss);
 g.fillRoundedRectangle({1281.f,985.f-53.f*level,13.f,53.f*level},2.f);
 // Rotating organism on each deep-edit page; the old GUI image is absent.
 const float angle=uiSeconds*(.53f+processor.state.getRawParameterValue("biomeDepth")->load());
 for(int k=0;k<8;++k){
  const float a=angle+(float)k*juce::MathConstants<float>::twoPi/8.f;
  const float x=1210.f+42.f*std::cos(a),y=87.f+25.f*std::sin(a);
  g.setColour((k%2?ochre:moss).withAlpha(.53f+.40f*level));
  g.fillEllipse(x-3.7f,y-3.7f,7.4f,7.4f);
 }
 painted::spiral(g,1210.f,87.f,27.f+12.f*level,ochre.withAlpha(.7f),2.2f);
}
void NoctivoreEditor::paint(Graphics& g){
 if(paintedPage()){
  if(originalPainting.isValid())g.drawImage(originalPainting,getLocalBounds().toFloat());
  else g.fillAll(juce::Colour::fromRGB(17,20,15));
  drawAnimation(g);
 }else drawSubpage(g);
}
