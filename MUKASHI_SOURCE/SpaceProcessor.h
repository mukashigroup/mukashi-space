
#pragma once
#include <JuceHeader.h>

struct Snapshot {
    float x = 0.f; // -100..100
    float y = 0.f; // 0..100
    float room = 40.f;
    float wet = 20.f;
};

class SpaceProcessor : public juce::AudioProcessor {
public:
    SpaceProcessor();
    void prepareToPlay(double sr, int bs) override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void releaseResources() override {}
    bool hasEditor() const override { return true; }
    juce::AudioProcessorEditor* createEditor() override;

    const juce::String getName() const override { return "MUKASHI Space v2"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.3; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock& d) override;
    void setStateInformation(const void* data, int size) override;

    // Snapshots A,B,C,D
    static constexpr int numSlots = 4;
    std::array<Snapshot, numSlots> snapshots;
    std::array<bool, numSlots> snapshotHasData{false, false, false, false};
    void saveSnapshot(int idx);
    void recallSnapshot(int idx);
    
    juce::AudioProcessorValueTreeState apvts;
private:
    auto createParams() -> juce::AudioProcessorValueTreeState::ParameterLayout;
    juce::dsp::StateVariableTPTFilter<float> airFilter;
    juce::dsp::Reverb reverb;
    float smoothedX=0, smoothedY=0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpaceProcessor)
};
