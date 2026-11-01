
#pragma once
#include <JuceHeader.h>
#include "SharedState.h"

struct HubSnapshot { std::array<TrackSlot,16> slots; };

class HubProcessor : public juce::AudioProcessor {
public:
    HubProcessor();
    void prepareToPlay(double, int) override {}
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override { /* hub is controller, audio passthrough */ }
    bool hasEditor() const override { return true; }
    juce::AudioProcessorEditor* createEditor() override;
    const juce::String getName() const override { return "MUKASHI Space HUB"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock& d) override;
    void setStateInformation(const void* data, int size) override;

    std::array<TrackSlot,16> slots;
    std::array<HubSnapshot,4> hubSnapshots;
    std::array<bool,4> hubSnapHas{false,false,false,false};
    void saveHubSnapshot(int idx); void recallHubSnapshot(int idx);
    void writeSharedFile(); void readSharedFile();

    juce::AudioProcessorValueTreeState apvts;
private:
    auto createParams()->juce::AudioProcessorValueTreeState::ParameterLayout;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HubProcessor)
};
