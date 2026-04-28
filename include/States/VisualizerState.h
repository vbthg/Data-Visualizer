#pragma once
#include "State.h"
#include "DataStructure.h"
#include "FloatingDock.h"
#include "InputPopover.h"
#include "DynamicIsland.h"
#include "PseudoCodeBox.h"
#include "StructurePanel.h"
#include "Slider.h"
#include "HistoryBoard.h"
#include "TimelineManager.h"
#include <stack>
#include <memory> // Thư viện cho smart pointers

class VisualizerState : public State
{
private:
    sf::RenderWindow& window;
    std::stack<State*>& states;
    sf::Font* font = nullptr;
    sf::Font* m_iconFont = nullptr;
    sf::Font* m_textFont = nullptr;
    sf::Font* m_codeFont = nullptr;

    sf::View m_camera;
    bool m_isDragging = false;
    sf::Vector2i m_lastMousePos;

    sf::Texture m_bgOriginal;

    // --- DATA ---
    // Dùng unique_ptr để quản lý vòng đời của CTDL truyền vào
    std::unique_ptr<DS::DataStructure> currentDS = nullptr;

    // --- UI COMPONENTS (Dùng unique_ptr để chống crash) ---
    std::unique_ptr<GUI::FloatingDock> dock = nullptr;
    std::unique_ptr<GUI::InputPopover> popover = nullptr;
    std::unique_ptr<GUI::DynamicIsland> notch = nullptr;
    std::unique_ptr<GUI::PseudoCodeBox> pseudoBox = nullptr;
    std::unique_ptr<GUI::Slider> testSlider = nullptr;
    std::unique_ptr<GUI::StructurePanel> structurePanel = nullptr;
    std::unique_ptr<GUI::HistoryBoard> m_historyBoard = nullptr;
    std::unique_ptr<Core::TimelineManager> m_timeline = nullptr;
    std::unique_ptr<GUI::Button> m_btnBack = nullptr;

    GUI::Button* m_btnPlayPause = nullptr; // Thêm dòng này để quản lý nút

    int m_lastSyncIdx = -1;
    int m_lastSnapshotCount = -1;

    void onResize(unsigned int width, unsigned int height);

public:
    VisualizerState(sf::RenderWindow& win, std::stack<State*>& st, std::unique_ptr<DS::DataStructure> dataStructure);
    virtual ~VisualizerState(); // Destructor giờ đây cực kỳ nhàn hạ

    void init() override;
    void handleInput(sf::Event& event) override;
    void update(float dt) override;
    void draw() override;
    void onEnter() override;
    void onExit() override;
};
