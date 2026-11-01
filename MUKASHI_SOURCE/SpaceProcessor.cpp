
#include "SpaceProcessor.h"
#include "SpaceEditor.h"

SpaceProcessor::SpaceProcessor() : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true).withOutput("Output", juce::AudioChannelSet::stereo(), true)),
apvts(*this, nullptr, "PARAMS", createParams()) {
    airFilter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    for (auto& s : snapshots) { s.x=0; s.y=0; s.room=40; s.wet=20; }
}

auto SpaceProcessor::createParams() -> juce::AudioProcessorValueTreeState::ParameterLayout {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
    p.push_back(std::make_unique<juce::AudioParameterFloat>("X", "Pan X", -100.f, 100.f, 0.f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("Y", "Depth Y", 0.f, 100.f, 0.f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("ROOM", "Room Size", 0.f, 100.f, 40.f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("WET", "Reverb", 0.f, 100.f, 20.f));
    return {p.begin(), p.end()};
}

void SpaceProcessor::prepareToPlay(double sr, int) {
    juce::dsp::ProcessSpec spec{sr, 512, 2};
    airFilter.prepare(spec);
    reverb.prepare(spec);
}

void SpaceProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) {
    float xNorm = apvts.getRawParameterValue("X")->load() / 100.f;
    float yNorm = apvts.getRawParameterValue("Y")->load() / 100.f;
    float roomMix = apvts.getRawParameterValue("WET")->load() / 100.f;
    float roomSize = apvts.getRawParameterValue("ROOM")->load() / 100.f;

    smoothedX += (xNorm - smoothedX) * 0.06f;
    smoothedY += (yNorm - smoothedY) * 0.06f;

    // DEPTH
    float cutoff = juce::jmap(smoothedY, 0.f, 1.f, 18500.f, 2200.f);
    airFilter.setCutoffFrequency(cutoff);
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> ctx(block);
    airFilter.process(ctx);

    float atten = juce::Decibels::decibelsToGain(juce::jmap(smoothedY, 0.f, 1.f, 0.f, -16.f));
    buffer.applyGain(atten);

    // PAN + WIDTH (стерео комната)
    float width = juce::jmap(smoothedY, 0.f, 1.f, 1.f, 0.12f);
    float panPos = smoothedX * 0.5f + 0.5f;
    float panL = std::cos(panPos * juce::MathConstants<float>::halfPi);
    float panR = std::cos((1.f - panPos) * juce::MathConstants<float>::halfPi);

    for (int i=0; i<buffer.getNumSamples(); ++i) {
        float l = buffer.getSample(0,i);
        float r = buffer.getNumChannels()>1 ? buffer.getSample(1,i) : l;
        float mid = (l+r)*0.5f;
        float nl = mid + (l-mid)*width;
        float nr = mid + (r-mid)*width;
        buffer.setSample(0,i, nl * panL);
        if (buffer.getNumChannels()>1) buffer.setSample(1,i, nr * panR);
        else if (buffer.getNumChannels()==1) {} // mono stay mono but panned via later? JUCE will keep stereo out
    }

    // ROOM reverb
    juce::dsp::Reverb::Parameters rp;
    rp.roomSize = 0.2f + roomSize * 0.8f;
    rp.damping = 0.3f + smoothedY*0.5f;
    rp.wetLevel = 0.33f;
    rp.dryLevel = 0.67f;
    rp.width = 1.f;
    rp.freezeMode = 0.f;
    reverb.setParameters(rp);

    float effWet = roomMix * (0.15f + smoothedY * 0.85f);
    if (effWet > 0.005f) {
        juce::AudioBuffer<float> wet; wet.makeCopyOf(buffer);
        juce::dsp::AudioBlock<float> wb(wet); juce::dsp::ProcessContextReplacing<float> wctx(wb);
        reverb.process(wctx);
        for (int ch=0; ch<buffer.getNumChannels(); ++ch) buffer.addFrom(ch,0,wet,ch,0,buffer.getNumSamples(), effWet);
        buffer.applyGain(1.f/(1.f+effWet*0.5f));
    }
}

void SpaceProcessor::saveSnapshot(int idx) {
    if (idx<0||idx>=numSlots) return;
    snapshots[idx].x = apvts.getRawParameterValue("X")->load();
    snapshots[idx].y = apvts.getRawParameterValue("Y")->load();
    snapshots[idx].room = apvts.getRawParameterValue("ROOM")->load();
    snapshots[idx].wet = apvts.getRawParameterValue("WET")->load();
    snapshotHasData[idx]=true;
}
void SpaceProcessor::recallSnapshot(int idx) {
    if (idx<0||idx>=numSlots||!snapshotHasData[idx]) return;
    apvts.getParameter("X")->setValueNotifyingHost(apvts.getParameter("X")->convertTo0to1(snapshots[idx].x));
    apvts.getParameter("Y")->setValueNotifyingHost(apvts.getParameter("Y")->convertTo0to1(snapshots[idx].y));
    apvts.getParameter("ROOM")->setValueNotifyingHost(apvts.getParameter("ROOM")->convertTo0to1(snapshots[idx].room));
    apvts.getParameter("WET")->setValueNotifyingHost(apvts.getParameter("WET")->convertTo0to1(snapshots[idx].wet));
}

void SpaceProcessor::getStateInformation(juce::MemoryBlock& d) {
    auto state = apvts.copyState();
    // сохраняем снапшоты в state тоже
    juce::ValueTree snapTree("SNAPSHOTS");
    for (int i=0;i<numSlots;++i){
        juce::ValueTree s("SLOT"); s.setProperty("idx", i, nullptr);
        s.setProperty("has", snapshotHasData[i], nullptr);
        s.setProperty("x", snapshots[i].x, nullptr); s.setProperty("y", snapshots[i].y, nullptr);
        s.setProperty("room", snapshots[i].room, nullptr); s.setProperty("wet", snapshots[i].wet, nullptr);
        snapTree.appendChild(s, nullptr);
    }
    state.appendChild(snapTree, nullptr);
    std::unique_ptr<juce::XmlElement> xml(state.createXml()); copyXmlToBinary(*xml,d);
}
void SpaceProcessor::setStateInformation(const void* data, int size) {
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data,size));
    if (!xml) return;
    auto tree = juce::ValueTree::fromXml(*xml);
    auto snapTree = tree.getChildWithName("SNAPSHOTS");
    if (snapTree.isValid()){
        for (auto s: snapTree){
            int idx = s.getProperty("idx");
            if (idx>=0&&idx<numSlots){
                snapshotHasData[idx]= s.getProperty("has");
                snapshots[idx].x = s.getProperty("x"); snapshots[idx].y = s.getProperty("y");
                snapshots[idx].room = s.getProperty("room"); snapshots[idx].wet = s.getProperty("wet");
            }
        }
        tree.removeChild(snapTree, nullptr);
    }
    apvts.replaceState(tree);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new SpaceProcessor(); }
