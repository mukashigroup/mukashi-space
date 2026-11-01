
#pragma once
#include "SpaceProcessor.h"

struct SpacePad : public juce::Component {
    SpacePad(SpaceProcessor& p) : proc(p) {
        xParam = proc.apvts.getParameter("X");
        yParam = proc.apvts.getParameter("Y");
    }
    void paint(juce::Graphics& g) override {
        auto b = getLocalBounds().toFloat();
        // комната - градиент глубины
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff151515), b.getCentreX(), b.getY(),
                                               juce::Colour(0xff2e2e2e), b.getCentreX(), b.getBottom(), false));
        g.fillRect(b);
        // сетка
        g.setColour(juce::Colours::white.withAlpha(0.07f));
        for(int i=1;i<4;++i){ float y=b.getY()+b.getHeight()*(i/4.f); g.drawLine(b.getX(), y, b.getRight(), y, 1.f); }
        g.drawLine(b.getCentreX(), b.getY(), b.getCentreX(), b.getBottom(), 1.f);
        g.setColour(juce::Colours::white.withAlpha(0.25f)); g.setFont(10.f);
        g.drawText("CLOSE", b.getX()+8, b.getY()+4, 60, 12, juce::Justification::left);
        g.drawText("FAR", b.getX()+8, b.getBottom()-16, 60, 12, juce::Justification::left);
        g.drawText("L", b.getX()+4, b.getCentreY()-10, 20, 20, juce::Justification::centred);
        g.drawText("R", b.getRight()-24, b.getCentreY()-10, 20, 20, juce::Justification::centred);
        if (lockAxis==1) { g.setColour(juce::Colours::yellow.withAlpha(0.12f)); g.fillRect(b.getX(), b.getCentreY()-1, b.getWidth(), 2); }
        if (lockAxis==2) { g.setColour(juce::Colours::cyan.withAlpha(0.12f)); g.fillRect(b.getCentreX()-1, b.getY(), 2, b.getHeight()); }

        float xN = xParam->getValue(); float yN = yParam->getValue();
        float x = juce::jmap(xN, 0.f,1.f, b.getX()+20, b.getRight()-20);
        float y = juce::jmap(yN, 0.f,1.f, b.getY()+24, b.getBottom()-24);
        float sz = juce::jmap(yN, 0.f,1.f, 32.f, 14.f);
        // halo
        g.setColour(juce::Colours::orange.withAlpha(0.25f)); g.fillEllipse(x-sz, y-sz, sz*2, sz*2);
        g.setColour(juce::Colours::orange); g.fillEllipse(x-sz*0.6f, y-sz*0.6f, sz*1.2f, sz*1.2f);
        g.setColour(juce::Colours::white); g.setFont(11.f); g.drawText("SRC", x-16, y-28, 32, 14, juce::Justification::centred);
    }
    void mouseDown(const juce::MouseEvent& e) override {
        startX = e.x; startY = e.y;
        startXVal = xParam->getValue(); startYVal = yParam->getValue();
        updateLock(e);
        mouseDrag(e);
    }
    void mouseDrag(const juce::MouseEvent& e) override {
        updateLock(e);
        auto b = getLocalBounds().toFloat();
        float x = juce::jmap((float)e.x, b.getX()+20, b.getRight()-20, 0.f, 1.f);
        float y = juce::jmap((float)e.y, b.getY()+24, b.getBottom()-24, 0.f, 1.f);
        x = juce::jlimit(0.f,1.f,x); y = juce::jlimit(0.f,1.f,y);
        if (lockAxis==1) x = xParam->getValue(); // lock horizontal = only Y
        if (lockAxis==2) y = yParam->getValue(); // lock vertical = only X
        xParam->setValueNotifyingHost(x);
        yParam->setValueNotifyingHost(y);
        repaint();
    }
    void mouseUp(const juce::MouseEvent&) override { lockAxis=0; repaint(); }
    void updateLock(const juce::MouseEvent& e){
        if (e.mods.isShiftDown()){
            int dx = std::abs(e.x - startX);
            int dy = std::abs(e.y - startY);
            if (dx>6 || dy>6){
                if (lockAxis==0) lockAxis = (dx>dy)?2:1; // 2=lock Y (move X only), 1=lock X
            }
        } else { if (e.mods.isShiftDown()==false && (std::abs(e.x-startX)<3 && std::abs(e.y-startY)<3)) lockAxis=0; }
        // если шифт зажат с самого начала - ждем движения
        if (e.mods.isShiftDown() && lockAxis==0){
            // пока не определились - не лочим
        }
        if (!e.mods.isShiftDown()) lockAxis=0;
    }
    SpaceProcessor& proc;
    juce::AudioProcessorParameter *xParam, *yParam;
    int startX=0, startY=0;
    float startXVal=0, startYVal=0;
    int lockAxis=0; //0 none, 1 lock X (vertical move), 2 lock Y (horizontal)
};

