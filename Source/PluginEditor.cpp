#include "PluginEditor.h"
#include <algorithm>
#include <cmath>
using namespace juce;
namespace {
 const Colour soil=Colour::fromRGB(23,25,20);
 const Colour bark=Colour::fromRGB(37,40,31);
 const Colour stone=Colour::fromRGB(53,55,43);
 const Colour outline=Colour::fromRGB(87,86,65);
 const Colour ivory=Colour::fromRGB(235,219,184);
 const Colour soft=Colour::fromRGB(176,170,143);
 const Colour moss=Colour::fromRGB(166,178,112);
 const Colour clay=Colour::fromRGB(205,117,77);
 const Colour ochre=Colour::fromRGB(224,180,102);
 const Colour slate=Colour::fromRGB(143,164,155);
 const Colour ash=Colour::fromRGB(188,149,129);
 const std::array<Colour,5> accents{{moss,clay,ochre,slate,ash}};
 Rectangle<int> rect(float x,float y,float w,float h,float sx,float sy){
  return {(int)std::round(x*sx),(int)std::round(y*sy),(int)std::round(w*sx),(int)std::round(h*sy)};
 }
}
NoctivoreEditor::Skin::Skin(){
 setColour(Slider::textBoxBackgroundColourId,Colours::transparentBlack);
 setColour(Slider::textBoxOutlineColourId,Colours::transparentBlack);
 setColour(Slider::textBoxTextColourId,ivory);
 setColour(ComboBox::backgroundColourId,stone.darker(.25f));
 setColour(ComboBox::outlineColourId,outline);
 setColour(ComboBox::textColourId,ivory);
 setColour(ComboBox::arrowColourId,ochre);
 setColour(PopupMenu::backgroundColourId,bark);
 setColour(PopupMenu::textColourId,ivory);
 setColour(PopupMenu::highlightedBackgroundColourId,clay.darker(.55f));
 setColour(TextButton::textColourOffId,ivory);
 setColour(TextButton::textColourOnId,soil);
}
void NoctivoreEditor::Skin::drawRotarySlider(Graphics& g,int x,int y,int w,int h,float value,float start,float end,Slider& slider){
 const auto col=accents[(size_t)std::clamp((int)slider.getProperties().getWithDefault("tint",0),0,4)];
 float radius=std::max(10.f,std::min(w*.32f,h*.37f));
 const float cx=x+w*.5f,cy=y+h*.43f;
 Path track;track.addCentredArc(cx,cy,radius,radius,0,start,end,true);
 g.setColour(outline.withAlpha(.75f));g.strokePath(track,PathStrokeType(4.5f,PathStrokeType::curved,PathStrokeType::rounded));
 float angle=start+(end-start)*value;
 Path arc;arc.addCentredArc(cx,cy,radius,radius,0,start,angle,true);
 g.setColour(col);g.strokePath(arc,PathStrokeType(4.5f,PathStrokeType::curved,PathStrokeType::rounded));
 ColourGradient grad(stone.brighter(.2f),cx-radius,cy-radius,soil,cx+radius,cy+radius,false);
 g.setGradientFill(grad);g.fillEllipse(cx-radius*.81f,cy-radius*.81f,radius*1.62f,radius*1.62f);
 g.setColour(outline);g.drawEllipse(cx-radius*.81f,cy-radius*.81f,radius*1.62f,radius*1.62f,1.f);
 float a=angle-MathConstants<float>::halfPi;
 g.setColour(col);
 g.drawLine(cx,cy,cx+std::cos(a)*radius*.62f,cy+std::sin(a)*radius*.62f,2.5f);
}
void NoctivoreEditor::Skin::drawButtonBackground(Graphics& g,Button& b,const Colour&,bool hovered,bool pressed){
 auto box=b.getLocalBounds().toFloat().reduced(1.5f);
 auto tint=accents[(size_t)std::clamp((int)b.getProperties().getWithDefault("tint",0),0,4)];
 bool active=b.getToggleState();
 g.setColour(active?tint:pressed?stone.brighter(.1f):hovered?stone:bark);
 g.fillRoundedRectangle(box,7.f);
 g.setColour(active?tint.brighter(.45f):outline);
 g.drawRoundedRectangle(box,7.f,1.f);
 g.setColour(tint.withAlpha(active?.78f:.52f));
 g.fillRoundedRectangle({box.getX()+6.f,box.getY()+5.f,2.f,box.getHeight()-10.f},1.f);
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
 // No resizing until EVERY child and attachment is constructed: essential
 // for Ableton Live, which may request editor layout immediately on creation.
 addAndMakeVisible(keyboard);keyboard.setAvailableRange(24,96);
 keyboard.setKeyWidth(21.f);keyboard.setScrollButtonsVisible(false);
 StringArray bankNames;for(auto& b:noctivore::banks)bankNames.add(b.name);bankNames.add("User Bank");
 auto& a=section(Atlas,"01 / SOURCE FIELD",moss,3);
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
 auto& x=section(Atlas,"02B / XENOBIOME MORPHOLOGY",slate,3);
 dial(x,"xenoFormant","ALIEN VOCAL TRACT",3);
 dial(x,"xenoCross","ORGANISM COUPLING",3);
 dial(x,"xenoWrithe","BODY MUTATION",3);
 auto& c=section(Atlas,"03 / RHYTHMIC GATE LAB",clay,3);
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
 const char* labels[]={"ATLAS","ALIEN FM","4 LFOs","PSY SPACE","UTILITY"};
 for(size_t i=0;i<tabs.size();++i){addAndMakeVisible(tabs[i]);tabs[i].setButtonText(labels[i]);
  tabs[i].getProperties().set("tint",(int)i);tabs[i].setClickingTogglesState(false);
  tabs[i].onClick=[this,i]{showPage((Page)i);};}
 const char* actionLabels[]={"MUTATE","EVOLVE","GROOVE"};
 for(size_t i=0;i<actions.size();++i){addAndMakeVisible(actions[i]);actions[i].setButtonText(actionLabels[i]);
  actions[i].getProperties().set("tint",(int)(i+1));}
 actions[0].onClick=[this]{mutate(false);};actions[1].onClick=[this]{mutate(true);};
 actions[2].onClick=[this]{setParameter("motif",3.f);setParameter("motifDivision",1.f);
   setParameter("motifPitch",.36f);setParameter("motifGate",.72f);
   setParameter("motifAccent",.86f);setParameter("motifFX",.21f);setParameter("wetLowCut",.76f);
   setParameter("reverbMix",.30f);setParameter("morphMotion",.12f);showPage(Atlas);};
 sceneLabel.setText("SHADOW RHYTHMS",dontSendNotification);sceneLabel.setFont(Font(10.f,Font::bold));
 sceneLabel.setColour(Label::textColourId,soft);addAndMakeVisible(sceneLabel);
 const char* presets[]={"CHOOSE SCENE","ROOT HYPNOSIS","SHADOW SIGNAL","SPORE CHATTER","DUSK PATTERN","GHOST CANOPY","LOW TIDE","GUTTER AIR","INSECT FOG","WORMHOLE BREATH","VOID GRIT","WET ROOTS","CAVE RHYTHM"};
 addAndMakeVisible(scenes);for(int i=0;i<13;++i)scenes.addItem(presets[i],i+1);
 scenes.setSelectedId(1,dontSendNotification);
 scenes.onChange=[this]{if(scenes.getSelectedId()>1)applyScene(scenes.getSelectedId()-2);};
 // Always-visible macros. Their controls are real host-automatable parameters.
 auto atm=std::make_unique<Dial>(processor.state,"macroAtmosphere","ATMOSPHERE",0);
 atm->setName("PERMANENT_ATM");addAndMakeVisible(*atm);controls.push_back(std::move(atm));
 auto fury=std::make_unique<Dial>(processor.state,"macroAggression","AGGRESSION",1);
 fury->setName("PERMANENT_FURY");addAndMakeVisible(*fury);controls.push_back(std::move(fury));
 showPage(Atlas);
 setResizable(true,true);setResizeLimits(1020,690,1620,1100);
 setSize(1160,790);
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
 setParameter("motifFX",.025f+r.nextFloat()*.30f);
 setParameter("wetLowCut",.49f+r.nextFloat()*.45f);
 setParameter("mudCut",.30f+r.nextFloat()*.60f);
 setParameter("reverbMix",.17f+r.nextFloat()*.25f);
 setParameter("morph",.15f+r.nextFloat()*.7f);
 setParameter("morphMotion",.12f+r.nextFloat()*.7f);
 setParameter("morphRate",.007f+r.nextFloat()*.13f);
 setParameter("fmMix",.01f+r.nextFloat()*.16f);
 setParameter("metalMix",.001f+r.nextFloat()*.055f);
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
 setParameter("fmMix",s.fm);setParameter("metalMix",s.metal);
 setParameter("fmIndex",s.index);setParameter("morph",s.morph);
 setParameter("macroAtmosphere",s.atm);setParameter("macroAggression",s.fury);
 setParameter("filter2Mode",2.f);setParameter("filterRoute",(float)(which%3));
 setParameter("filterBlend",s.filter2);setParameter("attack",.055f+.03f*(float)(which%4));
 setParameter("release",2.1f+.4f*(float)(which%5));setParameter("reverbMix",.24f+.025f*(float)(which%5));
 setParameter("reverbSize",.85f);setParameter("metalDecay",.60f);
 setParameter("motif",(float)(1+which%6));
 setParameter("motifDivision",(float)(which%4));
 setParameter("motifPitch",.19f+.067f*(float)(which%8));
 setParameter("motifGate",.32f+.08f*(float)(which%7));
 setParameter("motifAccent",.45f+.07f*(float)(which%6));
 setParameter("motifGlide",.08f+.06f*(float)(which%5));
 setParameter("motifFX",.03f+.042f*(float)(which%6));
 setParameter("mudCut",.45f+.04f*(float)(which%6));
 setParameter("wetLowCut",.55f+.05f*(float)(which%7));
 setParameter("focus",.58f+.04f*(float)(which%6));
 setParameter("trancePulse",.06f+.025f*(float)(which%7));
 setParameter("psySpiral",.20f+.042f*(float)(which%6));
 setParameter("clarity",.93f);setParameter("hiFiAir",.33f);
 setParameter("biomeDepth",.54f+.05f*(float)(which%7));
 setParameter("biomeRate",.012f+.004f*(float)(which%7));
 setParameter("biomeChaos",.28f+.11f*(float)(which%6));
 for(int i=2;i<=4;++i){auto prefix="lfo"+String(i);
  setParameter((prefix+"Depth").toRawUTF8(),.10f+.04f*(float)i);
  setParameter((prefix+"Rate").toRawUTF8(),.001f+.0022f*(float)i);
 }
 showPage(Atlas);
}
void NoctivoreEditor::showPage(Page next){
 page=next;
 for(auto& s:sections)for(auto* c:s.children)c->setVisible(s.page==page);
 for(size_t i=0;i<tabs.size();++i)tabs[i].setToggleState((int)i==(int)page,dontSendNotification);
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
 float sx=getWidth()/1160.f,sy=getHeight()/790.f;
 auto R=[&](float x,float y,float w,float h){return rect(x,y,w,h,sx,sy);};
 for(size_t i=0;i<tabs.size();++i)tabs[i].setBounds(R(18.f+(float)i*143.f,175,137,36));
 actions[0].setBounds(R(778,175,120,36));
 actions[1].setBounds(R(905,175,115,36));
 actions[2].setBounds(R(1026,175,119,36));
 sceneLabel.setBounds(R(22,97,135,18));scenes.setBounds(R(20,116,273,34));
 for(auto& c:controls){
  if(c->getName()=="PERMANENT_ATM")c->setBounds(R(826,79,120,86));
  if(c->getName()=="PERMANENT_FURY")c->setBounds(R(956,79,120,86));
 }
 for(auto& s:sections){
  if(s.page!=page)continue;
  const int localIndex=(int)(&s-&sections[0]);
  if(page==Atlas){
   int order=0;for(const auto& x:sections){if(x.page==Atlas && &x!=&s)++order;if(&x==&s)break;}
   layout(s,R(18+order*379,226,370,440));
  }else if(page==Forge){
   int order=0;for(const auto& x:sections){if(x.page==Forge && &x!=&s)++order;if(&x==&s)break;}
   layout(s,R(18+order*379,226,370,440));
  }else if(page==Motion){
   int order=0;for(const auto& x:sections){if(x.page==Motion && &x!=&s)++order;if(&x==&s)break;}
   if(order<4)layout(s,R(18+order*285,226,275,360));
   else layout(s,R(18,596,1124,70));
  }else if(page==Space){
   int order=0;for(const auto& x:sections){if(x.page==Space && &x!=&s)++order;if(&x==&s)break;}
   layout(s,R(18+order*379,226,370,440));
  }else{
   int order=0;for(const auto& x:sections){if(x.page==System && &x!=&s)++order;if(&x==&s)break;}
   layout(s,R(18+order*379,226,370,440));
  }
  (void)localIndex;
 }
 keyboard.setBounds(R(25,692,1110,65));
}
void NoctivoreEditor::timerCallback(){repaint();}
void NoctivoreEditor::paint(Graphics& g){
 g.fillAll(soil);
 float sx=getWidth()/1160.f,sy=getHeight()/790.f;
 Graphics::ScopedSaveState saved(g);g.addTransform(AffineTransform::scale(sx,sy));
 ColourGradient bg(Colour::fromRGB(37,43,31),0,0,soil,1100,790,false);
 g.setGradientFill(bg);g.fillRect(0,0,1160,790);
 // Desert relief lines are background-only; controls retain precise alignment.
 for(int i=0;i<20;++i){Path ridge;
  float y=20.f+i*38.f;ridge.startNewSubPath(0,y);
  ridge.cubicTo(300,y-26.f,650,y+34.f,1160,y-9.f);
  g.setColour(ochre.withAlpha(.028f));g.strokePath(ridge,PathStrokeType(.8f));}
 g.setColour(ivory);g.setFont(Font(34.f,Font::bold));
 g.drawText("NOCTIVORE",20,13,360,43,Justification::left);
 g.setColour(clay);g.setFont(Font(13.f,Font::bold));
 g.drawText("SHADOW BIOME  /  V9",21,56,275,20,Justification::left);
 g.setColour(soft);g.setFont(Font(11.f));
 g.drawText("SYNC GATES  •  ZERO MELODIC SEQUENCING  •  ULTRA SLOW LFOs",390,25,540,19,Justification::left);
 g.setColour(outline);g.drawLine(20,80,1140,80,1.f);
 g.setColour(bark);g.fillRoundedRectangle({18,93,790,69},9.f);
 g.setColour(outline);g.drawRoundedRectangle({18,93,790,69},9.f,1.f);
 const auto peak=std::clamp(processor.peak.load(std::memory_order_relaxed),0.f,1.f);
 g.setColour(stone);g.fillRoundedRectangle({1110,87,25,76},5.f);
 g.setColour(moss);g.fillRoundedRectangle({1115,158-peak*65,15,peak*65},3.f);
 g.setColour(soft);g.setFont(Font(10.f));g.drawText("OUT",1090,74,64,18,Justification::centred);
 for(auto& s:sections){
  if(s.page!=page)continue;
  auto r=s.bounds.toFloat();r.setX(r.getX()/sx);r.setY(r.getY()/sy);
  r.setWidth(r.getWidth()/sx);r.setHeight(r.getHeight()/sy);
  g.setColour(bark);g.fillRoundedRectangle(r,11.f);
  g.setColour(outline);g.drawRoundedRectangle(r,11.f,1.f);
  g.setColour(s.accent);g.fillRoundedRectangle({r.getX()+13,r.getY()+12,3.f,17.f},1.5f);
  g.setFont(Font(12.f,Font::bold));
  g.drawText(s.title,r.getX()+24,r.getY()+10,r.getWidth()-35,22,Justification::left);
 }
 g.setColour(stone);g.fillRoundedRectangle({18,683,1124,83},10.f);
 g.setColour(outline);g.drawRoundedRectangle({18,683,1124,83},10.f,1.f);
 g.setColour(soft);g.setFont(Font(10.2f));
 if(processor.libraryLoaded)
  g.drawText("50 DIVERSE BANKS / 48 kHz / NON-PITCHED GATES / 4 ULTRA-SLOW LFOs",22,766,900,17,Justification::left);
 else {g.setColour(clay);g.drawText("SAMPLE BANKS NOT FOUND — verify VST3 Contents/Resources/Banks",22,766,930,17,Justification::left);}
 g.setColour(ochre);g.drawText("SHADOW BIOME 9.0",1003,766,140,17,Justification::right);
}
