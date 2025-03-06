/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class MyFilterAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    MyFilterAudioProcessor();
    ~MyFilterAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
	//在.cpp文件中实现createParameterLayout()，返回值是ParameterLayout的对象
	static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
	//初始化一个AudioProcessorValueTreeState对象
    juce::AudioProcessorValueTreeState apvts{
		*this,//这个APTVS使用的AudioProcessor
		nullptr,//设置undoManager
		"Parameter",//设置parameterID
		createParameterLayout() //设置parameterLayout
    };
	//定义两个ProcessorChain对象，包含一个IIR::Filter<float>对象
    juce::dsp::ProcessorChain<juce::dsp::IIR::Filter<float>> LeftChain;
	juce::dsp::ProcessorChain<juce::dsp::IIR::Filter<float>> RightChain;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MyFilterAudioProcessor)
};
