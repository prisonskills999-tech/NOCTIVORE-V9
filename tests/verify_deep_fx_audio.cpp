#include "PsySpace.h"
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <algorithm>
using noctivore::PsySpace;
constexpr int sr=48000;
std::vector<float> render(PsySpace::Controls p){
 PsySpace engine;engine.prepare(sr);engine.setTempo(100);engine.setControls(p);
 std::vector<float> result(sr*3);
 for(size_t i=0;i<result.size();++i){
  float left=0,right=0;
  // Realistic bank-like chord + a sharp source impact, dry only in input.
  const float t=(float)i/sr;
  float source=(i<sr*2)?(.13f*std::sin(2.f*3.14159265f*110.f*t)
            +.09f*std::sin(2.f*3.14159265f*577.f*t)):0.f;
  if(i<sr/16)source+=.35f*std::exp(-70.f*t);
  engine.tick(source,source*.88f,left,right);
  if(!std::isfinite(left)||!std::isfinite(right)||std::abs(left)>8||std::abs(right)>8)
   throw std::runtime_error("nonfinite/unbounded DSP wet output");
  result[i]=left*.75f+right*.25f; // preserve side energy to detect stereo width
 }
 return result;
}
float delta(const std::vector<float>& a,const std::vector<float>& b){
 double sum=0,energy=0;
 for(size_t i=0;i<a.size();++i){
  double d=a[i]-b[i];sum+=d*d;energy+=(double)a[i]*a[i];}
 return (float)std::sqrt(sum/std::max(energy,1e-12));
}
void differs(const char* name,const std::vector<float>& baseline,PsySpace::Controls p,float minimum=.025f){
 auto output=render(p);
 auto d=delta(baseline,output);
 std::cout<<name<<" normalized wet audio difference "<<d<<"\n";
 if(d<minimum)throw std::runtime_error(std::string("Inactive FX setting: ")+name);
}
int main(){
 try{
  PsySpace::Controls p{};p.delayMix=.85f;p.reverbMix=0.f;
  p.delayFeedback=.74f;p.reverbDecay=.65f;p.duck=0.f;p.psySpiral=0.f;
  auto echo=render(p);
  for(int mode=1;mode<4;++mode){p.delayMode=mode;differs("delay algorithm",echo,p,.04f);}p.delayMode=0;
  p.delayDrive=.95f;differs("delay COLOR",echo,p);p.delayDrive=0.f;
  p.delayFreeze=1;differs("delay HOLD",echo,p);p.delayFreeze=0;
  p.delayMix=0.f;p.reverbMix=.86f;p.reverbWidth=.85f;
  auto verb=render(p);
  for(int mode=1;mode<4;++mode){p.reverbMode=mode;differs("reverb algorithm",verb,p,.02f);}p.reverbMode=0;
  p.reverbWidth=.03f;differs("reverb WIDTH",verb,p);p.reverbWidth=.85f;
  p.reverbFreeze=1;differs("reverb HOLD",verb,p,.02f);
  std::cout<<"PASS: eight audible deep FX changes, bounded 48 kHz DSP render\n";
 }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<"\n";return 1;}
}
