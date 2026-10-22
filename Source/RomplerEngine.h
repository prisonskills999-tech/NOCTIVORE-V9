#pragma once
// Original C++17 multi-zone sample playback and effects engine for NOCTIVORE.
// Self-contained core: does not depend on JUCE, VST3, or proprietary media.
#include "BankCatalog.h"
#include "PsySpace.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

namespace noctivore {
constexpr float pi=3.14159265358979323846f;
inline float clampf(float x,float a,float b){return std::max(a,std::min(b,x));}
struct LFOConfig {
 float rate=.009f, depth=0.f;
 int wave=0, target=0, sync=0;
};
struct Params {
 int bank=0, mode=0, velocityCurve=1, resampling=1, lfoWave=0, lfoTarget=0, filterMode=0;
 float attack=.28f,decay=3.2f,sustain=.93f,release=4.4f;
 float cutoff=.95f, resonance=.14f,glide=.07f,space=.34f,volume=.76f,pan=0.f;
 float lfoRate=.009f,lfoDepth=0.f,bendRange=2.f,bend=0.f;
 float sampleStart=0.f,reverse=0.f,drive=.04f,stereo=1.13f;
 float organic=.55f, layers=.32f, movement=.42f, sub=.10f;
 // V5 dual-bank soundscape engine (new parameters appended for V4 state compatibility).
 int bankB=7, textureMode=0;
 float morph=0.f, morphMotion=.06f, morphRate=.065f, texture=.42f;
 float grain=.30f, scatter=.31f, pulseRate=2.f, spectralTilt=0.f;
 // V6 Earthforge: four independent, host-syncable modulators, including
 // 0.001 Hz cycles (one cycle per 16.7 minutes) and ultra-fine depths.
 std::array<LFOConfig,4> lfos{};
 int filter2Mode=0, filterRoute=0; // off/LP/BP/HP/notch; serial/parallel/split
 float cutoff2=.70f,resonance2=.15f,filterBlend=.50f,filterDrive=.12f,keytrack=.12f;
 float filterEnvAttack=.30f,filterEnvDecay=2.5f,filterEnvSustain=.4f,filterEnvRelease=3.f,filterEnvDepth=0.f;
 float motionEnvAttack=.5f,motionEnvRelease=4.f,motionEnvDepth=.6f;
 float fmMix=.014f,fmRatio=1.0f,fmIndex=.85f,fmFeedback=.025f,fmInharmonic=.035f;
 float metalMix=0.f,metalTune=.30f,metalDecay=.45f,metalTone=.30f;
 float macroAtmosphere=.48f,macroAggression=.02f;
 // V7: musical hypnotic motion and open, less saturated high-fidelity output.
 float trancePulse=.08f,tranceRate=2.f,psySpiral=.24f,psySpiralRate=.16f;
 float hiFiAir=.24f,clarity=1.f;
 // V9: tempo-locked non-melodic gates, accents and controlled space.
 int motif=0, motifDivision=1; // Off or six non-pitched rhythmic gates
 float motifPitch=.36f, motifGate=.42f, motifAccent=.65f, motifGlide=.18f,motifFX=0.f;
 float focus=.68f, mudCut=.52f, wetLowCut=.64f;
 float biomeDepth=.72f,biomeRate=.034f,biomeChaos=.60f;
 // v10: physically inspired noise-free cross excitation and resonant vocal tract.
 float xenoFormant=.31f, xenoCross=.12f, xenoWrithe=.54f;
 // V10.1: tuned life below MIDI note 57 and correlated slow organic motion.
 float lowRegisterLife=.72f, coupledMotion=.53f;
 int maxVoices=12;
 // V10.6: no noise generator, transient strike, or preprogrammed MIDI jumps.
 float colony=.18f, creatures=.12f, psyFold=.0f, symbiosis=.10f;
 // v10.9 — three contrasting dark-forest organisms, always sample-linked.
 float predator=.08f, insectCode=.06f, burrow=.06f;
 float performanceMod=0.f;
 // V10.5 user-editable 16 step gate: default all steps active.
 std::array<float,16> gateSteps{{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}};


