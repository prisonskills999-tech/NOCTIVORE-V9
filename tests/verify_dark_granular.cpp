// Standalone DSP smoke test: compile with any C++17 compiler, no JUCE needed.
// Proves each final granular macro changes actual audio, emits no NaNs and
// avoids the old always-on blanket layer at zero.
#include "RomplerEngine.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <vector>

static constexpr int sr=48000;
static void u16(std::ofstream& f,uint16_t n){
 char b[2]={(char)(n&255),(char)((n>>8)&255)};
 f.write(b,2);
}
static void u32(std::ofstream& f,uint32_t n){
 char b[4]={(char)(n&255),(char)((n>>8)&255),(char)((n>>16)&255),(char)((n>>24)&255)};
 f.write(b,4);
}
static void fixture(const std::filesystem::path& dest){
 constexpr int n=sr*3;
 std::ofstream f(dest,std::ios::binary);
 if(!f)throw std::runtime_error("Unable to create wave fixture");
 f.write("RIFF",4);u32(f,36+2*n);f.write("WAVEfmt ",8);
 u32(f,16);u16(f,1);u16(f,1);u32(f,sr);u32(f,sr*2);u16(f,2);u16(f,16);
 f.write("data",4);u32(f,n*2);
 for(int i=0;i<n;++i){
  const float t=(float)i/(float)sr;
  const float pulses=std::exp(-23.f*std::fmod(t,.41f));
  const float body=std::sin(2.f*noctivore::pi*82.f*t);
  const float high=std::sin(2.f*noctivore::pi*710.f*t);
  const float siren=std::sin(2.f*noctivore::pi*(190.f*t+.08f*std::sin(t*4.f)));
  const float value=.21f*body+.14f*high+.13f*siren+.18f*pulses*std::sin(2.f*noctivore::pi*120.f*t);
  const auto encoded=(int16_t)std::lround(std::clamp(value,-.99f,.99f)*32767.f);
  u16(f,(uint16_t)encoded);
 }
}
static noctivore::Params neutral(){
 noctivore::Params p{};
 p.bank=0;p.bankB=0;p.morph=0.f;p.morphMotion=0.f;
 p.cutoff=.97f;p.resonance=.05f;p.character=0.f;p.fmMix=0.f;
 p.grain=0.f;p.texture=0.f;p.body=0.f;p.drop=0.f;
 p.evolution=0.f;
 p.hits=0.f;p.grit=0.f;p.dark=0.f;
 p.attack=.01f;p.decay=1.f;p.sustain=1.f;p.release=.3f;
 p.organic=0.f;p.motif=0;p.trancePulse=0.f;p.biomeDepth=0.f;
 p.lowRegisterLife=0.f;p.mudCut=0.f;p.focus=0.f;
 p.hiFiAir=0.f;p.volume=.65f;p.clarity=1.f;p.drive=0.f;p.filterDrive=0.f;
 p.toneLow=p.toneMid=p.toneHigh=.5f;
 p.fx.delayMix=0.f;p.fx.reverbMix=0.f;
 return p;
}
static std::vector<float> play(const std::filesystem::path& dir,const noctivore::Params& p){
 noctivore::Engine engine;
 engine.prepare(sr);
 if(!engine.loadLibrary(dir.string()))throw std::runtime_error("Fixture WAV failed to load");
 engine.setParameters(p);
 engine.noteOn(36,.92f);
 std::vector<float> left(sr),right(sr);
 engine.render(left.data(),right.data(),sr);
 for(size_t i=0;i<left.size();++i){
  if(!std::isfinite(left[i])||!std::isfinite(right[i]))
   throw std::runtime_error("Nonfinite signal");
  if(std::fabs(left[i])>1.01f||std::fabs(right[i])>1.01f)
   throw std::runtime_error("Uncontrolled master peak");
 }
 return left;
}
static double energy(const std::vector<float>& x){
 double e=0;for(float v:x)e+=(double)v*v;
 return e/std::max<size_t>(1,x.size());
}
static double difference(const std::vector<float>& a,const std::vector<float>& b){
 double e=0;for(size_t i=0;i<a.size();++i){
  const double d=(double)a[i]-(double)b[i];e+=d*d;
 }
 return std::sqrt(e/std::max(1e-12,energy(a)*(double)a.size()));
}
static void requiresChange(const char* name,const std::vector<float>& clean,const std::vector<float>& changed,double minimum=.04){
 const double r=difference(clean,changed);
 std::cout<<name<<" RMS difference: "<<r<<"\n";
 if(r<minimum)throw std::runtime_error(std::string(name)+" is not audibly connected to DSP");
}
int main(){
 try{
  const auto dir=std::filesystem::temp_directory_path()/"voidcore_granular_dsp_smoke";
  std::filesystem::create_directories(dir);
  fixture(dir/"00_36.wav");
  auto p=neutral();
  const auto base=play(dir,p);
  if(energy(base)<1e-7)throw std::runtime_error("Silent playback fixture");
  p.dark=.92f;requiresChange("DARK",base,play(dir,p));p=neutral();
  p.evolution=1.f;requiresChange("EVOLVE",base,play(dir,p),.025);p=neutral();
  p.grit=.82f;requiresChange("GRIT",base,play(dir,p));p=neutral();
  p.body=.83f;requiresChange("BODY",base,play(dir,p));p=neutral();
  p.hits=.91f;requiresChange("HITS",base,play(dir,p));p=neutral();
  p.grain=.91f;requiresChange("GRAIN",base,play(dir,p));p=neutral();
  p.grain=.90f;p.body=.70f;p.drop=0.f;
  const auto pitchedHigh=play(dir,p);
  p.drop=1.f;requiresChange("DROP",pitchedHigh,play(dir,p));p=neutral();
  p.grain=.55f;p.body=.48f;p.hits=.45f;p.dark=.46f;p.grit=.40f;p.drop=.62f;
  const auto full=play(dir,p);
  if(energy(full)<1e-7)throw std::runtime_error("Combined preset silent");
  std::cout<<"PASS: dark granular audio affects real samples; output finite and capped\n";
  return 0;
 }catch(const std::exception& e){
  std::cerr<<"FAIL: "<<e.what()<<"\n";return 1;
 }
}
