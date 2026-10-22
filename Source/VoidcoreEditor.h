#pragma once
#include "PluginProcessor.h"
#include "BankCatalog.h"
#include <JuceHeader.h>
#include <array>
#include <vector>
#include <memory>
#include <functional>
#include <cmath>
#include <algorithm>

class VoidcoreEditor final : public juce::AudioProcessorEditor, private juce::Timer {
public:
 explicit VoidcoreEditor(NoctivoreProcessor&);
 ~VoidcoreEditor() override;
 void paint(juce::Graphics&) override;
 void resized() override;
private:
 enum Page { Core=0, Motion, Fx, System, PageCount };
 static constexpr float W=1200.f,H=800.f;
 const juce::Colour green=juce::Colour::fromRGB(88,233,151);
 const juce::Colour red=juce::Colour::fromRGB(255,55,67);
 const juce::Colour text=juce::Colour::fromRGB(204,238,220);
 const juce::Colour dim=juce::Colour::fromRGB(102,144,128);
 const juce::Colour black=juce::Colour::fromRGB(7,12,10);
 NoctivoreProcessor& processor;
 Page page=Core;
 float clock=0.f,smoothedPeak=0.f;
 int focusLfo=0,focusEnv=0;
 bool playing=false;
 std::array<juce::String,4> pageNames{{"CORE","MOTION","FX","SYSTEM"}};
 struct Dial final : juce::Slider {
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> link;
  juce::String label;
  juce::Colour accent;
  Dial(NoctivoreProcessor&,const juce::String&,const juce::String&,juce::Colour);
  void paint(juce::Graphics&) override;
 };
 struct Fader final : juce::Slider {
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> link;
  Fader(NoctivoreProcessor&,const juce::String&);
  void paint(juce::Graphics&) override;
 };
 struct Choice final : juce::ComboBox {
  std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> link;
  Choice(NoctivoreProcessor&,const juce::String&,const juce::StringArray&);
  void paint(juce::Graphics&) override;
 };
 struct Click final : juce::TextButton {
  bool selected=false;
  juce::Colour accent;
  explicit Click(const juce::String&,juce::Colour);
  void paintButton(juce::Graphics&,bool,bool) override;
 };
 struct Positioned {
  std::unique_ptr<juce::Component> control;
  juce::Rectangle<float> area;
  int onPage=0; // -1 all pages
 };
 std::vector<Positioned> ui;
 std::array<Click*,16> steps{};
 std::array<Click*,4> pageButtons{},lfoButtons{};
 std::array<std::array<float,192>,2> previews{};
 void updatePreview(int);
 std::unique_ptr<juce::FileChooser> chooser;
 juce::Image reference;
 std::array<float,128> bankTraceA{},bankTraceB{};
 int cachedA=-1,cachedB=-1;
 juce::String lastScene="A1 FRACTURED SIGNAL";
 void knob(const char*,const char*,float,float,float,Page,int style=0);
 void fader(const char*,float,float,float,float,Page);
 void choice(const char*,juce::StringArray,float,float,float,float,Page);
 void click(const juce::String&,float,float,float,float,Page,std::function<void()>,int style=0);
 void allClick(const juce::String&,float,float,float,float,std::function<void()>,int style=0);
 
 void setPage(Page);
 float value(const char*)const;
 void set(const char*,float);
 void shift(const char*,int);
 void mutation(bool gentle);
 void preset(int);
 void save(),load();
 void timerCallback() override;
 void drawBase(juce::Graphics&);
 void panel(juce::Graphics&,juce::Rectangle<float>,const juce::String&,juce::Colour);
 void scope(juce::Graphics&,juce::Rectangle<float>,int,juce::Colour);
 void sourceWave(juce::Graphics&,juce::Rectangle<float>,bool);
 void env(juce::Graphics&,juce::Rectangle<float>);
 void lfo(juce::Graphics&,juce::Rectangle<float>,int);
 void filter(juce::Graphics&,juce::Rectangle<float>);
 void gates(juce::Graphics&,juce::Rectangle<float>);
 void effects(juce::Graphics&,juce::Rectangle<float>,int);
 void drawCore(juce::Graphics&);
 void drawDeep(juce::Graphics&);
 void visualEye(juce::Graphics&,float,float,float);
 void paintStatus(juce::Graphics&);
 void buildCore();
 void buildDeeper();
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VoidcoreEditor)
};