 PsySpace::Controls fx{};
};
struct Sample {
 std::vector<float> data;
 int rate=24000,root=60;
 bool loaded()const{return data.size()>16;}
 float linear(float pos)const {
  if(!loaded()||pos<0||pos>=(float)(data.size()-1))return 0;
  const size_t i=(size_t)pos;const float frac=pos-(float)i;
  return data[i]+(data[i+1]-data[i])*frac;
 }
 float cubic(float pos)const {
  if(!loaded()||pos<1||pos>=(float)(data.size()-3))return linear(pos);
  size_t i=(size_t)pos;float t=pos-(float)i;
  float a=data[i-1],b=data[i],c=data[i+1],d=data[i+2];
  return b + .5f*t*(c-a+t*(2*a-5*b+4*c-d+t*(3*(b-c)+d-a)));
 }
  // Polyphase 16-tap band-limited sample playback. Offline-initialized
  // interpolation kernels reject Nyquist foldback when pitching WAVs upward.
  // No allocations, locks or trigonometry in the sample interpolation path.
  struct SincBank {
   static constexpr int phases=256,taps=16,bands=13;
   std::array<std::array<std::array<float,taps>,phases>,bands> coeffs{};
   std::array<float,bands> cutoff{{.96f,.90f,.83f,.76f,.69f,.62f,.55f,
                                   .48f,.40f,.32f,.23f,.16f,.11f}};
   SincBank(){
    for(int band=0;band<bands;++band)
     for(int phase=0;phase<phases;++phase){
      const float fraction=(float)phase/(float)phases;
      float sum=0.f;
      for(int tap=0;tap<taps;++tap){
       const float d=(float)(tap-7)-fraction;
       const float z=pi*d;
       const float lowpass=std::fabs(d)<.000001f?cutoff[(size_t)band]:
          std::sin(cutoff[(size_t)band]*z)/z;
       const float window=.5f+.5f*std::cos(pi*d/8.f);
       const float k=lowpass*std::max(0.f,window);
       coeffs[(size_t)band][(size_t)phase][(size_t)tap]=k;
       sum+=k;
      }
      if(std::fabs(sum)>1e-8f)
       for(auto& k:coeffs[(size_t)band][(size_t)phase])k/=sum;
     }
   }
  };
  static const SincBank& sincBank(){
   static const SincBank table;
   return table;
  }
  static void warmUpResampler(){(void)sincBank();}
  float bandlimited(float pos,float ratio)const {
   if(!loaded()||pos<0.f||pos>=(float)data.size()-1.f)return 0.f;
   if(ratio<=1.01f)return cubic(pos);
   const auto& lut=sincBank();
   const float allowed=.96f/std::max(1.f,ratio);
   int band=0;
   while(band<SincBank::bands-1&&lut.cutoff[(size_t)band]>allowed)++band;
   const int centre=(int)pos;
   const float fraction=pos-(float)centre;
   const int phase=std::clamp((int)(fraction*SincBank::phases),0,SincBank::phases-1);
   const auto& kernels=lut.coeffs[(size_t)band][(size_t)phase];
   float result=0.f;
   for(int tap=0;tap<SincBank::taps;++tap){
    const int n=centre+tap-7;
    if(n>=0&&n<(int)data.size())result+=kernels[(size_t)tap]*data[(size_t)n];
   }
   return result;
  }
};
inline uint32_t read4(const uint8_t* x){return (uint32_t)x[0]|(uint32_t(x[1])<<8)|(uint32_t(x[2])<<16)|(uint32_t(x[3])<<24);}
inline uint16_t read2(const uint8_t* x){return (uint16_t)x[0]|uint16_t(x[1]<<8);}
inline bool readWave(const std::string& file,Sample& out,int root){
 std::ifstream f(file,std::ios::binary|std::ios::ate);if(!f)return false;
 const auto size=f.tellg();if(size<44||size>20*1024*1024)return false;
 std::vector<uint8_t> bytes((size_t)size);f.seekg(0);f.read((char*)bytes.data(),size);
 if(!f||std::string((char*)bytes.data(),4)!="RIFF"||std::string((char*)bytes.data()+8,4)!="WAVE")return false;
 uint16_t format=0,channels=0,bits=0;uint32_t rate=0;size_t offset=0,length=0;
 for(size_t cursor=12;cursor+8<=bytes.size();){
  uint32_t n=read4(bytes.data()+cursor+4);size_t start=cursor+8;
  if(n>bytes.size()-start)break;
  if(std::string((char*)bytes.data()+cursor,4)=="fmt "&&n>=16){
   format=read2(bytes.data()+start);channels=read2(bytes.data()+start+2);
   rate=read4(bytes.data()+start+4);bits=read2(bytes.data()+start+14);
  }
  if(std::string((char*)bytes.data()+cursor,4)=="data"){offset=start;length=n;}
  cursor=start+n+(n&1);
 }
 if(format!=1||channels!=1||bits!=16||rate<8000||length<32||length%2)return false;
 out.data.resize(length/2);out.rate=(int)rate;out.root=root;
 for(size_t i=0;i<out.data.size();++i){uint16_t raw=read2(bytes.data()+offset+i*2);out.data[i]=(int16_t)raw*(1.f/32768.f);}
 return true;
}
class Engine {
public:
 static constexpr int voiceCount=24;
 Engine(){Sample::warmUpResampler();prepare(44100.);}
 bool loadLibrary(const std::string& path){
  bool any=false;bankFolder=path;
  for(int bank=0;bank<(int)banks.size();++bank){
   for(int zone=0;zone<4;++zone){
    char fn[64];std::snprintf(fn,sizeof(fn),"/%02d_%d.wav",bank,rootNotes[(size_t)zone]);
    any|=readWave(path+fn,samples[(size_t)bank][(size_t)zone],rootNotes[(size_t)zone]);
   }
  }
  // Optional bank from user's own WAV recordings, see README (36/48/60/72.wav).
  for(int zone=0;zone<4;++zone){
   std::string file=path+"/User/"+std::to_string(rootNotes[(size_t)zone])+".wav";
   readWave(file,samples[50][(size_t)zone],rootNotes[(size_t)zone]);
  }
  return any;
 }
 int availableZones(int bank)const {
  if(bank<0||bank>50)return 0;
  int result=0;
  for(const auto& s:samples[(size_t)bank])if(s.loaded())++result;
  return result;
 }
 void prepare(double sampleRate){
  sr=std::max(8000.,sampleRate);for(auto& v:voices)v={};
  spaceFX.prepare(sr);mixLowL=mixLowR=0;mixMidL=mixMidR=0;
  lfoPhase=0;for(auto& l:modulators)l={};globalTime=0;motifBeats=0;hostBeatSynced=false;held.fill(false);sustained.fill(false);sustainDown=false;smoothedGate=1.f;smoothedAccent=1.f;
 }
 void setParameters(const Params& values){
  p=values;auto fx=p.fx;fx.psySpiral=p.psySpiral;fx.psySpiralRate=p.psySpiralRate;
  fx.wetLowCut=p.wetLowCut;
  spaceFX.setControls(fx);
 }
 void setTempo(double bpm){tempo=clampf((float)bpm,30.f,300.f);spaceFX.setTempo(bpm);}
 void setBeatClock(double ppq){
   if(!std::isfinite(ppq))return;
   const double position=std::fmod(ppq,65536.);
   if(!hostBeatSynced || std::abs(position-motifBeats)>.065){
     motifBeats=position;hostBeatSynced=true;
   }
 }
 void pitchWheel(int value){bend=clampf(((float)value-8192.f)/8192.f,-1,1);}
 void controller(int cc,int value){
  if(cc==1)modwheel=clampf((float)value/127,0,1);
  if(cc==64){bool next=value>=64;if(!next&&sustainDown){for(int n=0;n<128;++n)if(sustained[(size_t)n]){sustained[(size_t)n]=false;releaseKey(n);}}
   sustainDown=next;}
  if(cc==123||cc==120)allNotesOff();
 }
 void allNotesOff(){for(auto& v:voices)if(v.stage)v.stage=4;held.fill(false);sustained.fill(false);}
 void noteOn(int note,float velocity){
  if(note<0||note>127||velocity<=0)return;
  held[(size_t)note]=true;sustained[(size_t)note]=false;
  const int mode=std::clamp(p.mode,0,2);
  Voice* v=nullptr;bool retrigger=true;
  if(mode!=0){
   v=&voices[0];retrigger=mode==1||v->stage==0;
  }else{
   const int limit=std::clamp(p.maxVoices,1,voiceCount);
   for(int i=0;i<limit;++i)if(voices[(size_t)i].stage==0){v=&voices[(size_t)i];break;}
   if(!v)v=&*std::min_element(voices.begin(),voices.begin()+limit,
    [](const Voice&a,const Voice&b){return a.level<b.level;});
  }
  if(!v)return;
  const bool stolen=v->stage!=0;
  const float previousL=stolen?v->lastL:0.f,previousR=stolen?v->lastR:0.f;
  float oldPitch=v->pitch;
  int bank=std::clamp(p.bank,0,50);
  int z=std::clamp((int)std::lround((note-36)/12.f),0,3);
  if(!samples[(size_t)bank][(size_t)z].loaded()){
   for(int k=0;k<4;++k)if(samples[(size_t)bank][(size_t)k].loaded()){z=k;break;}
  }
  if(!samples[(size_t)bank][(size_t)z].loaded())return;
  v->sample=&samples[(size_t)bank][(size_t)z];
  int bankB=std::clamp(p.bankB,0,50);
  int zb=z;
  if(!samples[(size_t)bankB][(size_t)zb].loaded())
   for(int k=0;k<4;++k)if(samples[(size_t)bankB][(size_t)k].loaded()){zb=k;break;}
  v->sampleB=samples[(size_t)bankB][(size_t)zb].loaded()
   ? &samples[(size_t)bankB][(size_t)zb] : v->sample;
  v->bank=bank;v->note=note;v->vel=clampf(velocity,0,1);
  if(p.velocityCurve==0)v->vel=std::sqrt(v->vel);
  else if(p.velocityCurve==2)v->vel=v->vel*v->vel;
  v->target=(float)note;
  v->pitch=(p.glide>.002f&&oldPitch>0)?oldPitch:(float)note;
  v->age=++voiceCounter;
  // Each note gets its own repeatable phase/pitch drift so layers do not phase-lock.
  v->motionPhase=(float)((v->age*41+note*13)%101)/101.f*2.f*pi;
  v->bodyPhase=0;v->fmPhase=0;v->modPhase=0;v->prevMod=0;
  v->stealL=previousL;v->stealR=previousR;v->stealFade=stolen?1.f:0.f;
  v->lastL=v->lastR=0.f;v->fractureGain=1.f;
  v->smoothMorph=clampf(p.morph,0.f,1.f);
  v->smoothFmMix=clampf(p.fmMix,0.f,1.f);
  v->smoothFold=clampf(p.psyFold,0.f,1.f);
   v->cellPhase=v->organismPhase=v->organismTonePhase=v->foldLP=v->cleanBlend=0.f;
   v->predatorPhase=v->insectPhase=v->signalPhase=0.f;
  v->aliveA=v->aliveB=0.f;
  v->metalPos=0;v->metalDamp=0;v->fxPhase=0;v->fxStep=-1;v->filterEnv=0;v->filterAge=0;v->motionEnv=0;
  std::fill(v->metalLine.begin(),v->metalLine.end(),0.f);
  v->lp2=0;v->bp2=0;v->xenoLp1=v->xenoBp1=v->xenoLp2=v->xenoBp2=0;v->airLP=0;v->motifPitch=0;v->ghostLP=0;
  v->noiseState=(uint32_t)(v->age*747796405ULL+note*2891336453ULL+17);v->breath=0;
  v->layerOffset=(float)samples[(size_t)bank][(size_t)z].rate*(.10f+.15f*(float)((note*7+v->age)%11)/10.f);
  v->oneShot=bank<50?banks[(size_t)bank].oneShot:false;
  if(retrigger){
   v->position=clampf(p.sampleStart,0.f,.7f)*(float)v->sample->data.size();
   v->positionB=clampf(p.sampleStart,0.f,.7f)*(float)v->sampleB->data.size();
   v->aliveA=v->position;v->aliveB=v->positionB;
   if(p.reverse>.5f){v->position=(float)v->sample->data.size()-4.f;
     v->positionB=(float)v->sampleB->data.size()-4.f;
     v->aliveA=v->position;v->aliveB=v->positionB;}
   v->grainPhase=0.f;v->grainSecondHalf=false;
   v->anchorA[0]=v->position;v->anchorA[1]=v->position+.09f*v->sample->rate;
   v->anchorB[0]=v->positionB;v->anchorB[1]=v->positionB+.09f*v->sampleB->rate;
   v->level=0;v->stage=1;v->lp=0;v->bp=0;
  }
  v->keyDown=true;
 }
 void noteOff(int note){if(note<0||note>127)return;held[(size_t)note]=false;
  if(sustainDown){sustained[(size_t)note]=true;return;}
  releaseKey(note);
 }
 void render(float* left,float* right,int count){
  if(!left||!right||count<=0)return;
  const float dt=1.f/(float)sr;
  float amp=clampf(p.volume,0.f,1.5f);
  // Block-constant coefficients: avoid exp/pow per voice per sample.
  const float accentSlew=1.f-std::exp(-dt/.006f);
  const float gateSlew=1.f-std::exp(-dt/.005f);
  // Fracture no longer uses a stepped amplitude gate.
  const float pitchSlew=1.f-std::exp(-dt/std::max(.002f,p.glide));
  const float lowCoeff=1.f-std::exp(-2.f*pi*125.f*dt);
  const float midCoeff=1.f-std::exp(-2.f*pi*625.f*dt);
  const float filterReleaseDecay=std::exp(-dt/std::max(.01f,p.filterEnvRelease));
  const float motionReleaseDecay=std::exp(-dt/std::max(.01f,p.motionEnvRelease));
   const float modSlew=1.f-std::exp(-dt/.012f);
   const float controlSlew=1.f-std::exp(-dt/.015f);
  for(int i=0;i<count;++i){
   std::array<float,11> destinations{};
   // Per-target accumulation allows true simultaneous four-LFO routing.
   // A small modulation envelope scales depth after attack and before release.
   for(size_t m=0;m<modulators.size();++m){
    auto& osc=modulators[m];const auto& cfg=p.lfos[m];
    float hz=clampf(cfg.rate,.0002f,1.5f);
    if(cfg.sync>0){constexpr float beatLengths[]={4.f,2.f,1.f,.5f,.25f,8.f,16.f,32.f,64.f,128.f};
      hz=(tempo/60.f)/beatLengths[(size_t)std::clamp(cfg.sync-1,0,9)];}
    osc.phase+=hz*dt;
    if(osc.phase>=1.f){osc.phase-=std::floor(osc.phase);
      osc.seed=1664525u*osc.seed+1013904223u;
      osc.random=((osc.seed>>8)*(1.f/8388608.f)-1.f);}
    float shape=0.f;
    switch(cfg.wave){
      case 1: shape=1.f-4.f*std::fabs(osc.phase-.5f);break;
      case 2: shape=2.f*osc.phase-1.f;break;
      case 3: shape=osc.phase<.5f?1.f:-1.f;break;
      case 4: shape=osc.random;break;
      case 5: shape=std::sin(2.f*pi*osc.phase)*std::sin(2.f*pi*osc.phase*3.f);break;
      default:shape=std::sin(2.f*pi*osc.phase);
    }
    // Deliberate fine response for subtle organic movement.
    float depth=clampf(cfg.depth,0,1); depth=depth*depth;
    int dest=std::clamp(cfg.target,0,10);
    osc.smooth+=(shape-osc.smooth)*modSlew;
     destinations[(size_t)dest]+=osc.smooth*depth;
   }
   // MIDI CC1 and the painted mod fader share a musical modulation macro.
   const float modAmount=clampf(modwheel+p.performanceMod,0.f,1.f);
   destinations[6]+=.32f*modAmount;
   destinations[3]+=.14f*modAmount;
   destinations[10]+=.16f*modAmount;
   // The grid is rhythmic ONLY. No MIDI-note transposition or chirped melody.
   // Six contrasting gate/accent contours preserve an odd forest-psy feel.
   static constexpr float grooves[6][16] = {
    {1.f,.28f,.58f,.18f,.84f,.22f,.72f,.12f, .96f,.22f,.54f,.33f,.80f,.14f,.68f,.40f},
    {1.f,.15f,.27f,.80f,.22f,.16f,.89f,.22f, .90f,.14f,.30f,.86f,.15f,.18f,.94f,.32f},
    {.95f,.26f,.78f,.13f,.48f,.91f,.20f,.76f, .93f,.12f,.74f,.32f,.85f,.51f,.20f,.62f},
    {1.f,.11f,.84f,.18f,.14f,.91f,.57f,.13f, .87f,.67f,.11f,.89f,.21f,.13f,.73f,.41f},
    {.93f,.68f,.48f,.60f,.85f,.31f,.55f,.33f, .90f,.47f,.42f,.65f,.81f,.28f,.48f,.62f},
    {.95f,.10f,.14f,.12f,.76f,.11f,.12f,.20f, .90f,.09f,.13f,.18f,.70f,.14f,.15f,.28f}
   };
   static constexpr float divs[4]={2.f,4.f,8.f,3.f}; // eighth, 16th, 32nd, triplet
   const int grooveIndex=std::clamp(p.motif-1,0,5);
   const float beatsPerStep=divs[(size_t)std::clamp(p.motifDivision,0,3)];
   const double beatClock=motifBeats*(double)beatsPerStep;
   const int rawStep=(int)std::floor(beatClock);
   const int stepNum=((rawStep%16)+16)%16;
   float stepPhase=(float)(beatClock-std::floor(beatClock));
   // Swing shifts alternating steps WITHOUT changing pitch or grid rate.
   float swing=clampf(p.motifGlide,0.f,1.f)*.27f;
   stepPhase=clampf(stepPhase+(stepNum%2==1?swing:-swing),0.f,.999f);
   const float gateEnabled=clampf(p.gateSteps[(size_t)stepNum],0.f,1.f);
   const float rawAccent=p.motif==0?1.f:grooves[(size_t)grooveIndex][(size_t)stepNum]*gateEnabled;
   smoothedAccent+=(rawAccent-smoothedAccent)*accentSlew;
   const float accent=smoothedAccent;
   float grooveDensity=clampf(p.motifPitch,0.f,1.f);
   const float gateShape=(1.f-std::exp(-stepPhase*38.f)) *
     (1.f-clampf((stepPhase-(.58f-.18f*grooveDensity))/.42f,0.f,1.f));
   const float pulse=clampf(p.motifGate,0.f,1.f);
   const float gateStrength=(.26f+.74f*accent)*(.84f+.16f*grooveDensity);
   const float edgeFade=clampf(stepPhase*32.f,0.f,1.f)*clampf((1.f-stepPhase)*32.f,0.f,1.f);
   const float gateTarget=p.motif==0?1.f:
     (1.f-pulse*(.94f-gateShape*gateStrength))
     *(1.f-.99f*pulse*(1.f-gateEnabled)*edgeFade);
   smoothedGate+=(gateTarget-smoothedGate)*gateSlew;
   const float phraseGate=smoothedGate;
   float mixL=0,mixR=0;
   for(auto& v:voices){
    if(!v.stage||!v.sample||!v.sample->loaded())continue;
    const auto& s=*v.sample;
    const auto& sb=*v.sampleB;
    const float coeff=pitchSlew;
    v.pitch+=(v.target-v.pitch)*coeff;
    // MIDI input sets the pitch. No scripted pitch gestures at all.
    const float gesture=0.f;
    float semitones=v.pitch-(float)s.root+(bend+p.bend)*p.bendRange+gesture;
    semitones+=clampf(destinations[2],-1.f,1.f)*2.f;
    float micro=clampf(p.organic,0.f,1.f)*.030f*std::sin(v.motionPhase);
    float step=(float)s.rate*dt*std::exp2((semitones+micro)/12.f);
    step=clampf(step,.01f,20.f);
    const float semitonesB=v.pitch-(float)sb.root+(bend+p.bend)*p.bendRange+gesture
      +clampf(destinations[2],-1.f,1.f)*2.f;
    float stepB=clampf((float)sb.rate*dt*std::exp2((semitonesB+micro)/12.f),.01f,20.f);
    if(p.reverse>.5f){step=-step;stepB=-stepB;}
    // Two independent crossfaded, pitch-tracked source transports. No buffers
    // are allocated in the audio thread. Morph is equal-power and optionally
    // meanders at a second, deliberately slow modulation rate.
    const bool droneLoop=!v.oneShot && p.reverse<=.5f;
    auto readAt=[&](const Sample& source,float x)->float{
     const float n=(float)source.data.size();
     if(droneLoop){
      const float begin=n*.17f,end=n*.88f;
      const float overlap=std::min((float)source.rate*.19f,(end-begin)*.20f);
      const float cycle=end-begin-overlap;
      if(x>=end)x=begin+overlap+std::fmod(x-end,cycle);
      if(x<0.f)x=begin+overlap+std::fmod(std::fabs(x),cycle);
      if(x>end-overlap && x<end){
       const float u=clampf((x-(end-overlap))/overlap,0.f,1.f);
       const float a=u*u*(3.f-2.f*u);
       const float playbackRatio=(&source==&s)?std::fabs(step):std::fabs(stepB);
        const float near=source.bandlimited(x,playbackRatio);
       const float far=source.bandlimited(begin+x-(end-overlap),playbackRatio);
       return (1.f-a)*near+a*far;
      }
     }
     return source.bandlimited(x,(&source==&s)?std::fabs(step):std::fabs(stepB));
    };
    auto advance=[&](const Sample& source,float& pos,float delta){
     pos+=delta;
     if(droneLoop){
      float begin=(float)source.data.size()*.17f;
      float end=(float)source.data.size()*.88f;
      float overlap=std::min((float)source.rate*.19f,(end-begin)*.20f);
      if(pos>=end)pos=begin+overlap+std::fmod(pos-end,end-begin-overlap);
     }else if(p.reverse>.5f){
      if(pos<=1.f)pos=(float)source.data.size()-4.f;
     }else if(pos>=(float)source.data.size()-4.f){
      v.stage=4;v.keyDown=false;pos=(float)source.data.size()-4.f;
     }
    };
    const float movement=clampf(p.movement,0.f,1.f);
    const float organic=clampf(p.organic,0.f,1.f);
    // Three interacting slow biome cycles. Complexity varies their relative
    // phase; life modulates tone, source balance, layering and breath.
    const float biomeRate=clampf(p.biomeRate,.002f,.35f);
    const float complexity=clampf(p.biomeChaos,0.f,1.f);
    const float life=clampf(p.biomeDepth,0.f,1.f);
    const float bioPhase=2.f*pi*biomeRate*globalTime+v.motionPhase*.17f;
    const float bio1=std::sin(bioPhase+complexity*1.2f*std::sin(bioPhase*.38f+v.motionPhase));
    const float bio2=std::sin(bioPhase*(1.75f+1.6f*complexity)+v.motionPhase*1.11f);
    const float bio3=std::sin(bioPhase*(3.6f+2.3f*complexity)+2.f*std::sin(bioPhase*.62f+v.motionPhase));
    const float wr=clampf(p.xenoWrithe,0.f,1.f);
    const float biome=life*((.55f-.12f*wr)*bio1+(.27f+.04f*wr)*bio2+(.18f+.08f*wr)*bio3);
     const float coupling=clampf(p.coupledMotion,0.f,1.f);
     const float linked=coupling*(.82f*biome+.18f*(accent-.50f));
     const float lowWeight=clampf((57.f-v.pitch)/31.f,0.f,1.f);
     const float lowLife=lowWeight*clampf(p.lowRegisterLife,0.f,1.f)*(v.oneShot?.20f:1.f);
     // Self-coupled cellular oscillator: slowly evolving but sample-rate continuous.
     const float colony=clampf(p.colony,0.f,1.f);
     const float cellularRate=.071f+colony*.48f;
     v.cellPhase=wrapPhase(v.cellPhase+2.f*pi*cellularRate*dt*(1.f+.24f*bio2));
     const float cellular=std::sin(v.cellPhase+2.3f*colony*std::sin(v.cellPhase*.41f+bio3))
        *(.63f+.37f*bio1);
     const float osc=std::sin(2.f*pi*p.morphRate*globalTime+v.motionPhase*.37f);
    const float initialMorph=clampf(p.morph,0.f,1.f);
    // Bank selection should expose the actual bank, without permanently
    // contaminating source A with the bank B modulation oscillators.
    const float morphWindow=std::sin(pi*initialMorph);
    const float requestedMorph=clampf(initialMorph+morphWindow*(
      .38f*p.morphMotion*(osc+.15f*std::sin(2.f*pi*p.morphRate*.39f*globalTime))
      +.29f*destinations[6]+.075f*linked+.04f*colony*cellular
      +.085f*p.biomeDepth*std::sin(2.f*pi*p.biomeRate*globalTime+v.motionPhase*.13f)),0.f,1.f);
    v.smoothMorph+=(requestedMorph-v.smoothMorph)*controlSlew;
    const float morph=v.smoothMorph;
    const float mixA=std::cos(morph*pi*.5f),mixB=std::sin(morph*pi*.5f);
    const float baseA=readAt(s,v.position),baseB=readAt(sb,v.positionB);
    float sample=mixA*baseA+mixB*baseB;
     if(lowLife>.0001f && droneLoop){
       // Recorded-speed motion survives pitch transposition, without any hiss.
       const float live=mixA*readAt(s,v.aliveA)+mixB*readAt(sb,v.aliveB);
       const float blend=.21f*lowLife;
       sample=sample*(1.f-blend)+live*blend;
       advance(s,v.aliveA,(float)s.rate*dt*(1.f+.045f*linked));
       advance(sb,v.aliveB,(float)sb.rate*dt*(1.f-.035f*linked));
     }
    // Two source organisms interact instead of merely fading between tracks.
    // Pure, correlated cross-excitation: no stochastic hiss/noise generator.
    const float cross=clampf(p.xenoCross,0.f,1.f);
    const float interaction=cross>.0001f?std::tanh(baseA*baseB*(3.0f+1.7f*cross)):0.f;
    sample+=cross*.31f*interaction;
     // Source A and B hybridization, without a parallel stochastic texture.
     const float sym=clampf(p.symbiosis,0.f,1.f);
     const float hybrid=sym>.0001f?std::tanh(2.4f*baseA*baseB+.7f*(baseA-baseB)*cellular):0.f;
     sample+=.34f*sym*hybrid;
    // Shared biome cycles above feed source morph, the moving read-head, FM and filters.
    // Source mixing is also modulated by Life, not just an inaudible detune.
    sample=sample*(1.f+.12f*biome);
    v.motionPhase+=dt*(.17f+movement*1.70f);
    if(v.motionPhase>2.f*pi)v.motionPhase-=2.f*pi;
    const float drift=std::sin(v.motionPhase)+.34f*std::sin(v.motionPhase*.53f+1.2f);
    float layer=mixA*readAt(s,v.position+v.layerOffset*(1.f+.10f*movement*drift+.25f*biome))
               +mixB*readAt(sb,v.positionB+v.layerOffset*(float)sb.rate/(float)s.rate*(1.f+.10f*movement*drift+.25f*biome));
    // VOIDCORE CLEAN: real, continuous sample transports only.
    // All random grain-anchor resets and digital tearing have been removed.
    const int mode=std::clamp(p.textureMode,0,5);
    if(mode==1||mode==5){
      const float t=clampf(p.texture,0.f,1.f);
      const float phase=2.f*pi*(.047f+.14f*t)*globalTime+v.motionPhase*.12f;
      const float transport=1.f+.09f*std::sin(phase);
      const float secondary=mixA*readAt(s,v.position+v.layerOffset*transport)
        +mixB*readAt(sb,v.positionB+v.layerOffset*(float)sb.rate/(float)s.rate*transport);
      const float target=.23f*t*(.6f+.4f*std::sin(phase)*std::sin(phase));
      v.cleanBlend+=(target-v.cleanBlend)*(1.f-std::exp(-dt/.02f));
      sample=sample*(1.f-v.cleanBlend)+secondary*v.cleanBlend;
    }else v.cleanBlend+=(0.f-v.cleanBlend)*(1.f-std::exp(-dt/.02f));
    advance(s,v.position,step);
    advance(sb,v.positionB,stepB);
    float time=.02f;
    if(v.stage==1){time=std::max(.001f,p.attack);v.level+=dt/time;
     if(v.level>=1){v.level=1;v.stage=2;}}
    else if(v.stage==2){time=std::max(.01f,p.decay);
     v.level+=(p.sustain-v.level)*std::min(1.f,dt*5.f/time);
     if(std::fabs(p.sustain-v.level)<.002f)v.stage=3;
    }
    else if(v.stage==3)v.level+=(p.sustain-v.level)*.001f;
    else if(v.stage==4){v.level-=dt*std::max(.05f,v.releaseLevel)/std::max(.01f,p.release);
      if(v.level<=.0001f){v.stage=0;v.level=0;continue;}}
    // Two envelopes, one for the filter and one for LFO/motion intensity.
    const bool releasing=v.stage==4;
    const float fAttack=std::max(.003f,p.filterEnvAttack);
    if(!releasing){
      v.filterAge+=dt;
      if(v.filterAge<fAttack)v.filterEnv=v.filterAge/fAttack;
      else v.filterEnv=clampf(p.filterEnvSustain,0.f,1.f)
        +(1.f-clampf(p.filterEnvSustain,0.f,1.f))*
          std::exp(-(v.filterAge-fAttack)/std::max(.01f,p.filterEnvDecay));
      v.motionEnv=clampf(v.motionEnv+dt/std::max(.003f,p.motionEnvAttack),0.f,1.f);
    }else{
      v.filterEnv*=filterReleaseDecay;
      v.motionEnv*=motionReleaseDecay;
    }
    const float envAmt=v.filterEnv*clampf(p.filterEnvDepth,-1.f,1.f);
    const float moveDepth=.15f+v.motionEnv*clampf(p.motionEnvDepth,0.f,1.f);
    const float aggression=clampf(p.macroAggression,0.f,1.f);
    const float atmosphere=clampf(p.macroAtmosphere,0.f,1.f);
    // Two-operator inharmonic FM: no samples or wavetables required.
    const float carrier=std::min(7200.f,fundamentalHz(v.pitch+gesture+(bend+p.bend)*p.bendRange+clampf(destinations[2],-1.f,1.f)*2.f));
    const float fmFreq=std::min((float)sr*.11f,carrier*(p.fmRatio+.19f*p.fmInharmonic));
    v.modPhase=wrapPhase(v.modPhase+2.f*pi*fmFreq*dt);
    const float mod=std::sin(v.modPhase+clampf(p.fmFeedback,0.f,.95f)*v.prevMod*2.5f);
    v.prevMod=mod;
    v.fmPhase=wrapPhase(v.fmPhase+2.f*pi*carrier*dt);
    const float fmAmount=clampf(p.fmIndex+.32f*destinations[5]+aggression*.8f
       +.28f*wr*std::fabs(bio2)+.27f*linked+.19f*colony*cellular,0.f,3.0f);
    const float fmA=std::sin(v.fmPhase+fmAmount*mod);
    const float fmB=std::sin(v.fmPhase+(fmAmount*.46f)*std::sin(v.modPhase+bio2*.36f));
    const float fmSignal=(.68f*fmA+.32f*fmB)*(.12f+.88f*v.motionEnv);
    const float requestedFmMix=clampf(p.fmMix+aggression*.025f,0.f,.80f);
    v.smoothFmMix+=(requestedFmMix-v.smoothFmMix)*controlSlew;
    const float fmMix=v.smoothFmMix;
    sample=sample*(1.f-fmMix*.62f)+fmSignal*fmMix*.53f;
     // Tuned 3rd/5th/7th partials restore low-register articulation; no noise.
     if(lowLife>.0001f){
       const float creature=
         .55f*std::sin(3.f*v.fmPhase+.63f*mod+bio1*.25f)
        +.29f*std::sin(5.f*v.fmPhase+.45f*std::sin(v.modPhase*.74f)+bio2*.4f)
        +.16f*std::sin(7.f*v.fmPhase+.22f*bio3);
       const float breathe=.65f+.35f*(.5f+.5f*bio2);
       sample+=creature*lowLife*(.075f+.03f*coupling)*breathe
                   *(.40f+.60f*v.motionEnv);
     }
    // Irregular creature calls stay tied to the held fundamental (not MIDI arps).
    // Raised smooth envelope avoids boing onset and broadband noise.
    const float creatureAmt=clampf(p.creatures,0.f,1.f);
    if(creatureAmt>.0001f){
      const float rate=(.19f+creatureAmt*2.85f)*(1.f+.31f*bio2);
      v.organismPhase+=rate*dt;
      if(v.organismPhase>=1.f)v.organismPhase-=std::floor(v.organismPhase);
      const float growth=std::sin(pi*v.organismPhase);
      const float window=growth*growth*(.35f+.65f*growth);
      const float ratio=2.6f+1.1f*(.5f+.5f*bio1)+1.4f*(.5f+.5f*cellular);
      const float freq=std::min((float)sr*.18f,carrier*ratio);
      v.organismTonePhase=wrapPhase(v.organismTonePhase+2.f*pi*freq*dt);
      const float throat=std::sin(v.organismTonePhase+.84f*std::sin(v.modPhase*.57f+bio3));
      sample+=.10f*creatureAmt*window*throat*(.35f+.65f*v.motionEnv);
    }
    // The ecology has three different foreground characters, not one ambient bed.
    // 1) PREDATOR: a slow, unstable throat with formants derived from the key.
    // 2) SPORE CODE: independently drifting micro-chirps that speak in phrases.
    // 3) BURROW: downward breathing interference made from BOTH sample layers.
    // All oscillators run continuously. Raised-cosine envelopes eliminate clicks.
    const float predator=clampf(p.predator,0.f,1.f);
    const float insect=clampf(p.insectCode,0.f,1.f);
    const float burrow=clampf(p.burrow,0.f,1.f);
    if(predator>.0001f){
      float slow=.17f+.38f*predator+.13f*std::fabs(cellular);
      v.predatorPhase=wrapPhase(v.predatorPhase+2.f*pi*std::min((float)sr*.22f,
         carrier*(1.18f+.57f*bio1+.11f*slow))*dt);
      const float throat=std::sin(v.predatorPhase
        + (1.8f+3.4f*predator)*std::sin(v.modPhase*.41f+2.3f*bio2));
      const float breathing=.65f+.35f*std::sin(v.cellPhase*.67f+bio3);
      const float window=breathing*breathing;
      sample+=throat*(.095f+.08f*predator)*predator*window*(.3f+.7f*v.motionEnv);
    }
    if(insect>.0001f){
      // Insect calls are scattered in time, never quantized into musical notes.
      const float speed=.23f+2.05f*insect+.11f*std::sin(v.cellPhase);
      v.signalPhase+=dt*speed;
      if(v.signalPhase>=1.f)v.signalPhase-=std::floor(v.signalPhase);
      const float event=std::sin(pi*v.signalPhase);
      const float env=event*event*event*event; // continuous at event boundary
      const float glide=1.6f+.45f*std::sin(v.cellPhase*.53f+v.motionPhase);
      v.insectPhase=wrapPhase(v.insectPhase+2.f*pi*std::min((float)sr*.24f,carrier*glide)*dt);
      const float chirp=std::sin(v.insectPhase+1.5f*insect*std::sin(v.modPhase*.73f));
      sample+=.12f*insect*env*chirp*(.48f+.52f*v.motionEnv);
    }
    if(burrow>.0001f){
      const float phase=2.f*pi*(.09f+.32f*burrow)*globalTime+v.motionPhase*.13f;
      const float sweep=.5f+.5f*std::sin(phase+1.6f*bio2);
      const float mix=std::sin((2.3f+3.2f*burrow)*(baseA*.63f+baseB*.37f)+
         2.f*burrow*std::sin(v.cellPhase*.61f));
      sample+=burrow*(.05f+.12f*burrow)*mix*(.35f+.65f*sweep);
    }
    // Spectral wavefolding bypassed entirely to avoid bitcrushing artifacts.
    // Morphing pseudo-vocal tract with two narrow, stable resonant peaks.
    // Reacts to living biome and cross-layer motion without broadband noise.
    const float xeno=clampf(p.xenoFormant,0.f,1.f);
    if(xeno>.0001f){
      const float vowelPos=clampf(.5f+.27f*bio1+.18f*bio3+.15f*wr*std::sin(v.motionPhase*.4f)+.13f*linked+.14f*colony*cellular,0.f,1.f);
      const float f1=270.f+830.f*vowelPos;
      const float f2=1150.f+1650.f*(1.f-vowelPos);
      auto vocal=[&](float hz,float& lp,float& bp,float input){
        const float g=clampf(2.f*std::sin(pi*hz*dt),.0001f,.55f);
        const float hp=input-lp-.34f*bp;
        bp=clampf(bp+g*hp,-3.f,3.f);
        lp=clampf(lp+g*bp,-3.f,3.f);
        return bp;
      };
      float vocal1=vocal(f1,v.xenoLp1,v.xenoBp1,sample);
      float vocal2=vocal(f2,v.xenoLp2,v.xenoBp2,sample);
      sample=sample*(1.f-.22f*xeno)+(.61f*vocal1+.39f*vocal2)*.44f*xeno;
    }
    // Physical-style damped resonator. Persistent delay provides real metallic
    // sustain; partials from inharmonic FM add bright scraped-metal motion.
    const float metalAmt=clampf(p.metalMix+.12f*destinations[7]+aggression*.035f,0.f,1.f);
    if(metalAmt>.0001f){
    const int delaySamples=std::clamp((int)((float)sr/(carrier*(.51f+p.metalTune*3.8f))),16,4094);
    const int tap=(v.metalPos+4096-delaySamples)%4096;
    float ringing=v.metalLine[(size_t)tap];
    float feedback=clampf(.48f+p.metalDecay*.49f,0.f,.978f);
    v.metalDamp+=(ringing-v.metalDamp)*(.02f+.65f*p.metalTone);
    const float excitation=(.15f*sample+.12f*fmSignal)*metalAmt;
    v.metalLine[(size_t)v.metalPos]=std::tanh(v.metalDamp*feedback+excitation);
    v.metalPos=(v.metalPos+1)%4096;
    sample+=metalAmt*(ringing*.87f+.12f*fmSignal);
    }
    // Stable two-stage SVFs, with serial, parallel or band-split routing.
    float baseCut=clampf(p.cutoff + .27f*destinations[3]*moveDepth +envAmt*.35f + (accent-.5f)*.11f*p.motifAccent*(p.motif>0?1.f:0.f)
                  +p.keytrack*(v.pitch-60.f)/120.f
                  +.035f*organic*std::sin(v.motionPhase*.67f)+.19f*biome+.14f*linked,.008f,1.f);
    float otherCut=clampf(p.cutoff2+.27f*destinations[4]*moveDepth+envAmt*.28f
      -.13f*biome-.09f*linked-.09f*colony*cellular,.008f,1.f);
    auto filt=[&](float input,float cutoff,float resonance,int mode,float& lp,float& bp){
      // Topology-preserving SVF; no unconditional distortion in the filter.
      const float hz=std::min((float)sr*.44f,20.f*std::pow(930.f,cutoff));
      const float g=std::tan(pi*hz*dt);
      const float k=2.f-1.72f*clampf(resonance,0.f,1.f);
      const float a1=1.f/(1.f+g*(g+k));
      const float a2=g*a1,a3=g*a2;
      const float v3=input-lp;
      const float v1=a1*bp+a2*v3;
      const float v2=lp+a2*bp+a3*v3;
      bp=clampf(2.f*v1-bp,-8.f,8.f);
      lp=clampf(2.f*v2-lp,-8.f,8.f);
      const float hp=input-k*v1-v2;
      switch(mode){case 1:return v1;case 2:return hp;case 3:return hp+v2;default:return v2;}
    };
    const float clean=sample;
    const bool split=p.filterRoute==2 && p.filter2Mode>0;
    float stage1=filt(clean,baseCut,p.resonance,split?0:p.filterMode,v.lp,v.bp);
    float filtered=stage1;
    if(p.filter2Mode>0){
      float secondInput=(p.filterRoute==0)?stage1:clean;
      float stage2=filt(secondInput,otherCut,p.resonance2,split?2:p.filter2Mode-1,v.lp2,v.bp2);
      filtered=stage1*(1.f-p.filterBlend)+stage2*p.filterBlend;
    }
    float drive=clampf(p.drive+aggression*.22f,0.f,1.f);
    float dry=filtered*.84f+clean*.16f;
    dry+=p.spectralTilt*(clean-v.lp)*.6f;
    // Clarity macro preserves uncompressed detail while still allowing edge.
    const float cleanFraction=clampf(p.clarity,0.f,1.f);
    const float amount=clampf((drive*.72f+p.filterDrive*.49f)*(1.f-.73f*cleanFraction),0.f,.90f);
    const float saturation=std::tanh(dry*(1.f+amount*2.3f))/(1.f+.36f*amount);
    dry=dry*(1.f-amount*.40f)+saturation*(amount*.40f);
    const float edge=std::tanh((clean-v.lp)*1.2f)*amount*.07f;
    dry+=edge;
    // NOISE-FREE XENOBIOME: harmonic ghost events follow the rhythmic gate
    // without injecting white/pink noise. The event is ring-modulated from
    // the actual sample, with an exponential window and warm band-limited FM.
    if(p.motif>0 && p.motifFX>.0001f && accent>.32f){
      const float ghostEnv=(1.f-std::exp(-stepPhase*48.f))*std::exp(-stepPhase*14.f);
      const float ghost=std::sin(v.fmPhase*1.13f+.52f*std::sin(v.modPhase));
      dry+=.095f*clampf(p.motifFX,0.f,1.f)*accent*ghostEnv*ghost;
    }

    // Gentle air shelf made from the actual 48 kHz samples, not hard noise.
    v.airLP+=(sample-v.airLP)*.12f;
    dry+=(sample-v.airLP)*.10f*clampf(p.hiFiAir,0.f,1.f);
    float layerGain=clampf(p.layers+atmosphere*.18f,0.f,1.f)*(.29f+.32f*organic);
    layerGain*=clampf(1.f+.35f*biome+.18f*linked,.40f,1.5f);
    float stereoLayer=(layer*(1.f-.65f*drive)+std::tanh(layer*(1.f+drive*2.f))*.65f*drive)*.65f*layerGain;
    // Organic fundamental/subharmonic, never overly prominent for percussive banks.
    float fundamental=std::clamp(440.f*std::exp2((v.pitch-69.f)/12.f),20.f,1500.f);
    v.bodyPhase+=dt*fundamental*pi; // sub oscillator at half fundamental
    if(v.bodyPhase>2*pi)v.bodyPhase-=2*pi;
    float sub=(.21f*std::sin(v.bodyPhase)+.065f*std::sin(2.f*v.bodyPhase)) * clampf(p.sub,0.f,1.f);
    // No persistent stochastic breath layer; it previously followed the gate
    // on every bank, producing the unwanted moving white/pink-noise blanket.
    // All micro-motion comes from the sample's resonant material and biome.
    const float air=0.f;
    sub*=v.oneShot?.33f:1.f;
    // Start voices as a slow swell; rhythmic bleeps live inside the sample, not at note-on.
     const float voicedRise=clampf(v.filterAge/.18f,0.f,1.f);
     const float attackShape=voicedRise*voicedRise*(3.f-2.f*voicedRise);
     float gain=v.level*v.vel*.46f*(1.f+.15f*lowLife)*(.70f+.30f*attackShape);
    // Tempo-aware forest/psytrance polymetric breathing, available in every
    // texture algorithm. Pattern steps are softly interpolated, never clicked.
    constexpr float seq[16]={1.f,.64f,.92f,.48f,.98f,.56f,.72f,.39f,
                             1.f,.50f,.80f,.60f,.94f,.45f,.84f,.70f};
    float beat=globalTime*tempo/60.f*clampf(p.tranceRate,.25f,8.f);
    float slot=std::fmod(beat*4.f+v.note*.125f,16.f);
    int ix=std::clamp((int)slot,0,15);
    float frac=slot-ix;
    float transition=clampf((frac-.85f)/.15f,0.f,1.f);
    transition=transition*transition*(3.f-2.f*transition);
    float trance=seq[ix]*(1.f-transition)+seq[(ix+1)%16]*transition;
    gain*=1.f-clampf(p.trancePulse,0.f,1.f)*(.95f-trance);
    gain*=phraseGate;
    gain*=clampf(1.f+.18f*biome,.55f,1.35f);
    if(p.motif>0)gain*=1.f+.33f*clampf(p.motifAccent,0.f,1.f)*(accent-.45f);
    if(mode==3){
     float clockPhase=std::fmod(globalTime*tempo/60.f*clampf(p.pulseRate,.25f,16.f)
             +(float)(v.note%5)*.04f,1.f);
     float gate=.16f+.84f*(.5f+.5f*std::cos(2*pi*clockPhase));
     gain*=1.f-clampf(p.texture,0.f,1.f)*(1.f-gate);
    }
    if(mode==4){
     // Liquid deformation with no square gate or high-rate restart.
     const float angle=2.f*pi*globalTime*clampf(p.pulseRate,.25f,16.f)*tempo/60.f;
     const float breathing=.5f+.5f*std::sin(angle+.31f*bio2);
     gain*=1.f-.23f*clampf(p.texture,0.f,1.f)*(1.f-breathing);
    }
    gain*=clampf(1.f+destinations[0]*.45f,0.f,1.5f);
    float orbit=(mode==2? p.texture*.75f*std::sin(v.motionPhase*1.35f+globalTime*.37f):0.f);
    float pan=clampf(p.pan +destinations[1]*moveDepth+orbit+.21f*biome+.12f*linked,-1.f,1.f);
    // Equal-power output pan and subtle sample-bank stereo spread
    float spread=clampf(p.stereo,0,1.5f)*(((v.note%7)-3)/12.f);
    float a=clampf(pan+spread,-1,1);
    const float outL=(dry*(1-layerGain*.4f)+stereoLayer*.44f+sub+air)*gain*std::sqrt(.5f*(1.f-a));
    const float outR=(dry*(1-layerGain*.4f)+stereoLayer*.98f+sub-air*.36f)*gain*std::sqrt(.5f*(1.f+a));
    v.lastL=outL;v.lastR=outR;
    if(v.stealFade>0.f){
      mixL+=outL+v.stealL*v.stealFade;mixR+=outR+v.stealR*v.stealFade;
      v.stealFade=std::max(0.f,v.stealFade-dt/.008f);
    }else{mixL+=outL;mixR+=outR;}
   }
   globalTime+=dt;
   motifBeats+=(double)tempo/60.0/sr;
   if(motifBeats>65536.f)motifBeats-=65536.f;
   if(globalTime>100000.f)globalTime=0;
   float wetL=0,wetR=0;
   {
     auto fx=p.fx;
     fx.delayWarp=clampf(fx.delayWarp+.27f*destinations[9],0.f,1.f);
     fx.reverbMod=clampf(fx.reverbMod+.25f*destinations[10]+.2f*p.macroAtmosphere,0.f,1.f);
     fx.reverbMix=clampf(fx.reverbMix+.09f*p.macroAtmosphere,0.f,.98f);
     fx.psySpiral=p.psySpiral;fx.psySpiralRate=p.psySpiralRate;
     fx.wetLowCut=p.wetLowCut;
     spaceFX.setControls(fx); // assignment only; no allocation.
   }
   // Focus network: adjustable low-mid cleanup in DRY audio only.
   // Slow and fast low-passes isolate the 120–600 Hz buildup; the high pass
   // on the wet path lives INSIDE PsySpace and does not remove bass from dry.
   mixLowL+=(mixL-mixLowL)*lowCoeff;mixLowR+=(mixR-mixLowR)*lowCoeff;
   mixMidL+=(mixL-mixMidL)*midCoeff;mixMidR+=(mixR-mixMidR)*midCoeff;
   const float cut=.69f*clampf(p.mudCut,0.f,1.f);
   mixL-=cut*(mixMidL-mixLowL);
   mixR-=cut*(mixMidR-mixLowR);
   // Narrow low frequencies in a controllable way while keeping weird stereo
   // animation in the mids and highs; energy-normalized mid/side operation.
   const float narrow=clampf(p.focus,0.f,1.f)*.45f;
   const float side=(mixL-mixR)*.5f;
   const float mid=(mixL+mixR)*.5f;
   mixL=mid+(1.f-narrow)*side;
   mixR=mid-(1.f-narrow)*side;
   spaceFX.tick(mixL,mixR,wetL,wetR);
   // A softly limited master bus retains transient punch and space width.
   // More headroom and less blanket drive than Earthforge v6 master limiter.
   // Soft limiting is retained to make accidental dense chords manageable.
   auto transparentLimit=[](float x){
     const float magnitude=std::fabs(x);
     if(magnitude<=.90f)return x;
     const float over=magnitude-.90f;
     return std::copysign(.90f+.10f*over/(over+.10f),x);
    };
    left[i]+=transparentLimit((mixL+wetL)*amp);
    right[i]+=transparentLimit((mixR+wetR)*amp);
  }
 }
private:
 struct Voice {
  const Sample* sample=nullptr;
  const Sample* sampleB=nullptr;
  int note=-1,bank=0,stage=0;
  float vel=0,position=0,positionB=0,level=0,releaseLevel=1,pitch=0,target=0,lp=0,bp=0;
  float motionPhase=0,layerOffset=0,bodyPhase=0,breath=0,aliveA=0,aliveB=0;
   float cellPhase=0,organismPhase=0,organismTonePhase=0,foldLP=0,cleanBlend=0;
   float predatorPhase=0,insectPhase=0,signalPhase=0;
  float lp2=0,bp2=0,filterEnv=0,filterAge=0,motionEnv=0,airLP=0;
  float xenoLp1=0,xenoBp1=0,xenoLp2=0,xenoBp2=0;
  float fmPhase=0,modPhase=0,prevMod=0,metalDamp=0,motifPitch=0,fxPhase=0,ghostLP=0;int fxStep=-1;
  std::array<float,4096> metalLine{};int metalPos=0;
  float grainPhase=0.f,fractureGain=1.f,stealL=0,stealR=0,stealFade=0,lastL=0,lastR=0;
  float smoothMorph=0.f,smoothFmMix=0.f,smoothFold=0.f;
  bool grainSecondHalf=false;
  std::array<float,2> anchorA{},anchorB{};uint32_t noiseState=1;
  bool keyDown=false,oneShot=false;uint64_t age=0;
 };
 void releaseKey(int n){
  if(p.mode==2){int latest=-1;for(int i=0;i<128;++i)if(held[(size_t)i])latest=i;
   if(latest>=0){voices[0].target=(float)latest;voices[0].note=latest;return;}}
  for(auto& v:voices)if(v.stage&&v.note==n){v.releaseLevel=v.level;v.stage=4;v.keyDown=false;}
 }
 static float wave(float x,int k){
  switch(k){case 1:return 1.f-4.f*std::fabs(x-.5f);
   case 2:return x<.5f?1.f:-1.f;
   case 3:return 2.f*x-1.f;
   case 4:return 2.f*(std::exp(-5.f*x)-.5f*(1.f+std::exp(-5.f)))/(1.f-std::exp(-5.f));
   default:return std::sin(2*pi*x);}
 }
 static float fundamentalHz(float midi){return clampf(440.f*std::exp2((midi-69.f)/12.f),20.f,8000.f);}
 static float wrapPhase(float phase){return phase>=2.f*pi?std::fmod(phase,2.f*pi):phase;}
 struct ModState {float phase=0.f,random=0.f,smooth=0.f;uint32_t seed=1234567u;};
 double sr=44100.;Params p;
 std::array<ModState,4> modulators{};
 std::array<std::array<Sample,4>,51> samples{};
 std::array<Voice,voiceCount> voices{};
 std::array<bool,128> held{},sustained{};
 uint64_t voiceCounter=0;bool sustainDown=false;
 float bend=0,modwheel=0,lfoPhase=0,globalTime=0,tempo=145;
 double motifBeats=0.;bool hostBeatSynced=false;
 float mixLowL=0,mixLowR=0,mixMidL=0,mixMidR=0;
 float smoothedGate=1.f,smoothedAccent=1.f;
 PsySpace spaceFX;std::string bankFolder;
};
}
