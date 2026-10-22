#include "PluginProcessor.h"
#if !NOCTIVORE_GENERIC_EDITOR
#include "VoidcoreEditor.h"
#endif
#include <cstdlib>
#if JUCE_WINDOWS
 #include <windows.h>
#elif JUCE_LINUX
 #include <dlfcn.h>
#endif
using namespace juce;
// Module-local address is needed: the DAW executable path is NOT the plugin path.
namespace {
 void moduleLocator() {}
 File moduleResourceFolder() {
#if JUCE_WINDOWS
  HMODULE handle=nullptr;
  if(GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
      reinterpret_cast<LPCWSTR>(&moduleLocator), &handle) && handle) {
   wchar_t path[32768]{};
   if(GetModuleFileNameW(handle,path,32768)>0)
    return File(String(path)).getParentDirectory().getParentDirectory().getChildFile("Resources").getChildFile("Banks");
  }
#elif JUCE_LINUX
  Dl_info info{};
  if(dladdr((void*)&moduleLocator,&info) && info.dli_fname)
   return File(info.dli_fname).getParentDirectory().getParentDirectory().getChildFile("Resources").getChildFile("Banks");
#endif
  return {};
 }
}

AudioProcessorValueTreeState::ParameterLayout NoctivoreProcessor::layout(){
 std::vector<std::unique_ptr<RangedAudioParameter>> a;
 auto add=[&](const char* id,const char* name,float lo,float hi,float def){
  a.push_back(std::make_unique<AudioParameterFloat>(ParameterID{id,1},name,NormalisableRange<float>(lo,hi),def));
 };
 auto choose=[&](const char* id,const char* name,StringArray values,int def=0){
  a.push_back(std::make_unique<AudioParameterChoice>(ParameterID{id,1},name,values,def));
 };
 StringArray names;for(const auto& b:noctivore::banks)names.add(b.name);names.add("User Bank");
 choose("bank","Instrument Bank",names);
 choose("mode","Voice Mode",{"Polyphonic","Mono","Legato"});
 add("attack","Attack",.001f,8,.28f);
 add("decay","Decay",.01f,9,3.2f);
 add("sustain","Sustain",0,1,.93f);
 add("release","Release",.01f,12,4.4f);
 choose("filterMode","Filter Mode",{"Low Pass","Band Pass","High Pass"});
 add("cutoff","Filter Cutoff",.015f,1,.95f);
 add("resonance","Filter Resonance",0,1,.14f);
 a.push_back(std::make_unique<AudioParameterFloat>(ParameterID{"rate",1},"LFO 1 Rate",
      NormalisableRange<float>(.001f,35.f,0.f,3.f),.035f));
 add("depth","LFO 1 Depth",0,1,0.f);
 choose("wave","LFO 1 Shape",{"Sine","Triangle","Saw","Square","Sample & Hold","Fractal"});
 StringArray modTargets{"Volume","Pan","Pitch","Filter 1","Filter 2","FM Index","A/B Morph","Metal Exciter","Grain Density","Delay Warp","Reverb Motion"};
 choose("target","LFO 1 Target",modTargets,3);
 choose("lfo1Sync","LFO 1 Sync",{"Free","1 bar","1/2 bar","1/4 bar","1/8 bar","1/16 bar"});
 for(int i=2;i<=4;++i){
   auto j=String(i);
   String r="lfo"+j+"Rate",d="lfo"+j+"Depth",w="lfo"+j+"Wave",t="lfo"+j+"Target",sy="lfo"+j+"Sync";
   auto addSlow=[&](const String& id,const String& name,float low,float high,float def){
     a.push_back(std::make_unique<AudioParameterFloat>(ParameterID{id,1},name,
       NormalisableRange<float>(low,high,0.f,3.f),def));
   };
   addSlow(r,"LFO "+j+" Rate",.001f,35.f,i==2?.022f:i==3?.071f:.12f);
   addSlow(d,"LFO "+j+" Depth",0.f,1.f,0.f);
   choose(w.toRawUTF8(),("LFO "+j+" Shape").toRawUTF8(),{"Sine","Triangle","Saw","Square","Sample & Hold","Fractal"});
   choose(t.toRawUTF8(),("LFO "+j+" Target").toRawUTF8(),modTargets,i==2?6:i==3?5:4);
   choose(sy.toRawUTF8(),("LFO "+j+" Sync").toRawUTF8(),{"Free","1 bar","1/2 bar","1/4 bar","1/8 bar","1/16 bar"});
 }
 add("glide","Glide",0,1.5f,.07f);
 add("space","Space",0,.95f,.38f);
 add("volume","Volume",0,1.5f,.76f);
 add("pan","Pan",-1,1,0);
 choose("quality","Resampling Quality",{"Linear","Cubic"},1);
 choose("velocity","Velocity Curve",{"Soft","Normal","Hard"},1);
 add("bendRange","Bend Range",0,12,2);
 add("bend","Pitch Bend",-1,1,0);
 add("start","Sample Start",0,.7f,0);
 choose("reverse","Playback Direction",{"Forward","Reverse"});
 add("drive","Drive",0,1,.04f);
 add("stereo","Stereo Width",0,1.5f,1.13f);
 add("organic","Organic Drift",0,1,.73f);
 add("layers","Living Layers",0,1,.27f);
 add("movement","Micro Movement",0,1,.52f);
 add("sub","Sub Body",0,1,.08f);
 choose("division","Delay Division",{"1/4","1/8","1/8 dotted","1/16","1/16 dotted","1/32","1/4 triplet","1/8 triplet"},3);
 add("delayMix","Delay Wet",0,1,0.f);
 add("delayFeedback","Delay Feedback",0,.92f,.51f);
 add("delayTone","Delay Brightness",0,1,.59f);
 add("delayWarp","Delay Warp",0,1,.35f);
 add("delaySpread","Ping-Pong",0,1,.87f);
 add("delayDiffusion","Delay Smear",0,1,.52f);
 add("reverbMix","Reverb Wet",0,1,0.f);
 add("reverbSize","Reverb Size",0,1,.94f);
 add("reverbDecay","Reverb Decay",0,1,.78f);
 add("reverbDark","Reverb Darkness",0,1,.49f);
 add("reverbMod","Reverb Mutation",0,1,.71f);
 add("reverbPreDelay","Reverb Predelay",0,1,.19f);
 add("shimmer","Spectral Bloom",0,1,.09f);
 add("duck","FX Duck",0,1,.24f);
 // New controls appended: keeps all legacy V4 parameter IDs intact for Ableton.
 choose("bankB","Source B Bank",names,7);
 choose("textureMode","Texture Algorithm",{"Flow","Grain Cloud","Orbital","Pulsar","Liquid Warp","Time Freeze"});
 add("morph","Source Crossfade",0,1,0.f);
 add("morphMotion","Autonomous Morph",0,1,0.f);
 add("morphRate","Morph Rate (Hz)",.005f,1.2f,.065f);
 add("texture","Algorithm Intensity",0,1,.42f);
 add("grain","Grain Density",0,1,.36f);
 add("scatter","Time Scatter",0,1,.31f);
 add("pulseRate","Pulse (beats)",.25f,16.f,2.f);
 add("spectralTilt","Spectral Color",-1,1,0.f);

 // V6: the parameters below are additive so V5 sessions retain their settings.
 choose("filter2Mode","Filter 2 Mode",{"Off","Low Pass","Band Pass","High Pass","Notch"});
 choose("filterRoute","Filter Topology",{"Serial","Parallel","Split"});
 add("cutoff2","Filter 2 Cutoff",.008f,1.f,.70f);
 add("resonance2","Filter 2 Resonance",0.f,.95f,.15f);
 add("filterBlend","Filter 2 Mix",0.f,1.f,.50f);
 add("filterDrive","Filter Drive",0.f,1.f,.025f);
 add("keytrack","Filter Key Track",0.f,1.f,.12f);
 add("filterEnvAttack","Filter Env Attack",.003f,10.f,.3f);
 add("filterEnvDecay","Filter Env Decay",.01f,14.f,2.5f);
 add("filterEnvSustain","Filter Env Sustain",0.f,1.f,.4f);
 add("filterEnvRelease","Filter Env Release",.01f,15.f,3.f);
 add("filterEnvDepth","Filter Env Amount",-1.f,1.f,0.f);
 add("motionEnvAttack","Motion Env Attack",.003f,15.f,.5f);
 add("motionEnvRelease","Motion Env Release",.01f,20.f,4.f);
 add("motionEnvDepth","Motion Env Amount",0.f,1.f,.6f);
 add("fmMix","FM Amount",0.f,1.f,0.f);
 add("fmRatio","FM Ratio",.25f,16.f,1.f);
 add("fmIndex","FM Index",0.f,12.f,.85f);
 add("fmFeedback","FM Feedback",0.f,.95f,.025f);
 add("fmInharmonic","FM Inharmonicity",0.f,1.f,.035f);
 add("metalMix","Metal Resonator",0.f,1.f,0.f);
 add("metalTune","Metal Tune",0.f,1.f,.30f);
 add("metalDecay","Metal Decay",0.f,1.f,.6f);
 add("metalTone","Metal Tone",0.f,1.f,.42f);
 add("macroAtmosphere","Macro Atmosphere",0.f,1.f,.64f);
 add("macroAggression","Macro Aggression",0.f,1.f,0.f);
 // V7: independent musical groove and high-fidelity controls.
 add("trancePulse","Psy Pulse",0.f,1.f,0.f);
 add("tranceRate","Pulse Speed",.25f,8.f,2.f);
 add("psySpiral","Psy Spiral",0.f,1.f,.24f);
 add("psySpiralRate","Spiral Rate (Hz)",.005f,2.f,.16f);
 add("hiFiAir","Hi-Fi Air",0.f,1.f,0.f);
 add("clarity","Master Clarity",0.f,1.f,1.f);
 // V9 rhythmic-only shadow gate (legacy parameters retained for automation).
 choose("motif","Ritual Motif",{"Off","Serpent","Ritual","Insect","Portal","Fracture","Hollow"},0);
 choose("motifDivision","Motif Grid",{"1/8","1/16","1/32","Triplet"},1);
 add("motifPitch","Pitch Leap",0.f,1.f,.42f);
 add("motifGate","Motif Gate",0.f,1.f,.42f);
 add("motifAccent","Ritual Accents",0.f,1.f,.65f);
 add("motifGlide","Pitch Glide",0.f,1.f,.25f);
 add("motifFX","Resonant Events",0.f,1.f,0.f);
 add("mudCut","Low-Mid Cleanup",0.f,1.f,.52f);
 add("wetLowCut","FX Low Cut",0.f,1.f,.64f);
 add("focus","Stereo Focus",0.f,1.f,.68f);
 // V9: the living biome has a real spectral/texture motion engine.
 add("biomeDepth","Biome Life",0.f,1.f,.72f);
 a.push_back(std::make_unique<AudioParameterFloat>(ParameterID{"biomeRate",1},"Biome Evolution (Hz)",
   NormalisableRange<float>(.002f,.35f,0.f,2.8f),.034f));
 add("biomeChaos","Biome Complexity",0.f,1.f,.60f);
 // v10: optional expressive resonant interaction, never continuous noise.
 add("xenoFormant","Alien Formants",0.f,1.f,0.f);
 add("xenoCross","Organism Coupling",0.f,1.f,0.f);
 add("xenoWrithe","Body Mutation",0.f,1.f,.54f);
 add("lowRegisterLife","Low Register Life",0.f,1.f,0.f);
 add("coupledMotion","Coupled Motion",0.f,1.f,.53f);
 add("performanceMod","Performance Mod Wheel",0.f,1.f,0.f);
 choose("voiceLimit","CPU / Polyphony",{"8 voices","12 voices","16 voices","24 voices"},1);
 add("colony","Mutant Colony",0.f,1.f,0.f);
 add("creatures","Living Creatures",0.f,1.f,.39f);
 add("psyFold","Spectral Fold",0.f,1.f,.22f);
 add("symbiosis","Source Symbiosis",0.f,1.f,0.f);
 add("predator","Predator Throat",0.f,1.f,0.f);
 add("insectCode","Spore Code",0.f,1.f,0.f);
 add("burrow","Burrow Interference",0.f,1.f,0.f);
 for(int i=0;i<16;++i){
  auto key=juce::String::formatted("gateStep%02d",i+1);
  auto label=juce::String("Forest Gate Step ")+juce::String(i+1);
  add(key.toRawUTF8(),label.toRawUTF8(),0.f,1.f,1.f);
 }

 // New controls are additive; existing preset parameter IDs stay intact.
 // Final one-page source and tone controls (actual DSP parameters).
 add("tuneA","Source A Semitones",-12.f,12.f,0.f);
 add("tuneB","Source B Semitones",-12.f,12.f,0.f);
 add("levelA","Source A Level",0.f,1.5f,1.f);
 add("levelB","Source B Level",0.f,1.5f,1.f);
 add("toneLow","Tone Low",0.f,1.f,.5f);
 add("toneMid","Tone Mid",0.f,1.f,.5f);
 add("toneHigh","Tone High",0.f,1.f,.5f);
 add("character","XENO Exciter Amount",0.f,1.f,0.f);
 choose("sourceSolo","Pure Source Audition",{"Normal Mix","Pure Source"});
 return {a.begin(),a.end()};
}
juce::File NoctivoreProcessor::paintingFile() const {
 return moduleResourceFolder().getParentDirectory().getChildFile("GUI").getChildFile("painted_forest.jpg");
}
NoctivoreProcessor::NoctivoreProcessor():
 AudioProcessor(BusesProperties().withOutput("Stereo Output",AudioChannelSet::stereo(),true)),
 state(*this,nullptr,"NOCTIVORE_FX_STATE",layout()){
 const char* env=std::getenv("NOCTIVORE_BANK_PATH");
 if(env&&*env)libraryLoaded=sampler.loadLibrary(env);
 if(!libraryLoaded) {
  auto folder=moduleResourceFolder();
  if(folder.isDirectory())libraryLoaded=sampler.loadLibrary(folder.getFullPathName().toStdString());
 }
 if(!libraryLoaded){
  auto p=File::getCurrentWorkingDirectory().getChildFile("Banks");
  libraryLoaded=sampler.loadLibrary(p.getFullPathName().toStdString());
 }
 if(!libraryLoaded){
  auto home=File::getSpecialLocation(File::userHomeDirectory);
  StringArray paths={".vst3/NOCTIVORE_FX.vst3/Contents/Resources/Banks",
      "Library/Audio/Plug-Ins/VST3/NOCTIVORE_FX.vst3/Contents/Resources/Banks"};
  for(auto relative:paths)if(!libraryLoaded)
    libraryLoaded=sampler.loadLibrary(home.getChildFile(relative).getFullPathName().toStdString());
 }
}
void NoctivoreProcessor::prepareToPlay(double sampleRate,int){sampler.prepare(sampleRate);}
bool NoctivoreProcessor::isBusesLayoutSupported(const BusesLayout& layout)const{
 return layout.getMainOutputChannelSet()==AudioChannelSet::stereo() && layout.getMainInputChannelSet().isDisabled();
}
void NoctivoreProcessor::processBlock(AudioBuffer<float>& output,MidiBuffer& events){
 ScopedNoDenormals safe;output.clear();
 keyboardState.processNextMidiBuffer(events,0,output.getNumSamples(),true);
 noctivore::Params p;
 p.bank=(int)get("bank");p.mode=(int)get("mode");
 p.attack=get("attack");p.decay=get("decay");p.sustain=get("sustain");p.release=get("release");
 p.filterMode=(int)get("filterMode");p.cutoff=get("cutoff");p.resonance=get("resonance");
 p.lfoRate=get("rate");p.lfoDepth=get("depth");p.lfoWave=(int)get("wave");p.lfoTarget=(int)get("target");
 p.glide=get("glide");p.space=get("space");p.volume=get("volume");p.pan=get("pan");
 p.resampling=(int)get("quality");p.velocityCurve=(int)get("velocity");
 p.bendRange=get("bendRange");p.bend=get("bend");p.sampleStart=get("start");
 p.reverse=get("reverse");p.drive=get("drive");p.stereo=get("stereo");
 p.organic=get("organic");p.layers=get("layers");p.movement=get("movement");p.sub=get("sub");
 p.bankB=(int)get("bankB");p.textureMode=(int)get("textureMode");
 p.morph=get("morph");p.morphMotion=get("morphMotion");p.morphRate=get("morphRate");
 p.texture=get("texture");p.grain=get("grain");p.scatter=get("scatter");
 p.pulseRate=get("pulseRate");p.spectralTilt=get("spectralTilt");
 p.lfos[0]={get("rate"),get("depth"),(int)get("wave"),(int)get("target"),(int)get("lfo1Sync")};
 for(int i=1;i<4;++i){
  String prefix="lfo"+String(i+1);
  p.lfos[(size_t)i]={get((prefix+"Rate").toRawUTF8()),get((prefix+"Depth").toRawUTF8()),
   (int)get((prefix+"Wave").toRawUTF8()),(int)get((prefix+"Target").toRawUTF8()),(int)get((prefix+"Sync").toRawUTF8())};
 }
 p.filter2Mode=(int)get("filter2Mode");p.filterRoute=(int)get("filterRoute");
 p.cutoff2=get("cutoff2");p.resonance2=get("resonance2");p.filterBlend=get("filterBlend");
 p.filterDrive=get("filterDrive");p.keytrack=get("keytrack");
 p.filterEnvAttack=get("filterEnvAttack");p.filterEnvDecay=get("filterEnvDecay");
 p.filterEnvSustain=get("filterEnvSustain");p.filterEnvRelease=get("filterEnvRelease");p.filterEnvDepth=get("filterEnvDepth");
 p.motionEnvAttack=get("motionEnvAttack");p.motionEnvRelease=get("motionEnvRelease");p.motionEnvDepth=get("motionEnvDepth");
 p.fmMix=get("fmMix");p.fmRatio=get("fmRatio");p.fmIndex=get("fmIndex");
 p.fmFeedback=get("fmFeedback");p.fmInharmonic=get("fmInharmonic");
 p.metalMix=get("metalMix");p.metalTune=get("metalTune");p.metalDecay=get("metalDecay");p.metalTone=get("metalTone");
 p.macroAtmosphere=get("macroAtmosphere");p.macroAggression=get("macroAggression");
 p.trancePulse=get("trancePulse");p.tranceRate=get("tranceRate");
 p.psySpiral=get("psySpiral");p.psySpiralRate=get("psySpiralRate");
 p.hiFiAir=get("hiFiAir");p.clarity=get("clarity");
 p.motif=(int)get("motif");p.motifDivision=(int)get("motifDivision");
 p.motifPitch=get("motifPitch");p.motifGate=get("motifGate");
 p.motifAccent=get("motifAccent");p.motifGlide=get("motifGlide");p.motifFX=get("motifFX");
 p.mudCut=get("mudCut");p.wetLowCut=get("wetLowCut");p.focus=get("focus");
 p.biomeDepth=get("biomeDepth");p.biomeRate=get("biomeRate");p.biomeChaos=get("biomeChaos");
 p.xenoFormant=get("xenoFormant");p.xenoCross=get("xenoCross");p.xenoWrithe=get("xenoWrithe");
 p.lowRegisterLife=get("lowRegisterLife");p.coupledMotion=get("coupledMotion");
 p.performanceMod=get("performanceMod");
 {static constexpr int limits[]={8,12,16,24};
  p.maxVoices=limits[std::clamp((int)get("voiceLimit"),0,3)];}
 p.colony=get("colony");p.creatures=get("creatures");
 p.psyFold=get("psyFold");p.symbiosis=get("symbiosis");
 p.predator=get("predator");p.insectCode=get("insectCode");p.burrow=get("burrow");
 p.character=get("character");p.sourceSolo=get("sourceSolo");
 p.tuneA=get("tuneA");p.tuneB=get("tuneB");
 p.levelA=get("levelA");p.levelB=get("levelB");
 p.toneLow=get("toneLow");p.toneMid=get("toneMid");p.toneHigh=get("toneHigh");
 for(int i=0;i<16;++i){auto key=juce::String::formatted("gateStep%02d",i+1);
  p.gateSteps[(size_t)i]=get(key.toRawUTF8());
 }
 p.fx.division=(int)get("division");
 p.fx.delayMix=get("delayMix");p.fx.delayFeedback=get("delayFeedback");p.fx.delayTone=get("delayTone");
 p.fx.delayWarp=get("delayWarp");p.fx.delaySpread=get("delaySpread");p.fx.delayDiffusion=get("delayDiffusion");
 p.fx.reverbMix=get("reverbMix");p.fx.reverbSize=get("reverbSize");p.fx.reverbDecay=get("reverbDecay");
 p.fx.reverbDark=get("reverbDark");p.fx.reverbMod=get("reverbMod");p.fx.reverbPreDelay=get("reverbPreDelay");
 p.fx.shimmer=get("shimmer");p.fx.duck=get("duck");
 // Preserve old saved 'Space' settings as a macro on the new reverb amount.
 p.fx.reverbMix=noctivore::clampf(p.fx.reverbMix*(.65f+get("space")),0,1);
 if(auto* playhead=getPlayHead())
  if(auto position=playhead->getPosition()) {
   if(auto bpm=position->getBpm()){
    sampler.setTempo(*bpm);
    displayTempo.store((float)*bpm,std::memory_order_relaxed);
   }
   displayPlaying.store(position->getIsPlaying(),std::memory_order_relaxed);
   if(auto ppq=position->getPpqPosition())
    displayPpq.store((float)*ppq,std::memory_order_relaxed);
   // Quantized motifs lock to Live's beat grid when transport is running;
   // when stopped the keyboard can still audition freely.
   if(position->getIsPlaying())
    if(auto ppq=position->getPpqPosition())sampler.setBeatClock(*ppq);
  }
 sampler.setParameters(p);
 int cur=0,n=output.getNumSamples();float* left=output.getWritePointer(0),*right=output.getWritePointer(1);
 // Add GUI keyboard notes into this very same MIDI stream, on the audio thread.
 // DAW MIDI continues to work unchanged; there is no synth engine on the UI thread.
 keyboardState.processNextMidiBuffer(events,0,n,true);
 for(const auto event:events){
  int pos=jlimit(cur,n,event.samplePosition);
  if(pos>cur)sampler.render(left+cur,right+cur,pos-cur);
  auto message=event.getMessage();
  if(message.isNoteOn())sampler.noteOn(message.getNoteNumber(),message.getFloatVelocity());
  else if(message.isNoteOff())sampler.noteOff(message.getNoteNumber());
  else if(message.isPitchWheel())sampler.pitchWheel(message.getPitchWheelValue());
  else if(message.isController())sampler.controller(message.getControllerNumber(),message.getControllerValue());
  else if(message.isAllNotesOff()||message.isAllSoundOff())sampler.allNotesOff();
  cur=pos;
 }
 if(cur<n)sampler.render(left+cur,right+cur,n-cur);
 const float leftMagnitude=output.getMagnitude(0,0,n);
 const float rightMagnitude=output.getMagnitude(1,0,n);
 const float v=jmax(leftMagnitude,rightMagnitude);
 peakLeft.store(leftMagnitude,std::memory_order_relaxed);
 peakRight.store(rightMagnitude,std::memory_order_relaxed);
 peak.store(jmax(v,peak.load(std::memory_order_relaxed)*.88f),std::memory_order_relaxed);
 // Low-rate display trace, safe for the message thread to read.
 for(int s=0;s<n;s+=64){auto k=scopeCursor.fetch_add(1,std::memory_order_relaxed)%scope.size();
  scope[k].store(.5f*(left[s]+right[s]),std::memory_order_relaxed);}
}
void NoctivoreProcessor::getStateInformation(MemoryBlock& dest){if(auto xml=state.copyState().createXml())copyXmlToBinary(*xml,dest);}
void NoctivoreProcessor::setStateInformation(const void* bytes,int size){
 if(auto xml=getXmlFromBinary(bytes,size))if(xml->hasTagName(state.state.getType()))state.replaceState(ValueTree::fromXml(*xml));
}
AudioProcessorEditor* NoctivoreProcessor::createEditor(){
#if NOCTIVORE_GENERIC_EDITOR
 return new juce::GenericAudioProcessorEditor(*this);
#else
 return new VoidcoreEditor(*this);
#endif
}
AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new NoctivoreProcessor();}
