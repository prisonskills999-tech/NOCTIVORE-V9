#pragma once
#include "PluginProcessor.h"
#include "PaintedForest.h"
#include <array>
#include <memory>
#include <vector>

class NoctivoreEditor final : public juce::AudioProcessorEditor, private juce::Timer {
public:
 explicit NoctivoreEditor(NoctivoreProcessor&);
 ~NoctivoreEditor() override;
 void paint(juce::Graphics&) override;
 void resized() override;
private:
 enum Page { Atlas=0, Forge, Motion, Space, System };
 struct Skin final : juce::LookAndFeel_V4 {
  Skin();
  void drawRotarySlider(juce::Graphics&,int,int,int,int,float,float,float,juce::Slider&) override;
  void drawButtonBackground(juce::Graphics&,juce::Button&,const juce::Colour&,bool,bool) override;
  void drawComboBox(juce::Graphics&,int,int,bool,int,int,int,int,juce::ComboBox&) override;
 } skin;
 struct Dial final : juce::Component {
  juce::Slider knob;
  juce::Label name;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
  Dial(juce::AudioProcessorValueTreeState&,const juce::String&,const juce::String&,int);
  void resized() override;
 };
 struct Selector final : juce::Component {
  juce::ComboBox box;
  juce::Label name;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;
  Selector(juce::AudioProcessorValueTreeState&,const juce::String&,const juce::String&,const juce::StringArray&);
  void resized() override;
 };
 struct ArtKnob final : juce::Slider {
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
  ArtKnob(juce::AudioProcessorValueTreeState&,const juce::String&,const juce::String&);
  void paint(juce::Graphics&) override;
 };
 struct ArtFader final : juce::Slider {
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
  ArtFader(juce::AudioProcessorValueTreeState&,const juce::String&,const juce::String&);
  void paint(juce::Graphics&) override;
 };
 struct ArtMenu final : juce::ComboBox {
  std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;
  ArtMenu(juce::AudioProcessorValueTreeState&,const juce::String&,const juce::StringArray&);
 };
 struct ArtHotspot final : juce::Button {
  ArtHotspot():juce::Button("art_hotspot"){}
  bool selected=false;
  void paintButton(juce::Graphics&,bool hover,bool down) override;
 };
 struct ArtDialSpot { std::unique_ptr<ArtKnob> knob; float x=0,y=0,d=54; };
 struct ArtFaderSpot { std::unique_ptr<ArtFader> fader; float x=0,y=0,w=0,h=0; };
 struct ArtMenuSpot { std::unique_ptr<ArtMenu> menu; float x=0,y=0,w=0,h=0; };
 struct ArtButtonSpot { std::unique_ptr<ArtHotspot> button; float x=0,y=0,w=0,h=0; };
 struct Section {
  Page page;
  juce::String title;
  juce::Colour accent;
  int columns=3;
  juce::Rectangle<int> bounds{};
  std::vector<juce::Component*> children;
 };
 juce::Image originalPainting;
 std::unique_ptr<juce::FileChooser> presetChooser;
 float uiSeconds=0.f, smoothedPeak=0.f;
 int currentGateStep=-1;
 bool hasCustomPatch=false;
 std::vector<ArtDialSpot> artDials;
 std::vector<ArtMenuSpot> artMenus;
 std::vector<ArtFaderSpot> artFaders;
 std::vector<ArtButtonSpot> artButtons;
 bool paintedPage() const {return page==Atlas && originalPainting.isValid();}
 NoctivoreProcessor& processor;
 juce::MidiKeyboardComponent keyboard;
 std::vector<std::unique_ptr<juce::Component>> controls;
 std::vector<Section> sections;
 std::array<juce::TextButton,5> tabs{};
 std::array<juce::TextButton,3> actions{};
 juce::ComboBox scenes;
 juce::Label sceneLabel;
 Section& section(Page,const juce::String&,juce::Colour,int columns=3);
 void dial(Section&,const char* id,const char* label,int colour=0);
 void select(Section&,const char* id,const char* label,juce::StringArray options);
 void showPage(Page);
 void applyScene(int);
 void mutate(bool subtle);
 void setParameter(const char*,float);
 void stepChoice(const char*,int,int);
 void stepScene(int);
 void savePreset();
 void loadPreset();
 void drawAnimation(juce::Graphics&);
 void drawSubpage(juce::Graphics&);
 void layout(Section&,juce::Rectangle<int>);
 void timerCallback() override;
 Page page=Atlas;
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NoctivoreEditor)
};
