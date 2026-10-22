#pragma once
// Original, real-time safe stereo space engine. No external DSP dependencies.
// Tempo-locked ping-pong delay with modulated taps / feedback filtering,
// 8-line modulated feedback-delay-network reverb, early reflections and ducking.
#include <array>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

namespace noctivore {
class PsySpace final {
public:
 struct Controls {
  float delayMix=.24f, delayFeedback=.51f, delayTone=.63f, delayWarp=.29f;
  float delaySpread=.87f, delayDiffusion=.52f;
  int division=3;
  float reverbMix=.34f, reverbSize=.89f, reverbDecay=.78f;
  float reverbDark=.49f, reverbMod=.71f, reverbPreDelay=.19f;
  float shimmer=.09f, duck=.24f;
  float wetLowCut=.64f; // HP feeds both delay and reverb: no sub swamp
  float psySpiral=.28f, psySpiralRate=.16f;
 };
 void prepare(double sampleRate) {
  sr=std::max(8000.0,sampleRate); phase=0; envelope=0;
  const size_t len=(size_t)(sr*4.25)+16;
  delayL.assign(len,0);delayR.assign(len,0);delayPos=0;
  preL.assign((size_t)(sr*.22)+64,0);preR.assign(preL.size(),0);prePos=0;
  const float times[8]={.0371f,.0411f,.0437f,.0499f,.0531f,.0593f,.0617f,.0679f};
  for(int i=0;i<8;++i){lines[(size_t)i].data.assign((size_t)(sr*times[i]*2.10)+128,0.f);
   lines[(size_t)i].pos=0;lines[(size_t)i].damp=0;lines[(size_t)i].hp=0;
   lines[(size_t)i].base=times[i];}
  toneL=toneR=hpL=hpR=smoothedDelay=0; inLP1L=inLP1R=inLP2L=inLP2R=0;
  cachedWetCut=-1.f;setControls(controls);
  attackSlew=1.f-std::exp(-1.f/(float)(sr*.009));
  releaseSlew=1.f-std::exp(-1.f/(float)(sr*.23));
  spiralInL.fill(0.f);spiralOutL.fill(0.f);spiralInR.fill(0.f);spiralOutR.fill(0.f);
  for(auto& v:diffL)v=0;
  for(auto& v:diffR)v=0;
 }
 void setTempo(double bpm){tempo=std::clamp(bpm,40.0,260.0);}
 void setControls(Controls c){
  if(cachedWetCut!=c.wetLowCut){
    cachedWetCut=c.wetLowCut;
    const float hz=55.f*std::pow(8.73f,std::clamp(c.wetLowCut,0.f,1.f));
    wetCutCoeff=1.f-std::exp(-6.283185307f*hz/(float)sr);
  }
  controls=c;
 }
 void clear(){prepare(sr);}
 // The output is added to the dry signal externally; no allocations in tick().
 void tick(float inL,float inR,float& outL,float& outR){
  const auto& c=controls;
  // Two-pole high-pass on the FX send only: the dry bass stays intact.
  // Low-cut 0..1 roughly corresponds to 55..480 Hz (12 dB/oct).
  const float alpha=wetCutCoeff;
  inLP1L+=(inL-inLP1L)*alpha; inLP1R+=(inR-inLP1R)*alpha;
  const float hp1L=inL-inLP1L, hp1R=inR-inLP1R;
  inLP2L+=(hp1L-inLP2L)*alpha;inLP2R+=(hp1R-inLP2R)*alpha;
  inL=hp1L-inLP2L;inR=hp1R-inLP2R;
  const float inputEnergy=std::max(std::abs(inL),std::abs(inR));
  const float smooth=inputEnergy>envelope?attackSlew:releaseSlew;
  envelope+=(inputEnergy-envelope)*smooth;
  float duck=1.f-std::clamp(c.duck,0.f,1.f)*std::clamp(envelope*2.6f,0.f,.83f);
  phase+=1.f/(float)sr;
  if(phase>4096.f)phase-=4096.f;
  const float warp=std::clamp(c.delayWarp,0.f,1.f);
  static constexpr float beatDivisions[]={1.f, .5f,.75f,.25f,.375f,.125f, 1.f/3.f, 2.f/3.f};
  const float beatSec=60.f/(float)tempo;
  float desired=(float)sr*beatSec*beatDivisions[std::clamp(c.division,0,7)];
  desired=std::clamp(desired,64.f,(float)delayL.size()-100.f);
  if(smoothedDelay<=0)smoothedDelay=desired;
  smoothedDelay+=(desired-smoothedDelay)*.00025f;
  // Dual out-of-phase modulation; the time deviation never crosses input head.
  float timeMod=warp*(.002f+.009f*warp)*(float)sr;
  float tapL=smoothedDelay+timeMod*std::sin(phase*6.2831853f*.23f);
  float tapR=smoothedDelay*1.008f-timeMod*std::sin(phase*6.2831853f*.19f+.8f);
  float feedbackL=read(delayL,delayPos,tapL);
  float feedbackR=read(delayR,delayPos,tapR);
  // Saturated, dark feedback with DC reduction to avoid swampy low-frequency buildup.
  float lpCoeff=.018f+std::clamp(c.delayTone,0.f,1.f)*.56f;
  toneL+=(feedbackL-toneL)*lpCoeff;toneR+=(feedbackR-toneR)*lpCoeff;
  hpL+=(toneL-hpL)*.00065f;hpR+=(toneR-hpR)*.00065f;
  float darkL=toneL-hpL,darkR=toneR-hpR;
  float diffusion=std::clamp(c.delayDiffusion,0.f,1.f)*.30f;
  float spread=std::clamp(c.delaySpread,0.f,1.f);
  float fb=std::clamp(c.delayFeedback,0.f,.92f);
  float writeL=std::tanh((inL+fb*(darkL*(1-spread)+darkR*spread)+diffusion*diffL[0])*.88f);
  float writeR=std::tanh((inR+fb*(darkR*(1-spread)+darkL*spread)+diffusion*diffR[0])*.88f);
  diffL[0]=darkL;diffR[0]=darkR;
  delayL[delayPos]=writeL;delayR[delayPos]=writeR;
  if(++delayPos>=delayL.size())delayPos=0;
  float wetDL=std::tanh((feedbackL*.86f+feedbackR*.14f)*1.25f);
  float wetDR=std::tanh((feedbackR*.86f+feedbackL*.14f)*1.25f);

  // Up to 120ms pre-delay plus short independent early reflections.
  size_t preCount=preL.size();
  float pd=(float)sr*(.004f+std::clamp(c.reverbPreDelay,0.f,1.f)*.115f);
  preL[prePos]=.78f*inL+.22f*wetDL;
  preR[prePos]=.78f*inR+.22f*wetDR;
  float earlyL=read(preL,prePos,pd);
  float earlyR=read(preR,prePos,pd);
  earlyL+=.28f*read(preR,prePos,pd+sr*.0121f);
  earlyR+=.28f*read(preL,prePos,pd+sr*.0179f);
  if(++prePos>=preCount)prePos=0;

  // 8-dimensional unitary Hadamard FDN with per-line lowpass, DC block,
  // fractional modulated reads; decay mapping stays below unity.
  std::array<float,8> t{};
  float sz=.57f+std::clamp(c.reverbSize,0.f,1.f)*1.22f;
  const float mod=std::clamp(c.reverbMod,0.f,1.f);
  const float dark=std::clamp(c.reverbDark,0.f,1.f);
  for(size_t k=0;k<8;++k){
   auto& line=lines[k];
   float speed=.09f+.013f*(float)k;
   float flutter=std::sin(6.2831853f*(phase*speed+(float)k*.137f));
   float readSamples=(float)sr*line.base*sz+(float)sr*(.00035f+mod*.0024f)*flutter;
   float v=read(line.data,line.pos,readSamples);
   float tone=.45f-dark*.405f;
   line.damp+=(v-line.damp)*tone;
   line.hp+=(line.damp-line.hp)*.00040f;
   t[k]=line.damp-line.hp;
  }
  // In-place fast Walsh-Hadamard transform divided by sqrt(8).
  auto mixed=t;
  for(size_t stride=1;stride<8;stride*=2)
   for(size_t base=0;base<8;base+=stride*2)
    for(size_t j=0;j<stride;++j){
     float a=mixed[base+j],b=mixed[base+j+stride];
     mixed[base+j]=a+b;mixed[base+j+stride]=a-b;
    }
  float tailGain=.75f+std::clamp(c.reverbDecay,0.f,1.f)*.245f;
  float monoIn=(earlyL+earlyR)*.5f;
  for(size_t k=0;k<8;++k){
   float injection=(k%2==0?earlyL:earlyR)*.18f+monoIn*.055f;
   // Finite, bounded feedback; shimmer adds only subtle bright overtone energy.
   float harmonic=std::clamp(c.shimmer,0.f,1.f)*.075f*(t[k]-t[(k+3)%8]);
   auto& line=lines[k];
   line.data[line.pos]=std::tanh(mixed[k]*(.353553391f*tailGain)+injection+harmonic);
   if(++line.pos>=line.data.size())line.pos=0;
  }
  // Orthogonal-ish output taps: stereo tails are different, not duplicated mono.
  float rvL=(t[0]+t[2]-t[3]+t[5]-t[7])*.28f;
  float rvR=(t[1]-t[2]+t[4]-t[5]+t[6])*.28f;
  rvL=std::tanh(rvL*1.7f+earlyL*.17f);
  rvR=std::tanh(rvR*1.7f+earlyR*.17f);
  // Slowly sweeping four-stage allpass cloud: phase animation without
  // boosting narrow metallic resonances. Stages use opposite stereo sweeps.
  const float spiral=std::clamp(c.psySpiral,0.f,1.f);
  float shimmerL=wetDL*.65f+rvL*.85f;
  float shimmerR=wetDR*.65f+rvR*.85f;
  for(size_t stage=0;stage<4;++stage){
   float t=phase*6.283185307f*std::clamp(c.psySpiralRate,.005f,2.f);
   float aL=.14f+.60f*(.5f+.5f*std::sin(t+stage*.84f));
   float aR=.14f+.60f*(.5f+.5f*std::sin(t+stage*.84f+1.61f));
   float outPhaseL=-aL*shimmerL+spiralInL[stage]+aL*spiralOutL[stage];
   float outPhaseR=-aR*shimmerR+spiralInR[stage]+aR*spiralOutR[stage];
   spiralInL[stage]=shimmerL;spiralOutL[stage]=outPhaseL;
   spiralInR[stage]=shimmerR;spiralOutR[stage]=outPhaseR;
   shimmerL=outPhaseL;shimmerR=outPhaseR;
  }
  float baseL=std::clamp(c.delayMix,0.f,1.f)*wetDL*.65f+std::clamp(c.reverbMix,0.f,1.f)*rvL;
  float baseR=std::clamp(c.delayMix,0.f,1.f)*wetDR*.65f+std::clamp(c.reverbMix,0.f,1.f)*rvR;
  float swirlGain=spiral*.28f*(c.reverbMix+c.delayMix);
  outL=duck*(baseL*(1.f-spiral*.15f)+swirlGain*shimmerL);
  outR=duck*(baseR*(1.f-spiral*.15f)+swirlGain*shimmerR);
 }
private:
 struct Line{std::vector<float> data;size_t pos=0;float damp=0,hp=0,base=0;};
 static float read(const std::vector<float>& ring,size_t pos,float back){
  if(ring.size()<4)return 0;
  float where=(float)pos-back;
  const float n=(float)ring.size();
  while(where<0)where+=n;
  while(where>=n)where-=n;
  size_t i=(size_t)where;size_t next=(i+1)%ring.size();
  float f=where-(float)i;
  return ring[i]+(ring[next]-ring[i])*f;
 }
 double sr=44100,tempo=145;
 Controls controls{};
 std::vector<float> delayL,delayR,preL,preR;
 std::array<Line,8> lines{};
 size_t delayPos=0,prePos=0;
 float toneL=0,toneR=0,hpL=0,hpR=0;
 std::array<float,1> diffL{},diffR{};
 std::array<float,4> spiralInL{},spiralOutL{},spiralInR{},spiralOutR{};
 float smoothedDelay=0,phase=0,envelope=0;
 float inLP1L=0,inLP1R=0,inLP2L=0,inLP2R=0;
 float cachedWetCut=-1.f,wetCutCoeff=.02f,attackSlew=.002f,releaseSlew=.0001f;
};
} // namespace noctivore
