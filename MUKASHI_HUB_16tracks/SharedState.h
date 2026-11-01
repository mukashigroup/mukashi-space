
#pragma once
#include <JuceHeader.h>

struct TrackSlot {
    juce::String id = "track_0";
    juce::String name = "Drums";
    float x = 0.f; // -100..100
    float y = 0.f; // 0..100 close->far
    float room = 40.f;
    float wet = 20.f;
    juce::Colour color = juce::Colours::orange;
    bool active = true;
    juce::int64 lastUpdate = 0;
};

inline juce::File getSharedFile() {
    return juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("mukashi_space_v3.json");
}

inline juce::var slotsToJson(const std::array<TrackSlot,16>& slots) {
    juce::DynamicObject::Ptr obj = new juce::DynamicObject();
    juce::Array<juce::var> arr;
    for (auto& s: slots) {
        juce::DynamicObject::Ptr o = new juce::DynamicObject();
        o->setProperty("id", s.id); o->setProperty("name", s.name);
        o->setProperty("x", s.x); o->setProperty("y", s.y);
        o->setProperty("room", s.room); o->setProperty("wet", s.wet);
        o->setProperty("color", (int)s.color.getARGB()); o->setProperty("active", s.active);
        o->setProperty("lastUpdate", s.lastUpdate);
        arr.add(juce::var(o.get()));
    }
    obj->setProperty("slots", arr);
    return juce::var(obj.get());
}
inline void jsonToSlots(const juce::var& v, std::array<TrackSlot,16>& slots) {
    if (!v.isObject()) return;
    auto* obj = v.getDynamicObject();
    auto arr = obj->getProperty("slots");
    if (!arr.isArray()) return;
    for (int i=0;i<juce::jmin(arr.size(), 16); ++i){
        auto* o = arr[i].getDynamicObject(); if(!o) continue;
        slots[i].id = o->getProperty("id").toString();
        slots[i].name = o->getProperty("name").toString();
        slots[i].x = (float)o->getProperty("x"); slots[i].y = (float)o->getProperty("y");
        slots[i].room = (float)o->getProperty("room"); slots[i].wet = (float)o->getProperty("wet");
        slots[i].color = juce::Colour((juce::uint32)(int)o->getProperty("color"));
        slots[i].active = (bool)o->getProperty("active");
        slots[i].lastUpdate = (juce::int64)o->getProperty("lastUpdate");
    }
}