struct SnapshotStrip : public juce::Component {
    SnapshotStrip(SpaceProcessor& p) : proc(p) {
        for(int i=0;i<proc.numSlots;++i){
            auto* b = new juce::TextButton(juce::String::charToString('A'+i));
            b->setClickingTogglesState(false);
            b->onClick = [this, i]{ proc.recallSnapshot(i); if(onRecall) onRecall(); };
            // right click / alt+click = save
            addAndMakeVisible(b);
            buttons.add(b);
            // контекстное меню для сохранения
            b->setWantsKeyboardFocus(false);
        }
        saveLabel.setText("ALT+CLICK = SAVE  |  SHIFT+DRAG = LOCK AXIS", juce::dontSendNotification);
        saveLabel.setFont(juce::Font(10.f)); saveLabel.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.35f));
        addAndMakeVisible(saveLabel);
    }
    void resized() override {
        auto r = getLocalBounds(); saveLabel.setBounds(r.removeFromRight(240));
        int w=36; for(int i=0;i<buttons.size();++i) buttons[i]->setBounds(r.removeFromLeft(w+4).reduced(2));
    }
    void paint(juce::Graphics& g) override {
        for(int i=0;i<buttons.size();++i){
            if(proc.snapshotHasData[i]){
                g.setColour(juce::Colours::orange.withAlpha(0.15f));
                // подсветка если есть данные
            }
        }
    }
    void mouseDown(const juce::MouseEvent& e) override {
        // проверяем alt для сохранения
        if (e.mods.isAltDown()){
            for(int i=0;i<buttons.size();++i){
                if(buttons[i]->getBounds().contains(e.getPosition())){
                    proc.saveSnapshot(i); repaint(); return;
                }
            }
        }
    }
    juce::OwnedArray<juce::TextButton> buttons;
    juce::Label saveLabel;
    SpaceProcessor& proc;
    std::function<void()> onRecall;
};

struct SpaceEditor : public juce::AudioProcessorEditor, public juce::Timer {
    SpaceEditor(SpaceProcessor& p) : AudioProcessorEditor(p), proc(p), pad(p), strip(p) {
        setSize(640, 520);
        addAndMakeVisible(pad);
        addAndMakeVisible(strip);
        strip.onRecall = [this]{ pad.repaint(); };
        // ловим alt+click на кнопках снапшотов
        for(auto* b: strip.buttons){
            b->addMouseListener(this, true);
        }
        startTimerHz(22);
    }
    void resized() override {
        auto r = getLocalBounds();
        strip.setBounds(r.removeFromBottom(32));
        pad.setBounds(r);
    }
    void timerCallback() override { pad.repaint(); }
    void mouseDown(const juce::MouseEvent& e) override {
        if (e.mods.isAltDown()){
            for(int i=0;i<strip.buttons.size();++i){
                if(strip.buttons[i]->getBounds().contains(e.getPosition() + strip.getPosition())){
                    proc.saveSnapshot(i); strip.repaint(); return;
                }
            }
        }
        // проброс в дочерние
        juce::AudioProcessorEditor::mouseDown(e);
    }
    SpaceProcessor& proc;
    SpacePad pad;
    SnapshotStrip strip;
};
