
#include "HubProcessor.h"
#include "HubEditor.h"

HubProcessor::HubProcessor() : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true).withOutput("Output", juce::AudioChannelSet::stereo(), true)),
apvts(*this, nullptr, "PARAMS", createParams()){
    juce::StringArray names = {"KICK","SNARE","HIHAT","BASS","GTR L","GTR R","KEYS","VOX LEAD","VOX BG","PERC","SYNTH","FX","808","CLAP","PAD","MISC"};
    juce::Array<juce::Colour> cols = {juce::Colours::orange, juce::Colours::red, juce::Colours::yellow, juce::Colours::cyan, juce::Colours::lime, juce::Colours::greenyellow, juce::Colours::violet, juce::Colours::white, juce::Colours::lightgrey, juce::Colours::orange, juce::Colours::aqua, juce::Colours::pink, juce::Colours::gold, juce::Colours::skyblue, juce::Colours::mediumpurple, juce::Colours::grey};
    for(int i=0;i<16;++i){ slots[i].id="track_"+juce::String(i); slots[i].name=names[i]; slots[i].color=cols[i]; slots[i].x=juce::Random::getSystemRandom().nextFloat()*60-30; slots[i].y=juce::Random::getSystemRandom().nextFloat()*70; slots[i].active=true; }
    readSharedFile();
}
auto HubProcessor::createParams()->juce::AudioProcessorValueTreeState::ParameterLayout { return {}; }
void HubProcessor::writeSharedFile(){
    auto f = getSharedFile(); f.replaceWithText(juce::JSON::toString(slotsToJson(slots), true));
}
void HubProcessor::readSharedFile(){
    auto f = getSharedFile(); if(!f.existsAsFile()) return;
    auto txt = f.loadFileAsString(); auto v = juce::JSON::parse(txt); jsonToSlots(v, slots);
}
void HubProcessor::saveHubSnapshot(int idx){ if(idx<0||idx>=4) return; hubSnapshots[idx].slots=slots; hubSnapHas[idx]=true; }
void HubProcessor::recallHubSnapshot(int idx){ if(idx<0||idx>=4||!hubSnapHas[idx]) return; slots=hubSnapshots[idx].slots; writeSharedFile(); }
void HubProcessor::getStateInformation(juce::MemoryBlock& d){
    auto state = apvts.copyState(); juce::ValueTree hub("HUB");
    for(int i=0;i<16;++i){ juce::ValueTree s("SLOT"); s.setProperty("id", slots[i].id, nullptr); s.setProperty("name", slots[i].name, nullptr); s.setProperty("x", slots[i].x, nullptr); s.setProperty("y", slots[i].y, nullptr); s.setProperty("room", slots[i].room, nullptr); s.setProperty("wet", slots[i].wet, nullptr); s.setProperty("col", (int)slots[i].color.getARGB(), nullptr); s.setProperty("active", slots[i].active, nullptr); hub.appendChild(s,nullptr); }
    for(int i=0;i<4;++i) if(hubSnapHas[i]){ juce::ValueTree hs("HUB_SNAP"); hs.setProperty("idx", i, nullptr); juce::ValueTree arr("SLOTS"); for(int j=0;j<16;++j){ juce::ValueTree s("S"); s.setProperty("x", hubSnapshots[i].slots[j].x, nullptr); s.setProperty("y", hubSnapshots[i].slots[j].y, nullptr); arr.appendChild(s,nullptr);} hs.appendChild(arr,nullptr); hub.appendChild(hs,nullptr); }
    state.appendChild(hub,nullptr); std::unique_ptr<juce::XmlElement> xml(state.createXml()); copyXmlToBinary(*xml,d);
}
void HubProcessor::setStateInformation(const void* data, int size){
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data,size)); if(!xml) return; auto tree=juce::ValueTree::fromXml(*xml); auto hub=tree.getChildWithName("HUB"); if(!hub.isValid()){apvts.replaceState(tree); return;}
    int idx=0; for(auto c: hub){ if(c.hasType("SLOT") && idx<16){ slots[idx].id=c.getProperty("id").toString(); slots[idx].name=c.getProperty("name").toString(); slots[idx].x=c.getProperty("x"); slots[idx].y=c.getProperty("y"); slots[idx].color=juce::Colour((juce::uint32)(int)c.getProperty("col")); slots[idx].active=c.getProperty("active"); idx++; } if(c.hasType("HUB_SNAP")){ int si=c.getProperty("idx"); auto arr=c.getChildWithName("SLOTS"); int j=0; for(auto s: arr){ if(j<16){ hubSnapshots[si].slots[j].x=s.getProperty("x"); hubSnapshots[si].slots[j].y=s.getProperty("y"); j++; } } hubSnapHas[si]=true; } } tree.removeChild(hub,nullptr); apvts.replaceState(tree);
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){ return new HubProcessor(); }
