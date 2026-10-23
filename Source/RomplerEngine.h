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
 float rate=.001f, depth=0.f;
 int wave=0, target=0, sync=0;
};
struct Params {
 int bank=0, mode=0, velocityCurve=1, resampling=1, lfoWave=0, lfoTarget=0, filterMode=0;
 float attack=.028f,decay=2.8f,sustain=.93f,release=2.2f;
 float cutoff=.95f, resonance=.14f,glide=.07f,space=.34f,volume=.76f,pan=0.f;
 float lfoRate=.009f,lfoDepth=0.f,bendRange=2.f,bend=0.f;
 float sampleStart=0.f,reverse=0.f,drive=.16f,stereo=1.13f;
 float organic=.73f, layers=.59f, movement=.52f, sub=.19f;
 // V5 dual-bank soundscape engine (new parameters appended for V4 state compatibility).
 int bankB=7, textureMode=0;
 float morph=0.f, morphMotion=0.f, morphRate=.065f, texture=.42f;
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
 float macroAtmosphere=.64f,macroAggression=.13f;
 // V7: musical hypnotic motion and open, less saturated high-fidelity output.
 float trancePulse=.17f,tranceRate=2.f,psySpiral=.24f,psySpiralRate=.16f;
 float hiFiAir=.32f,clarity=.91f;
 // V9: tempo-locked non-melodic gates, accents and controlled space.
 int motif=1, motifDivision=1; // Off or six non-pitched rhythmic gates
 float motifPitch=.36f, motifGate=.42f, motifAccent=.65f, motifGlide=.18f,motifFX=0.f;
 float focus=.68f, mudCut=.52f, wetLowCut=.64f;
 float biomeDepth=.72f,biomeRate=.034f,biomeChaos=.60f;
 // v10: physically inspired noise-free cross excitation and resonant vocal tract.
 float xenoFormant=.31f, xenoCross=.12f, xenoWrithe=.54f;
 // V10.1: tuned life below MIDI note 57 and correlated slow organic motion.
 float lowRegisterLife=.72f, coupledMotion=.53f;
 int maxVoices=12;
 // V10.6: no noise generator, transient strike, or preprogrammed MIDI jumps.
 float colony=.33f, creatures=.39f, psyFold=.22f, symbiosis=.26f;
 // v10.9 — three contrasting dark-forest organisms, always sample-linked.
 float predator=.63f, insectCode=.57f, burrow=.46f;
 float performanceMod=0.f;
 // Independent exciter amount and sample-only audition switch.
 float character=0.f,sourceSolo=0.f;
 float tuneA=0.f,tuneB=0.f,levelA=1.f,levelB=1.f;
 float toneLow=.5f,toneMid=.5f,toneHigh=.5f;
 // Final XENO destruction matrix: deliberate, bank-linked character, never
 // a permanent hiss/bitcrusher on every source.
 float drop=.55f, hits=.24f, body=.46f, grit=.30f, dark=.43f;
 float evolution=.68f; // Long-horizon sample-chapter exploration
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
 // Anti-alias reconstruction for samples played above their recorded root.
 // 17-tap bandlimited source reconstruction, with phase and cutoff LUTs.
 // Precomputation occurs once in prepare(), never while rendering voices.
 using SincTable=std::array<std::array<std::array<float,17>,65>,17>;
 static const SincTable& sincTable(){
  static const SincTable table=[](){
   SincTable built{};
   for(int bin=0;bin<17;++bin){
    const float cutoff=.020f+.330f*(float)bin/16.f;
    for(int phase=0;phase<=64;++phase){
     const float frac=(float)phase/64.f;
     float sum=0.f;
     for(int k=0;k<17;++k){
      const float distance=frac-(float)(k-8);
      const float theta=2.f*pi*cutoff*distance;
      const float sinc=std::fabs(theta)<1e-6f?1.f:std::sin(theta)/theta;
      const float window=.5f+.5f*std::cos(pi*distance/9.f);
      const float weight=2.f*cutoff*sinc*window;
      built[(size_t)bin][(size_t)phase][(size_t)k]=weight;
      sum+=weight;
     }
     if(std::fabs(sum)>1e-6f)
      for(float& w:built[(size_t)bin][(size_t)phase])w/=sum;
    }
   }
   return built;
  }();
  return table;
 }
 static void prepareSinc(){(void)sincTable();}
 float bandLimited(float pos,float increment)const {
  const float ratio=std::fabs(increment);
  if(ratio<=1.38f)return cubic(pos);
  if(!loaded()||pos<9.f||pos>(float)(data.size()-10))return cubic(pos);
  const int origin=(int)std::floor(pos);
  const float frac=pos-(float)origin;
  const float phaseFloat=frac*64.f;
  const int phase=std::clamp((int)phaseFloat,0,63);
  const float phaseMix=phaseFloat-(float)phase;
  const float cutoff=std::min(.35f,.47f/ratio);
  const float binFloat=std::clamp((cutoff-.020f)/.330f*16.f,0.f,16.f);
  const int bin=std::min(15,(int)binFloat);
  const float binMix=binFloat-(float)bin;
  const auto& table=sincTable();
  float output=0.f;
  for(int k=0;k<17;++k){
   const float a=table[(size_t)bin][(size_t)phase][(size_t)k];
   const float b=table[(size_t)bin][(size_t)phase+1][(size_t)k];
   const float c=table[(size_t)bin+1][(size_t)phase][(size_t)k];
   const float d=table[(size_t)bin+1][(size_t)phase+1][(size_t)k];
   const float low=a+(b-a)*phaseMix,high=c+(d-c)*phaseMix;
   output+=data[(size_t)(origin+k-8)]*(low+(high-low)*binMix);
  }
  return output;
 }
 float cubic(float pos)const {
  if(!loaded()||pos<1||pos>=(float)(data.size()-3))return linear(pos);
  size_t i=(size_t)pos;float t=pos-(float)i;
  float a=data[i-1],b=data[i],c=data[i+1],d=data[i+2];
  return b + .5f*t*(c-a+t*(2*a-5*b+4*c-d+t*(3*(b-c)+d-a)));
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
 Engine(){prepare(44100.);}
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
  Sample::prepareSinc();
  sr=std::max(8000.,sampleRate);for(auto& v:voices)v={};
  // Build the TPT coefficient curve off the audio thread, once per host SR.
  for(size_t j=0;j<filterGTable.size();++j){
   const float cutoff=(float)j/1024.f;
   const float hz=clampf(25.f*std::pow(760.f,cutoff),25.f,(float)sr*.42f);
   filterGTable[j]=std::tan(pi*hz/(float)sr);
  }
  spaceFX.prepare(sr);mixLowL=mixLowR=0;mixMidL=mixMidR=0;toneLoL=toneLoR=toneHiL=toneHiR=0.f;
  lfoPhase=0;for(auto& l:modulators)l={};globalTime=0;motifBeats=0;hostBeatSynced=false;held.fill(false);sustained.fill(false);sustainDown=false;smoothedGate=1.f;smoothedAccent=1.f;smoothedSoloFx=0.f;masterGain=1.f;
 }
 void setParameters(const Params& values){
  // Live bank changes crossfade held notes instead of waiting for new MIDI.
  if(values.bank!=p.bank || values.bankB!=p.bankB){
   for(auto& v:voices){
    if(!v.stage||!v.sample||!v.sampleB)continue;
    const int zone=std::clamp((int)std::lround((v.note-36)/12.f),0,3);
    auto choose=[&](int bank)->const Sample*{
     const auto& row=samples[(size_t)std::clamp(bank,0,50)];
     if(row[(size_t)zone].loaded())return &row[(size_t)zone];
     for(const auto& candidate:row)if(candidate.loaded())return &candidate;
     return nullptr;
    };
    const Sample* a=choose(values.bank);
    const Sample* b=choose(values.bankB);
    if(!a)continue;
    if(!b)b=a;
    if(a==v.sample && b==v.sampleB)continue;
    v.oldSample=v.sample;v.oldSampleB=v.sampleB;
    v.oldPosition=v.position;v.oldPositionB=v.positionB;
    const float fractionA=v.position/std::max(1.f,(float)v.sample->data.size());
    const float fractionB=v.positionB/std::max(1.f,(float)v.sampleB->data.size());
    v.sample=a;v.sampleB=b;
    v.position=clampf(fractionA*(float)a->data.size(),9.f,(float)a->data.size()-11.f);
    v.positionB=clampf(fractionB*(float)b->data.size(),9.f,(float)b->data.size()-11.f);
    v.aliveA=v.position;v.aliveB=v.positionB;
    v.bodyPosA=v.position;v.bodyPosB=v.positionB;
    v.impactProgress=1.f;v.debris={};
    v.bankFade=1.f;
    v.oneShot=values.bank>=0&&values.bank<50?banks[(size_t)values.bank].oneShot:false;
    v.layerOffset=(float)a->rate*.18f;
   }
  }
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
  v->oldSample=v->oldSampleB=nullptr;
  v->bankFade=0.f;
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
  v->bodyPhase=0;v->fmPhase=0;v->fmAltPhase=0;v->modPhase=0;v->prevMod=0;
  v->stealL=previousL;v->stealR=previousR;v->stealFade=stolen?1.f:0.f;
  v->lastL=v->lastR=0.f;v->fractureGain=1.f;
  v->smoothMorph=clampf(p.morph,0.f,1.f);
  v->smoothFmMix=clampf(p.fmMix,0.f,1.f);
  v->smoothFold=clampf(p.psyFold,0.f,1.f);
   v->smoothSolo=clampf(p.sourceSolo,0.f,1.f);
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
   v->debris={};v->debrisCountdown=.07f;
   v->bodyPosA=std::max(v->position,(float)v->sample->data.size()*.20f);
   v->bodyPosB=std::max(v->positionB,(float)v->sampleB->data.size()*.20f);
   v->bodyLP=0.f;
   v->impactPosA=v->position;v->impactPosB=v->positionB;
   v->impactProgress=1.f;v->impactCountdown=.08f;
   v->impactLength=.2f;v->impactSpeed=.5f;v->impactPitch=0.f;
   v->impactPhase=0.f;v->gritPre=v->gritPost=v->darkLP=0.f;
   v->evolveAge=v->evolvePhase=v->evolveScan=0.f;
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
  // Block-constant coefficients avoid millions of pow/exp calls per second
  // during polyphonic granular playback and keep transient scheduling stable.
  const float drop=clampf(p.drop,0.f,1.f);
  const float bodyAmount=clampf(p.body,0.f,1.f);
  const float bodySpeed=std::exp2((-5.f-19.f*drop)/12.f);
  const float bodyCoeff=1.f-std::exp(-2.f*pi*(320.f+270.f*bodyAmount)*dt);
  const float gritAmount=clampf(p.grit,0.f,1.f);
  const float gritPreCoeff=1.f-std::exp(-2.f*pi*3400.f*dt);
  const float gritPostCoeff=1.f-std::exp(-2.f*pi*5400.f*dt);
  const float darkAmount=clampf(p.dark,0.f,1.f);
  const float darkHz=12500.f*std::exp(-2.15f*darkAmount);
  const float darkCoeff=1.f-std::exp(-2.f*pi*darkHz*dt);
  float amp=clampf(p.volume,0.f,1.5f);
  // Block-constant coefficients: avoid exp/pow per voice per sample.
  const float accentSlew=1.f-std::exp(-dt/.006f);
  const float gateSlew=1.f-std::exp(-dt/.005f);
  // Fracture no longer uses a stepped amplitude gate.
  const float pitchSlew=1.f-std::exp(-dt/std::max(.002f,p.glide));
  const float lowCoeff=1.f-std::exp(-2.f*pi*125.f*dt);
  const float midCoeff=1.f-std::exp(-2.f*pi*625.f*dt);
  const float toneLoGain=std::pow(10.f,(clampf(p.toneLow,0.f,1.f)-.5f)*1.8f);
  const float toneMiGain=std::pow(10.f,(clampf(p.toneMid,0.f,1.f)-.5f)*1.8f);
  const float toneHiGain=std::pow(10.f,(clampf(p.toneHigh,0.f,1.f)-.5f)*1.8f);
  const float lowEq=1.f-std::exp(-2.f*pi*190.f*dt);
  const float highEq=1.f-std::exp(-2.f*pi*2400.f*dt);
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
    float hz=clampf(cfg.rate,.00001f,3.f);
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
    float semitones=v.pitch-(float)s.root+(bend+p.bend)*p.bendRange+gesture+p.tuneA;
    // Intentional MIDI pitch remains; evolving modulation no longer detunes samples.
    const float micro=0.f;
    float step=(float)s.rate*dt*std::exp2((semitones+micro)/12.f);
    step=clampf(step,.01f,20.f);
    const float semitonesB=v.pitch-(float)sb.root+(bend+p.bend)*p.bendRange+gesture+p.tuneB
      ;
    float stepB=clampf((float)sb.rate*dt*std::exp2((semitonesB+micro)/12.f),.01f,20.f);
    if(p.reverse>.5f){step=-step;stepB=-stepB;}
    // Two independent crossfaded, pitch-tracked source transports. No buffers
    // are allocated in the audio thread. Morph is equal-power and optionally
    // meanders at a second, deliberately slow modulation rate.
    const bool droneLoop=!v.oneShot && p.reverse<=.5f;
    // Each held voice visits different temporal chapters of the WAV.
    // Incommensurate multi-rate cycles never reset at loop wrap.
    const float evolving=clampf(p.evolution,0.f,1.f);
    v.evolveAge+=dt;
    v.evolvePhase+=dt*(.17f+.31f*evolving);
    const float exploreFade=clampf((v.evolveAge-.7f)/3.2f,0.f,1.f);
    v.evolveScan=droneLoop?evolving*exploreFade*
       (.075f*std::sin(v.evolvePhase+.013f*(float)v.note)+
        .044f*std::sin(.413f*v.evolvePhase+.29f*(float)(v.bank%10))):0.f;
    auto readAt=[&](const Sample& source,float x)->float{
     const float n=(float)source.data.size();
     if(droneLoop)x+=n*v.evolveScan; // smoothly scan recorded chapters
     if(droneLoop){
      const float begin=n*.17f,end=n*.88f;
      const float overlap=std::min((float)source.rate*.19f,(end-begin)*.20f);
      const float cycle=end-begin-overlap;
      if(x>=end)x=begin+overlap+std::fmod(x-end,cycle);
      if(x<0.f)x=begin+overlap+std::fmod(std::fabs(x),cycle);
      if(x>end-overlap && x<end){
       const float u=clampf((x-(end-overlap))/overlap,0.f,1.f);
       const float a=u*u*(3.f-2.f*u);
       const float increment=(&source==&s)?step:stepB;
       const float near=source.bandLimited(x,increment);
       const float far=source.bandLimited(begin+x-(end-overlap),increment);
       return (1.f-a)*near+a*far;
      }
     }
     return source.bandLimited(x,(&source==&s)?step:stepB);
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
    const float bioPhase=2.f*pi*biomeRate*globalTime+std::sin(v.motionPhase)*.17f;
    const float bio1=std::sin(bioPhase+complexity*1.2f*std::sin(bioPhase*.38f+v.motionPhase));
    const float bio2=std::sin(bioPhase*(1.75f+1.6f*complexity)+std::sin(v.motionPhase)*1.11f);
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
     const float cellular=std::sin(v.cellPhase+2.3f*colony*std::sin(v.cellPhase+bio3))
        *(.63f+.37f*bio1);
     const float osc=std::sin(2.f*pi*p.morphRate*globalTime+std::sin(v.motionPhase)*.37f);
    const float requestedMorph=clampf(p.morph
      +p.morphMotion*(.5f*(osc+.24f*std::sin(2.f*pi*p.morphRate*.39f*globalTime))
       +.13f*linked+.12f*colony*cellular
       +.20f*clampf(p.biomeDepth,0.f,1.f)*std::sin(2.f*pi*p.biomeRate*globalTime+std::sin(v.motionPhase)*.13f))
      +.35f*destinations[6],0.f,1.f);
    v.smoothMorph+=(requestedMorph-v.smoothMorph)*controlSlew;
    const float morph=v.smoothMorph;
    const float mixA=std::cos(morph*pi*.5f)*clampf(p.levelA,0.f,1.5f),
                mixB=std::sin(morph*pi*.5f)*clampf(p.levelB,0.f,1.5f);
    const float baseA=readAt(s,v.position),baseB=readAt(sb,v.positionB);
    float sample=mixA*baseA+mixB*baseB;
     if(v.bankFade>0.f && v.oldSample && v.oldSampleB){
      const auto& oldA=*v.oldSample;
      const auto& oldB=*v.oldSampleB;
      const float pitchOffset=v.pitch+(bend+p.bend)*p.bendRange;
      const float oldStepA=clampf((float)oldA.rate*dt*
        std::exp2((pitchOffset-(float)oldA.root)/12.f),.01f,20.f);
      const float oldStepB=clampf((float)oldB.rate*dt*
        std::exp2((pitchOffset-(float)oldB.root)/12.f),.01f,20.f);
      const float oldSound=mixA*oldA.bandLimited(v.oldPosition,oldStepA)
                          +mixB*oldB.bandLimited(v.oldPositionB,oldStepB);
      const float oldWeight=v.bankFade;
      sample=sample*(1.f-oldWeight)+oldSound*oldWeight;
      v.oldPosition+=oldStepA;v.oldPositionB+=oldStepB;
      if(droneLoop){
       const float lengthA=(float)oldA.data.size(),lengthB=(float)oldB.data.size();
       if(v.oldPosition>lengthA*.88f)v.oldPosition=lengthA*.17f;
       if(v.oldPositionB>lengthB*.88f)v.oldPositionB=lengthB*.17f;
      }
      v.bankFade=std::max(0.f,v.bankFade-dt/.075f);
      if(v.bankFade<=0.f)v.oldSample=v.oldSampleB=nullptr;
     }
     const float pureSource=sample; // Before FM, added organisms, filter and FX.
     // DEBRIS / BODY / IMPACT. Randomness selects fragments from the *current
     // A/B bank* and runs only while MIDI voices exist. No white-noise blanket,
     // bit reduction, oscillator phase reset, or allocations in render().
     auto random01=[&](){
      v.noiseState=1664525u*v.noiseState+1013904223u;
      return (float)((v.noiseState>>8)&0xffffffu)*(1.f/16777216.f);
     };
     auto advanceFragment=[&](const Sample& source,float& pos,float speed){
      pos+=speed;
      const float n=(float)source.data.size();
      if(droneLoop){
       const float lo=n*.17f,hi=n*.87f;
       if(pos>=hi||pos<lo)pos=lo+std::fmod(std::fabs(pos-lo),hi-lo);
      }else{
       if(pos>=n-9.f)pos=n*.11f;
       if(pos<9.f)pos=9.f;
      }
     };
     if(bodyAmount>.0001f){
      // Sample-derived, pitched-down correlated body, not an unrelated sine.
      const float rawBody=mixA*readAt(s,v.bodyPosA)+
                          mixB*readAt(sb,v.bodyPosB);
      v.bodyLP+=(rawBody-v.bodyLP)*bodyCoeff;
      sample=sample*(1.f-.13f*bodyAmount)+v.bodyLP*(.90f*bodyAmount);
      advanceFragment(s,v.bodyPosA,step*bodySpeed);
      advanceFragment(sb,v.bodyPosB,stepB*bodySpeed);
     }
     const float debrisAmount=clampf(p.grain,0.f,1.f);
     if(debrisAmount>.001f){
      v.debrisCountdown-=dt;
      if(v.debrisCountdown<=0.f){
       v.debrisCountdown=(.43f-.28f*debrisAmount)*(.55f+1.4f*random01());
       for(auto& g:v.debris)if(g.progress>=1.f){
        g.posA=(float)s.data.size()*(.20f+.58f*random01());
        g.posB=(float)sb.data.size()*(.20f+.58f*random01());
        g.progress=0.f;
        g.increment=dt/(.11f+.22f*random01()+.14f*debrisAmount);
        const float randomPitch=(random01()-.5f)*9.f*clampf(p.texture,0.f,1.f);
        g.rate=std::exp2((-2.f-23.f*drop+randomPitch)/12.f);
        g.level=.76f+.38f*random01();
        break;
       }
      }
      float debrisSound=0.f;
      for(auto& g:v.debris)if(g.progress<1.f){
       const float t=g.progress;
       const float envelope=4.f*t*(1.f-t);
       debrisSound+=envelope*envelope*g.level*
          (mixA*readAt(s,g.posA)+mixB*readAt(sb,g.posB));
       advanceFragment(s,g.posA,step*g.rate);
       advanceFragment(sb,g.posB,stepB*g.rate);
       g.progress=std::min(1.f,g.progress+g.increment);
      }
      sample=sample*(1.f-.20f*debrisAmount)+debrisSound*.64f*debrisAmount;
     }
     const float hitAmount=clampf(p.hits,0.f,1.f);
     if(hitAmount>.001f){
      v.impactCountdown-=dt;
      if(v.impactCountdown<=0.f&&v.impactProgress>=1.f){
       v.impactCountdown=(.21f+1.18f*(1.f-hitAmount))*
                            (.50f+1.40f*random01());
       v.impactProgress=0.f;
       v.impactLength=.095f+.19f*random01();
       v.impactPosA=(float)s.data.size()*(.20f+.58f*random01());
       v.impactPosB=(float)sb.data.size()*(.20f+.58f*random01());
       v.impactSpeed=std::exp2((-5.f-15.f*drop+4.f*(random01()-.5f))/12.f);
       v.impactPitch=2.f*pi*(39.f+2.1f*(float)(v.note%19));
       v.impactPhase=0.f;
      }
      if(v.impactProgress<1.f){
       const float t=v.impactProgress;
       const float onset=clampf(t/.06f,0.f,1.f);
       const float envelope=onset*(1.f-t)*(1.f-t);
       const float fragment=mixA*readAt(s,v.impactPosA)+
                            mixB*readAt(sb,v.impactPosB);
       // One low-end drum-body burst *per event*, never a constant oscillator.
       const float thump=std::sin(v.impactPhase)*.16f;
       sample+=hitAmount*envelope*(1.5f*fragment+thump);
       advanceFragment(s,v.impactPosA,step*v.impactSpeed);
       advanceFragment(sb,v.impactPosB,stepB*v.impactSpeed);
       v.impactPhase+=v.impactPitch*dt*(1.f-.34f*t);
       if(v.impactPhase>2.f*pi)v.impactPhase-=2.f*pi;
       v.impactProgress=std::min(1.f,t+dt/v.impactLength);
      }
     }

     // Unlike the old inert grain parameter, GRAIN now crossfades in a
     // continuously moving delayed sample head. No buffer discontinuities.
     const float grainAmount=clampf(p.grain,0.f,1.f);
     if(grainAmount>.0001f){
      const float aOffset=(float)s.rate*(.008f+.095f*grainAmount)*
                          (.5f+.5f*std::sin(v.motionPhase));
      const float bOffset=(float)sb.rate*(.008f+.095f*grainAmount)*
                          (.5f+.5f*std::sin(v.motionPhase+.9f));
      const float shifted=mixA*readAt(s,v.position-aOffset)+
                          mixB*readAt(sb,v.positionB-bOffset);
      sample=sample*(1.f-.78f*grainAmount)+shifted*(.78f*grainAmount);
     }
     // TEXTURE supplies overt organic amplitude movement in every algorithm.
     // Zero is identical to the input bank; there is no shared default layer.
     const float texturing=clampf(p.texture,0.f,1.f);
     if(texturing>.0001f){
      const float phase=.5f+.5f*std::sin(globalTime*(1.0f+2.5f*texturing)+v.motionPhase);
      sample*=1.f-.72f*texturing*phase;
     }
     const float character=clampf(p.character,0.f,1.f);
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
    const float cross=clampf(p.xenoCross*character,0.f,1.f);
    const float interaction=cross>.0001f?std::tanh(baseA*baseB*(3.0f+1.7f*cross)):0.f;
    sample+=cross*.09f*interaction;
     // Source A and B hybridization, without a parallel stochastic texture.
     const float sym=clampf(p.symbiosis*character,0.f,1.f);
     const float hybrid=sym>.0001f?std::tanh(2.4f*baseA*baseB+.7f*(baseA-baseB)*cellular):0.f;
     sample+=.10f*sym*hybrid;
    // Shared biome cycles above feed source morph, the moving read-head, FM and filters.
    // Source mixing is also modulated by Life, not just an inaudible detune.
    sample=sample*(1.f+.12f*biome);
    v.motionPhase+=dt*(.17f+movement*1.70f);
    if(v.motionPhase>2.f*pi)v.motionPhase-=2.f*pi;
    const float drift=std::sin(v.motionPhase)+.34f*std::sin(std::sin(v.motionPhase)*.53f+1.2f);
    float layer=mixA*readAt(s,v.position+v.layerOffset*(1.f+.10f*movement*drift+.25f*biome))
               +mixB*readAt(sb,v.positionB+v.layerOffset*(float)sb.rate/(float)s.rate*(1.f+.10f*movement*drift+.25f*biome));
    // VOIDCORE CLEAN: real, continuous sample transports only.
    // All random grain-anchor resets and digital tearing have been removed.
    const int mode=std::clamp(p.textureMode,0,5);
    if(mode==1||mode==5){
      const float t=clampf(p.texture,0.f,1.f);
      const float phase=2.f*pi*(.047f+.14f*t)*globalTime+std::sin(v.motionPhase)*.12f;
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
    const float carrier=std::min(7200.f,fundamentalHz(v.pitch+gesture+(bend+p.bend)*p.bendRange));
    const float fmFreq=std::min((float)sr*.17f,carrier*(p.fmRatio+.19f*p.fmInharmonic));
    v.modPhase=wrapPhase(v.modPhase+2.f*pi*fmFreq*dt);
    const float mod=std::sin(v.modPhase+clampf(p.fmFeedback,0.f,.95f)*v.prevMod*2.5f);
    v.prevMod=mod;
    v.fmPhase=wrapPhase(v.fmPhase+2.f*pi*carrier*dt);
     v.fmAltPhase=wrapPhase(v.fmAltPhase+2.f*pi*carrier*1.019f*dt);
    const float fmAmount=clampf(p.fmIndex+.6f*destinations[5]+aggression*1.5f
       +.85f*wr*std::fabs(bio2)+.85f*linked+.95f*colony*cellular,0.f,6.5f);
    const float fmA=std::sin(v.fmPhase+fmAmount*mod);
    const float fmB=std::sin(v.fmAltPhase+(fmAmount*.7f)*std::sin(v.modPhase));
    const float fmSignal=(.68f*fmA+.32f*fmB)*(.12f+.88f*v.motionEnv);
    const float requestedFmMix=character*clampf(p.fmMix+aggression*.025f,0.f,1.f);
    v.smoothFmMix+=(requestedFmMix-v.smoothFmMix)*controlSlew;
    const float fmMix=v.smoothFmMix;
    sample=sample*(1.f-fmMix*.48f)+fmSignal*fmMix*.75f;
     // Tuned 3rd/5th/7th partials restore low-register articulation; no noise.
     if(lowLife>.0001f){
       const float creature=
         .55f*std::sin(3.f*v.fmPhase+.63f*mod+bio1*.25f)
        +.29f*std::sin(5.f*v.fmPhase+.45f*std::sin(v.modPhase)+bio2*.4f)
        +.16f*std::sin(7.f*v.fmPhase+.22f*bio3);
       const float breathe=.65f+.35f*(.5f+.5f*bio2);
       sample+=creature*character*lowLife*(.20f+.065f*coupling)*breathe
                   *(.40f+.60f*v.motionEnv);
     }
    // Irregular creature calls stay tied to the held fundamental (not MIDI arps).
    // Raised smooth envelope avoids boing onset and broadband noise.
    const float creatureAmt=character*clampf(p.creatures,0.f,1.f);
    if(creatureAmt>.0001f){
      const float rate=(.19f+creatureAmt*2.85f)*(1.f+.31f*bio2);
      v.organismPhase+=rate*dt;
      if(v.organismPhase>=1.f)v.organismPhase-=std::floor(v.organismPhase);
      const float growth=std::sin(pi*v.organismPhase);
      const float window=growth*growth*(.35f+.65f*growth);
      const float ratio=2.6f+1.1f*(.5f+.5f*bio1)+1.4f*(.5f+.5f*cellular);
      const float freq=std::min((float)sr*.18f,carrier*ratio);
      v.organismTonePhase=wrapPhase(v.organismTonePhase+2.f*pi*freq*dt);
      const float throat=std::sin(v.organismTonePhase+.84f*std::sin(v.modPhase+bio3));
      sample+=.17f*creatureAmt*window*throat*(.35f+.65f*v.motionEnv);
    }
    // The ecology has three different foreground characters, not one ambient bed.
    // 1) PREDATOR: a slow, unstable throat with formants derived from the key.
    // 2) SPORE CODE: independently drifting micro-chirps that speak in phrases.
    // 3) BURROW: downward breathing interference made from BOTH sample layers.
    // All oscillators run continuously. Raised-cosine envelopes eliminate clicks.
    const float predator=character*clampf(p.predator,0.f,1.f);
    const float insect=character*clampf(p.insectCode,0.f,1.f);
    const float burrow=character*clampf(p.burrow,0.f,1.f);
    if(predator>.0001f){
      float slow=.17f+.38f*predator+.13f*std::fabs(cellular);
      v.predatorPhase=wrapPhase(v.predatorPhase+2.f*pi*std::min((float)sr*.22f,
         carrier*(1.18f+.57f*bio1+.11f*slow))*dt);
      const float throat=std::sin(v.predatorPhase
        + (1.8f+3.4f*predator)*std::sin(v.modPhase+2.3f*bio2));
      const float breathing=.65f+.35f*std::sin(v.cellPhase+bio3);
      const float window=breathing*breathing;
      sample+=throat*(.05f+.07f*predator)*predator*window*(.3f+.7f*v.motionEnv);
    }
    if(insect>.0001f){
      // Insect calls are scattered in time, never quantized into musical notes.
      const float speed=.23f+2.05f*insect+.11f*std::sin(v.cellPhase);
      v.signalPhase+=dt*speed;
      if(v.signalPhase>=1.f)v.signalPhase-=std::floor(v.signalPhase);
      const float event=std::sin(pi*v.signalPhase);
      const float env=event*event*event*event; // continuous at event boundary
      const float glide=1.6f+.45f*std::sin(v.cellPhase+v.motionPhase);
      v.insectPhase=wrapPhase(v.insectPhase+2.f*pi*std::min((float)sr*.24f,carrier*glide)*dt);
      const float chirp=std::sin(v.insectPhase+1.5f*insect*std::sin(v.modPhase));
      sample+=.10f*insect*env*chirp*(.48f+.52f*v.motionEnv);
    }
    if(burrow>.0001f){
      const float phase=2.f*pi*(.09f+.32f*burrow)*globalTime+std::sin(v.motionPhase)*.13f;
      const float sweep=.5f+.5f*std::sin(phase+1.6f*bio2);
      const float mix=std::sin((2.3f+3.2f*burrow)*(baseA*.63f+baseB*.37f)+
         2.f*burrow*std::sin(v.cellPhase));
      sample+=burrow*(.04f+.09f*burrow)*mix*(.35f+.65f*sweep);
    }
    // Spectral wavefolding bypassed entirely to avoid bitcrushing artifacts.
    // Morphing pseudo-vocal tract with two narrow, stable resonant peaks.
    // Reacts to living biome and cross-layer motion without broadband noise.
    const float xeno=character*clampf(p.xenoFormant,0.f,1.f);
    if(xeno>.0001f){
      const float vowelPos=clampf(.5f+.27f*bio1+.18f*bio3+.15f*wr*std::sin(std::sin(v.motionPhase)*.4f)+.13f*linked+.14f*colony*cellular,0.f,1.f);
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
    const float metalAmt=character*clampf(p.metalMix+.12f*destinations[7]+aggression*.035f,0.f,1.f);
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
                  +.035f*organic*std::sin(std::sin(v.motionPhase)*.67f)+.19f*biome+.14f*linked
                  -.48f*clampf(p.character,0.f,1.f)+.09f*p.character*bio1
                  -.39f*clampf(p.dark,0.f,1.f),.008f,1.f);
    float otherCut=clampf(p.cutoff2+.27f*destinations[4]*moveDepth+envAmt*.28f
      -.13f*biome-.09f*linked-.09f*colony*cellular,.008f,1.f);
    auto filt=[&](float input,float cutoff,float resonance,int mode,float& ic2,float& ic1){
      // Zero-delay-feedback topology-preserving SVF. The previous explicit
      // integrator approached instability at 48k with high cutoff/resonance
      // and added a consistent harsh ringing to unrelated source banks.
      const float idxFloat=clampf(cutoff,0.f,1.f)*1024.f;
      const int idx=std::clamp((int)idxFloat,0,1023);
      const float u=idxFloat-(float)idx;
      const float g=filterGTable[(size_t)idx]+
        (filterGTable[(size_t)idx+1]-filterGTable[(size_t)idx])*u;
      const float k=clampf(1.60f-resonance*1.12f,.48f,1.8f);
      const float a1=1.f/(1.f+g*(g+k));
      const float a2=g*a1,a3=g*a2;
      const float v3=input-ic2;
      const float band=a1*ic1+a2*v3;
      const float low=ic2+a2*ic1+a3*v3;
      ic1=2.f*band-ic1;
      ic2=2.f*low-ic2;
      const float high=input-k*band-low;
      switch(mode){case 1:return band;case 2:return high;
       case 3:return high+low;default:return low;}

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
    // Dedicated intentional overdrive: completely off at zero; audible on
    // its own at any CLARITY setting, without an always-on saturation layer.
    const float pushed=clampf(p.filterDrive,0.f,1.f);
    if(pushed>.0001f){
      const float wetDrive=std::tanh(dry*(1.f+5.f*pushed))/
                          std::max(.1f,std::tanh(1.f+5.f*pushed));
      dry=dry*(1.f-.82f*pushed)+wetDrive*(.82f*pushed);
    }

    // GRIT is low-mid parallel analog-style abrasion, rather than an always
    // active bright hardclipper. Pre/post bandwidth controls keep fizz down.
    if(gritAmount>.001f){
     v.gritPre+=(dry-v.gritPre)*gritPreCoeff;
     const float saturated=std::tanh(v.gritPre*(1.f+7.f*gritAmount));
     v.gritPost+=(saturated-v.gritPost)*gritPostCoeff;
     dry=dry*(1.f-.68f*gritAmount)+v.gritPost*(.84f*gritAmount);
    }
    // DARK is a real rolling low-pass as well as a filter-cutoff offset;
    // neutral at zero, deeper and heavier toward its maximum.
    if(darkAmount>.001f){
     v.darkLP+=(dry-v.darkLP)*darkCoeff;
     dry=dry*(1.f-.72f*darkAmount)+v.darkLP*(.72f*darkAmount);
    }
    dry+=p.spectralTilt*(clean-v.lp)*.6f;
    // Clarity macro preserves uncompressed detail while still allowing edge.
    const float cleanFraction=clampf(p.clarity,0.f,1.f);
    const float saturationAmount=(1.f-cleanFraction)*clampf(drive*.34f+p.filterDrive*.26f,0.f,.55f);
     float saturated=std::tanh(dry*(1.f+.75f*drive));
    dry=dry*(1.f-saturationAmount)+saturated*saturationAmount;
    const float edge=(clean-v.lp)*(.02f+.04f*drive);
    dry+=edge*(1.f-cleanFraction);
    // NOISE-FREE XENOBIOME: harmonic ghost events follow the rhythmic gate
    // without injecting white/pink noise. The event is ring-modulated from
    // the actual sample, with an exponential window and warm band-limited FM.
    if(p.motif>0 && p.motifFX>.0001f && accent>.32f){
      const float ghostEnv=(1.f-std::exp(-stepPhase*48.f))*std::exp(-stepPhase*14.f);
      const float ghost=std::sin(v.fmAltPhase+.52f*std::sin(v.modPhase));
      dry+=.095f*clampf(p.motifFX,0.f,1.f)*accent*ghostEnv*ghost;
    }

    // Gentle air shelf made from the actual 48 kHz samples, not hard noise.
    v.airLP+=(sample-v.airLP)*.12f;
    dry+=(sample-v.airLP)*.10f*clampf(p.hiFiAir,0.f,1.f);
    float layerGain=character*clampf(p.layers+atmosphere*.18f,0.f,1.f)*(.29f+.32f*organic);
    layerGain*=clampf(1.f+.35f*biome+.18f*linked,.40f,1.5f);
    float stereoLayer=layer*layerGain*(.70f+.10f*drive);
    // Organic fundamental/subharmonic, never overly prominent for percussive banks.
    float fundamental=std::clamp(440.f*std::exp2((v.pitch-69.f)/12.f),20.f,1500.f);
    v.bodyPhase+=dt*fundamental*pi; // sub oscillator at half fundamental
    if(v.bodyPhase>2*pi)v.bodyPhase-=2*pi;
    float sub=character*(.21f*std::sin(v.bodyPhase)+.065f*std::sin(2.f*v.bodyPhase)) * clampf(p.sub,0.f,1.f);
    // No persistent stochastic breath layer; it previously followed the gate
    // on every bank, producing the unwanted moving white/pink-noise blanket.
    // All micro-motion comes from the sample's resonant material and biome.
    const float air=0.f;
    sub*=v.oneShot?.33f:1.f;
    // Start voices as a slow swell; rhythmic bleeps live inside the sample, not at note-on.
     const float voicedRise=clampf(v.filterAge/.034f,0.f,1.f);
     const float attackShape=voicedRise*voicedRise*(3.f-2.f*voicedRise);
     float gain=v.level*v.vel*.46f*(1.f+.15f*lowLife)*(.83f+.17f*attackShape);
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
    float orbit=(mode==2? p.texture*.75f*std::sin(std::sin(v.motionPhase)*1.35f+globalTime*.37f):0.f);
    float pan=clampf(p.pan +destinations[1]*moveDepth+orbit+.21f*biome+.12f*linked,-1.f,1.f);
    // Equal-power output pan and subtle sample-bank stereo spread
    float spread=clampf(p.stereo,0,1.5f)*(((v.note%7)-3)/12.f);
    float a=clampf(pan+spread,-1,1);
    v.smoothSolo+=(clampf(p.sourceSolo,0.f,1.f)-v.smoothSolo)*controlSlew;
    const float pureGain=v.level*v.vel*.46f*(.83f+.17f*attackShape);
    const float pureL=pureSource*pureGain*std::sqrt(.5f*(1.f-a));
    const float pureR=pureSource*pureGain*std::sqrt(.5f*(1.f+a));
    const float coloredL=(dry*(1-layerGain*.4f)+stereoLayer*.44f+sub+air)*gain*std::sqrt(.5f*(1.f-a));
    const float outL=coloredL*(1.f-v.smoothSolo)+pureL*v.smoothSolo;
    const float coloredR=(dry*(1-layerGain*.4f)+stereoLayer*.98f+sub-air*.36f)*gain*std::sqrt(.5f*(1.f+a));
    const float outR=coloredR*(1.f-v.smoothSolo)+pureR*v.smoothSolo;
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
     // Atmosphere cannot introduce an unwanted reverb layer when REV MIX is zero.
      fx.reverbMix=clampf(fx.reverbMix*(1.f+.09f*p.macroAtmosphere),0.f,.98f);
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
    smoothedSoloFx+=(clampf(p.sourceSolo,0.f,1.f)-smoothedSoloFx)*controlSlew;
    wetL*=1.f-smoothedSoloFx;wetR*=1.f-smoothedSoloFx;
   // A softly limited master bus retains transient punch and space width.
   // More headroom and less blanket drive than Earthforge v6 master limiter.
   // Soft limiting is retained to make accidental dense chords manageable.
   const float masterBoost=.93f;
   const float preL=mixL+wetL,preR=mixR+wetR;
   toneLoL+=(preL-toneLoL)*lowEq;toneLoR+=(preR-toneLoR)*lowEq;
   toneHiL+=(preL-toneHiL)*highEq;toneHiR+=(preR-toneHiR)*highEq;
   const float eqL=toneLoL*toneLoGain+(toneHiL-toneLoL)*toneMiGain+
                    (preL-toneHiL)*toneHiGain;
   const float eqR=toneLoR*toneLoGain+(toneHiR-toneLoR)*toneMiGain+
                    (preR-toneHiR)*toneHiGain;
   const float rawL=eqL*amp*masterBoost;
   const float rawR=eqR*amp*masterBoost;
   // Transparent linked peak guard: exactly linear for normal levels,
   // unlike the always-on master distortion from previous versions.
   const float peak=std::max(std::fabs(rawL),std::fabs(rawR));
   const float target=std::min(1.f,.95f/std::max(peak,1.e-8f));
   masterGain=std::min(masterGain,target); // instant overload catch
   if(target>masterGain)
    masterGain+=(target-masterGain)*(1.f-std::exp(-dt/.12f));
   left[i]+=rawL*masterGain;
   right[i]+=rawR*masterGain;
  }
 }
private:
 struct Voice {
  const Sample* sample=nullptr;
  const Sample* sampleB=nullptr;
  const Sample* oldSample=nullptr;
  const Sample* oldSampleB=nullptr;
  float oldPosition=0.f,oldPositionB=0.f,bankFade=0.f;
  int note=-1,bank=0,stage=0;
  float vel=0,position=0,positionB=0,level=0,releaseLevel=1,pitch=0,target=0,lp=0,bp=0;
  float motionPhase=0,layerOffset=0,bodyPhase=0,breath=0,aliveA=0,aliveB=0;
   float cellPhase=0,organismPhase=0,organismTonePhase=0,foldLP=0,cleanBlend=0;
   float predatorPhase=0,insectPhase=0,signalPhase=0;
  float lp2=0,bp2=0,filterEnv=0,filterAge=0,motionEnv=0,airLP=0;
  float xenoLp1=0,xenoBp1=0,xenoLp2=0,xenoBp2=0;
  float fmPhase=0,fmAltPhase=0,modPhase=0,prevMod=0,metalDamp=0,motifPitch=0,fxPhase=0,ghostLP=0;int fxStep=-1;
  std::array<float,4096> metalLine{};int metalPos=0;
  float grainPhase=0.f,fractureGain=1.f,stealL=0,stealR=0,stealFade=0,lastL=0,lastR=0;
  struct Grain {
   float posA=0.f,posB=0.f,progress=1.f,increment=0.f,rate=1.f,level=0.f;
  };
  std::array<Grain,2> debris{};
  float debrisCountdown=0.f;
  float bodyPosA=0.f,bodyPosB=0.f,bodyLP=0.f;
  float impactPosA=0.f,impactPosB=0.f,impactProgress=1.f,impactLength=.2f;
  float impactCountdown=0.f,impactSpeed=.5f,impactPitch=0.f;
  float impactPhase=0.f,gritPre=0.f,gritPost=0.f,darkLP=0.f;
  float evolveAge=0.f,evolvePhase=0.f,evolveScan=0.f;
  float smoothMorph=.2f,smoothFmMix=0.f,smoothFold=0.f,smoothSolo=0.f;
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
 std::array<float,1025> filterGTable{};
 std::array<ModState,4> modulators{};
 std::array<std::array<Sample,4>,51> samples{};
 std::array<Voice,voiceCount> voices{};
 std::array<bool,128> held{},sustained{};
 uint64_t voiceCounter=0;bool sustainDown=false;
 float bend=0,modwheel=0,lfoPhase=0,globalTime=0,tempo=145;
 double motifBeats=0.;bool hostBeatSynced=false;
 float mixLowL=0,mixLowR=0,mixMidL=0,mixMidR=0;
 float smoothedGate=1.f,smoothedAccent=1.f,smoothedSoloFx=0.f,masterGain=1.f;
 float toneLoL=0.f,toneLoR=0.f,toneHiL=0.f,toneHiR=0.f;
 PsySpace spaceFX;std::string bankFolder;
};
}
