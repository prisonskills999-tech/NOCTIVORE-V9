#pragma once
#include <JuceHeader.h>
#include "RomplerEngine.h"
#include <atomic>

class NoctivoreProcessor final : public juce::AudioProcessor {
public:
 NoctivoreProcessor();
 void prepareToPlay(double sampleRate,int samplesPerBlock) override;
 void releaseResources()override {}
 bool isBusesLayoutSupported(const BusesLayout& layout)const override;
 void processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer& events)override;
 using juce::AudioProcessor::processBlock;
 juce::AudioProcessorEditor* createEditor() override;
 bool hasEditor() const override{return true;}
 const juce::String getName() const override{return JucePlugin_Name;}
 bool acceptsMidi()const override{return true;}
 bool producesMidi()const override{return false;}
 bool isMidiEffect()const override{return false;}
 double getTailLengthSeconds()const override{return 2;}
 int getNumPrograms()override{return 1;}
 int getCurrentProgram()override{return 0;}
 void setCurrentProgram(int)override{}
 const juce::String getProgramName(int)override{return "User Patch";}
 void changeProgramName(int,const juce::String&)override{}
 void getStateInformation(juce::MemoryBlock& data)override;
 void setStateInformation(const void* bytes,int n)override;
 juce::AudioProcessorValueTreeState state;
 juce::MidiKeyboardState keyboardState;
 bool libraryLoaded=false;
 juce::File paintingFile() const;
 std::atomic<float> peak{0.f};
 std::array<std::atomic<float>,128> scope{};
 std::atomic<unsigned> scopeCursor{0};
private:
 static juce::AudioProcessorValueTreeState::ParameterLayout layout();
 float get(const char* id)const{return state.getRawParameterValue(id)->load();}
 noctivore::Engine sampler;
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NoctivoreProcessor)
};
