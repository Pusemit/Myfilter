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
	//��.cpp�ļ���ʵ��createParameterLayout()������ֵ��ParameterLayout�Ķ���
	static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
	//��ʼ��һ��AudioProcessorValueTreeState����
    juce::AudioProcessorValueTreeState apvts{
		*this,//���APTVSʹ�õ�AudioProcessor
		nullptr,//����undoManager
		"Parameter",//����parameterID
		createParameterLayout() //����parameterLayout
    };
	//��������ProcessorChain���󣬰���һ��IIR::Filter<float>����
    juce::dsp::ProcessorChain<juce::dsp::IIR::Filter<float>> LeftChain;
	juce::dsp::ProcessorChain<juce::dsp::IIR::Filter<float>> RightChain;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MyFilterAudioProcessor)
};
