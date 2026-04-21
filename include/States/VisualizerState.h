// VisualizerState.h
#pragma once
#include "State.h"
#include "DataStructure.h"
#include "FloatingDock.h"
#include "InputPopover.h"
#include "DynamicIsland.h"
#include "PseudoCodeBox.h"
#include "StructurePanel.h"
#include "Slider.h"
#include "Spring.h"
#include "HistoryBoard.h"
#include "TimelineManager.h"
#include <stack>

// Giả sử bạn sẽ tạo hoặc đã có class này để vẽ hình bo góc
// Nếu chưa có, hãy báo tôi để tôi cung cấp code class này.
class Squircle;

class VisualizerState : public State
{
private:
    sf::RenderWindow& window;
    std::stack<State*>& states;
    sf::Font& font;

    sf::View m_camera;
    bool m_isDragging;
    sf::Vector2i m_lastMousePos;

    // --- NEW: LAYERED BACKGROUNDS (Kính mờ) ---
    sf::Texture m_bgOriginal;  // Ảnh gốc
//    sf::Texture m_bgBlurLevel1; // Mờ 10px cho CTDL
//    sf::Texture m_bgBlurLevel2; // Mờ 25px cho Dock/Island
//    sf::Texture m_bgBlurLevel3; // Mờ 50px cho CodeBox

    // --- DATA ---
    DS::DataStructure* currentDS;

    // --- UI Components ---
    GUI::FloatingDock* dock;
    GUI::InputPopover* popover;
    GUI::DynamicIsland* notch;
    GUI::PseudoCodeBox* pseudoBox;
    GUI::Slider* testSlider;
    GUI::StructurePanel* structurePanel;
    GUI::HistoryBoard* m_historyBoard;

    int m_lastSyncIdx = -1; // Khởi tạo là -1 để nó đồng bộ ngay lần đầu tiên

    Core::TimelineManager* m_timeline;

    void onResize(unsigned int width, unsigned int height);

    void createMockTest();
    void renderMock(sf::RenderTarget& target, const Core::RenderFrame& frame);

    void updateNotchLogic();

public:
    VisualizerState(sf::RenderWindow& win, std::stack<State*>& st, DS::DataStructure* ds);
    virtual ~VisualizerState();

    void init() override;
    void handleInput(sf::Event& event) override;
    void update(float dt) override;
    void draw() override;
    void onEnter() override;
    void onExit() override;


};
