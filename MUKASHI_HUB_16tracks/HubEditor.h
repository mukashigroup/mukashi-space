
#pragma once
#include "HubProcessor.h"

struct HubPad : public juce::Component, public juce::Timer {
    HubPad(HubProcessor& p) : proc(p) { startTimerHz(12); selectedIdx=0; }
    void paint(juce::Graphics& g) override {
        auto b=getLocalBounds().toFloat();
        // room bg
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff121212), b.getCentreX(), b.getY(), juce::Colour(0xff2b2b2b), b.getCentreX(), b.getBottom(), false)); g.fillRect(b);
        // grid
        g.setColour(juce::Colours::white.withAlpha(0.06f)); for(int i=1;i<4;++i){ float y=b.getY()+b.getHeight()*(i/4.f); g.drawLine(b.getX(), y, b.getRight(), y, 1.f);} g.drawLine(b.getCentreX(), b.getY(), b.getCentreX(), b.getBottom(), 1.f);
        g.setColour(juce::Colours::white.withAlpha(0.2f)); g.setFont(10.f); g.drawText("CLOSE / FRONT", b.getX()+8, b.getY()+4, 100,12, juce::Justification::left); g.drawText("FAR / BACK + REVERB", b.getX()+8, b.getBottom()-16, 140,12, juce::Justification::left);
        // draw slots
        for(int i=0;i<16;++i){ auto& s=proc.slots[i]; if(!s.active) continue;
            float xN = juce::jmap(s.x, -100.f, 100.f, 0.f, 1.f); float yN = juce::jmap(s.y, 0.f, 100.f, 0.f, 1.f);
            float x = juce::jmap(xN, 0.f,1.f, b.getX()+14, b.getRight()-14); float y = juce::jmap(yN, 0.f,1.f, b.getY()+18, b.getBottom()-18);
            float sz = juce::jmap(yN, 0.f,1.f, 22.f, 10.f); if(i==selectedIdx) sz+=6;
            // line to center
            if(i==selectedIdx){ g.setColour(s.color.withAlpha(0.15f)); g.drawLine(b.getCentreX(), b.getY()+20, x, y, 1.f); }
            // shadow by depth
            float alpha = juce::jmap(yN, 0.f,1.f, 1.f, 0.35f);
            g.setColour(s.color.withAlpha(0.25f*alpha)); g.fillEllipse(x-sz, y-sz, sz*2, sz*2);
            g.setColour(i==selectedIdx? juce::Colours::white : s.color); if(i==selectedIdx) g.drawEllipse(x-sz, y-sz, sz*2, sz*2, 2.f);
            g.setColour(s.color); g.fillEllipse(x-sz*0.6f, y-sz*0.6f, sz*1.2f, sz*1.2f);
            // label
            g.setColour(juce::Colours::white.withAlpha(i==selectedIdx?1.f:0.7f)); g.setFont(juce::Font(i==selectedIdx?12.f:10.f, juce::Font::bold)); g.drawText(s.name, x-30, y- sz -14, 60, 12, juce::Justification::centred);
        }
        if(lockAxis!=0){ g.setColour(lockAxis==1? juce::Colours::yellow.withAlpha(0.12f): juce::Colours::cyan.withAlpha(0.12f)); if(lockAxis==1) g.fillRect(b.getX(), 0.f, b.getWidth(), 2.f); else g.fillRect(0.f, b.getY(), 2.f, b.getHeight()); }
    }
    void mouseDown(const juce::MouseEvent& e) override {
        // find closest
        auto b=getLocalBounds().toFloat(); int best=-1; float bestD=1e9;
        for(int i=0;i<16;++i){ float xN=juce::jmap(proc.slots[i].x,-100.f,100.f,0.f,1.f); float yN=juce::jmap(proc.slots[i].y,0.f,100.f,0.f,1.f); float x=juce::jmap(xN,0.f,1.f,b.getX()+14,b.getRight()-14); float y=juce::jmap(yN,0.f,1.f,b.getY()+18,b.getBottom()-18); float d=std::hypot(e.x-x, e.y-y); if(d<bestD && d<30){bestD=d; best=i;}}
        if(best>=0) selectedIdx=best;
        startX=e.x; startY=e.y; lockAxis=0; mouseDrag(e);
    }
    void mouseDrag(const juce::MouseEvent& e) override {
        if(e.mods.isShiftDown()){
            int dx=std::abs(e.x-startX); int dy=std::abs(e.y-startY); if(lockAxis==0 && (dx>6||dy>6)) lockAxis = (dx>dy)?2:1;
        } else lockAxis=0;
        auto b=getLocalBounds().toFloat(); float x = juce::jmap((float)e.x, b.getX()+14, b.getRight()-14, -100.f, 100.f); float y = juce::jmap((float)e.y, b.getY()+18, b.getBottom()-18, 0.f, 100.f);
        x=juce::jlimit(-100.f,100.f,x); y=juce::jlimit(0.f,100.f,y);
        auto& s=proc.slots[selectedIdx];
        if(lockAxis==1) x=s.x; if(lockAxis==2) y=s.y;
        s.x=x; s.y=y; s.lastUpdate=juce::Time::getMillisecondCounter();
        proc.writeSharedFile(); repaint();
    }
    void mouseUp(const juce::MouseEvent&) override { lockAxis=0; }
    void timerCallback() override { proc.readSharedFile(); repaint(); }
    HubProcessor& proc; int selectedIdx; int startX=0,startY=0; int lockAxis=0;
};

struct HubSnapshotStrip : public juce::Component {
    HubSnapshotStrip(HubProcessor& p):proc(p){ for(int i=0;i<4;++i){ auto* b=new juce::TextButton(juce::String::charToString('A'+i)); b->onClick=[this,i]{ proc.recallHubSnapshot(i); }; addAndMakeVisible(b); btns.add(b);} info.setText("ALT+CLICK SAVE | CLICK RECALL | SHIFT DRAG LOCK AXIS | 16 TRACKS", juce::dontSendNotification); info.setFont(juce::Font(10.f)); info.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.35f)); addAndMakeVisible(info);}
    void resized() override { auto r=getLocalBounds(); info.setBounds(r.removeFromRight(360)); int w=40; for(auto* b: btns) b->setBounds(r.removeFromLeft(w+4).reduced(2)); }
    void mouseDown(const juce::MouseEvent& e) override { if(e.mods.isAltDown()){ for(int i=0;i<btns.size();++i) if(btns[i]->getBounds().contains(e.getPosition())){ proc.saveHubSnapshot(i); repaint(); } } }
    HubProcessor& proc; juce::OwnedArray<juce::TextButton> btns; juce::Label info;
};

struct HubEditor : public juce::AudioProcessorEditor {
    HubEditor(HubProcessor& p):AudioProcessorEditor(p), proc(p), pad(p), strip(p){ setSize(900, 620); addAndMakeVisible(pad); addAndMakeVisible(strip); pad.setBounds(0,0,900,580); strip.setBounds(0,580,900,40); }
    void resized() override { auto r=getLocalBounds(); strip.setBounds(r.removeFromBottom(40)); pad.setBounds(r); }
    HubProcessor& proc; HubPad pad; HubSnapshotStrip strip;
};
