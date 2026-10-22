// Independent C++17 regression for the JUCE-free VOIDCORE sample engine.
// Build: c++ -std=c++17 -O2 -I. tests/test_hifi_resampler.cpp -o /tmp/voidcore-hifi && /tmp/voidcore-hifi
#include "Source/RomplerEngine.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>

int main(){
 using noctivore::Sample;
 Sample::warmUpResampler();
 Sample s;
 s.rate=48000;s.root=60;s.data.resize(48000);
 constexpr float tau=6.2831853071795864769f;
 // This tone is audible at source speed but above destination Nyquist when
 // pitch shifted by an octave. Naive cubic/linear playback aliases to 7 kHz.
 for(size_t i=0;i<s.data.size();++i)
  s.data[i]=std::sin(tau*17000.f*(float)i/48000.f)*.6f;
 double raw=0.0,clean=0.0;
 for(int i=50;i<20000;++i){
  const float pos=(float)i*2.f;
  const float naive=s.cubic(pos);
  const float filtered=s.bandlimited(pos,2.f);
  assert(std::isfinite(filtered));
  raw+=(double)naive*naive;clean+=(double)filtered*filtered;
 }
 const double suppression=10.*std::log10(std::max(clean,1e-20)/std::max(raw,1e-20));
 std::cout<<"Octave-up 17kHz alias suppression: "<<suppression<<" dB\n";
 if(suppression> -12.0)return 1;
 // Upward pitch changes must not overshoot to NaNs or generate DC on silence.
 Sample quiet=s;std::fill(quiet.data.begin(),quiet.data.end(),0.f);
 for(float pos=0;pos<20000.f;pos+=1.3f){
  if(std::fabs(quiet.bandlimited(pos,1.8f))>1e-8f)return 2;
 }
 // Check source sample all 50 banks exist by invariant elsewhere, not here.
 std::cout<<"PASS: anti-aliased resampling, silence, stable finite output\n";
 return 0;
}
