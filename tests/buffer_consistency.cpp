// NOCTIVORE audio regression: host buffer sizes must not change the sound.
// Compiles without JUCE, a VST3 host, or any test-framework dependencies.
#include "RomplerEngine.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
using namespace noctivore;

struct Render {
 std::vector<float> l,r;
 bool ok=false;
};

Render render(const char* library,int chunk){
 Engine engine;
 if(!engine.loadLibrary(library))return {};
 engine.prepare(48000.);
 Params p;
 p.bank=0;p.bankB=7;p.resampling=1;
 p.attack=.12f;p.release=1.8f;
 p.textureMode=4;p.texture=.86f;
 p.motif=3;p.motifGate=.82f;p.motifDivision=1;
 p.psyFold=.70f;p.fmMix=.58f;p.fmRatio=3.4f;p.fmIndex=5.f;
 p.lfos[0]={.23f,.76f,4,3,0}; // smoothed sample/hold -> filter
 p.lfos[1]={.18f,.58f,3,5,0}; // smoothed square -> FM
 p.fx.delayMix=.19f;p.fx.reverbMix=.37f;
 p.fx.reverbSize=.82f;p.fx.reverbDecay=.75f;
 engine.setParameters(p);
 engine.setTempo(145.);
 engine.setBeatClock(16.);
 engine.noteOn(60,.8f);
 constexpr int samples=48000*2;
 Render result;
 result.l.resize(samples,0.f);result.r.resize(samples,0.f);
 for(int pos=0;pos<samples;pos+=chunk){
   const int n=std::min(chunk,samples-pos);
   // Mimic Ableton: new host PPQ and parameter snapshot on every audio buffer.
   engine.setTempo(145.);
   engine.setBeatClock(16.+(double)pos*145./(48000.*60.));
   engine.setParameters(p);
   engine.render(result.l.data()+pos,result.r.data()+pos,n);
 }
 result.ok=true;
 return result;
}
int main(int argc,char**argv){
 if(argc!=2){std::fprintf(stderr,"Usage: buffer_test <BankFolder>\n");return 2;}
 auto at512=render(argv[1],512);
 auto at256=render(argv[1],256);
 auto at128=render(argv[1],128);
 if(!at512.ok||!at256.ok||!at128.ok){
  std::fprintf(stderr,"Sound library missing: %s\n",argv[1]);return 2;
 }
 float maxPeak=0.f,err128=0.f,err256=0.f;
 for(size_t i=0;i<at512.l.size();++i){
  maxPeak=std::max({maxPeak,std::abs(at512.l[i]),std::abs(at512.r[i])});
  err128=std::max({err128,std::abs(at512.l[i]-at128.l[i]),std::abs(at512.r[i]-at128.r[i])});
  err256=std::max({err256,std::abs(at512.l[i]-at256.l[i]),std::abs(at512.r[i]-at256.r[i])});
 }
 std::printf("Samples: %zu  peak: %.5f  Δ128: %.7f  Δ256: %.7f\n",
   at512.l.size(),maxPeak,err128,err256);
 if(maxPeak<.0002f||!std::isfinite(maxPeak)||maxPeak>1.001f){
  std::fprintf(stderr,"Silence, nonfinite, or clipping in render\n");return 1;
 }
 if(err128>1.e-4f||err256>1.e-4f){
  std::fprintf(stderr,"Host buffer size audibly changes DSP output\n");return 1;
 }
 std::puts("PASS: 128 / 256 / 512 render identical within 0.0001");
 return 0;
}
