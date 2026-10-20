#pragma once
#include "PluginProcessor.h"
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
 struct Section {
  Page page;
  juce::String title;
  juce::Colour accent;
  int columns=3;
  juce::Rectangle<int> bounds{};
  std::vector<juce::Component*> children;
 };
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
 void layout(Section&,juce::Rectangle<int>);
 void timerCallback() override;
 Page page=Atlas;
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NoctivoreEditor)
};
