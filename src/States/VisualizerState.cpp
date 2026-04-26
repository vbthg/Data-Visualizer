#include "VisualizerState.h"
#include "ResourceManager.h"
#include "Theme.h"
#include "SpeedController.h"
#include "ViewHandler.h"
#include "WindowConfig.h"
#include "NotchManager.h"
#include "FileUtils.h"
#include "FileDropManager.h"
//#include "drawGlassPane.h"
#include <iostream>
#include <algorithm> // Bổ sung thư viện cho std::find_if

namespace Theme = Utils::Graphics::Theme;

VisualizerState::VisualizerState(sf::RenderWindow& win, std::stack<State*>& st, DS::DataStructure* ds)
    : window(win), states(st), currentDS(ds), m_lastSyncIdx(-1), m_isDragging(false),
      font(ResourceManager::getInstance().getFont("assets/fonts/font.ttf"))
{
    // 1. Khởi tạo Dock
    std::cout << "1. Start Constructor" << std::endl;

    dock = new GUI::FloatingDock();
    popover = new GUI::InputPopover(font);
    notch = new GUI::DynamicIsland(font);
    pseudoBox = new GUI::PseudoCodeBox(ResourceManager::getInstance().getFont("assets/fonts/consolas-regular.ttf"), window.getSize().x, window.getSize().y);

    m_timeline = new Core::TimelineManager();
    std::cout << "2. Timeline Created" << std::endl;
    m_historyBoard = new GUI::HistoryBoard();
    std::cout << "3. HistoryBoard Created" << std::endl;

    m_timeline->syncCodeBox(pseudoBox);

    // Sửa lỗi lambda capture tham chiếu rác, chuyển từ [&] sang [this]
    m_historyBoard->setOnJumpCallback([this](int targetIdx) {
        m_timeline->seek(targetIdx);
        m_timeline->pause();
    });

    std::cout << "4. About to sync Timeline" << std::endl;
    if (m_timeline && m_historyBoard)
        m_historyBoard->syncWithManager(*m_timeline);






    // Nạp thử code test
    std::vector<std::string> testCode = {
        "Node* temp = root;",
        "while (temp != nullptr)",
        "{",
        "    if (value < temp->data)",
        "        temp = temp->left;",
        "    else",
        "        temp = temp->right;",
        "}"
    };
    pseudoBox->loadCode("BST::Insert", testCode);

    // 2. Thêm nhóm Navigation (Điều hướng)
    GUI::Button* btnPrev = new GUI::Button(font, "Back", {Theme::Style::IconButtonSize, Theme::Style::IconButtonSize});
    btnPrev->applyPreset(GUI::ButtonPreset::Ghost);
    btnPrev->onClick = [this]() { /* Logic lùi bước thuật toán */ };
    dock->addItem(btnPrev);

    GUI::Button* btnNext = new GUI::Button(font, "Next", {Theme::Style::IconButtonSize, Theme::Style::IconButtonSize});
    btnNext->applyPreset(GUI::ButtonPreset::Ghost);
    btnNext->onClick = [this]() { /* Logic tới bước tiếp theo */ };
    dock->addItem(btnNext);

    // 3. Thêm vạch ngăn (Separator)
    dock->addItem(new GUI::Separator(Theme::Style::DockHeight));

    // 4. Thêm nhóm Playback & Speed
    GUI::Button* btnPlay = new GUI::Button(font, "Play", {Theme::Style::IconButtonSize, Theme::Style::IconButtonSize});
    btnPlay->applyPreset(GUI::ButtonPreset::Ghost);
    btnPlay->onClick = [this]() { /* Logic Play/Pause */ };
    dock->addItem(btnPlay);

    // "Ngôi sao" của chúng ta: SpeedController
    GUI::SpeedController* speedCtrl = new GUI::SpeedController(font, m_timeline);
    dock->addItem(speedCtrl);

    // 5. Thêm vạch ngăn cuối
    dock->addItem(new GUI::Separator(Theme::Style::DockHeight));

    // 6. Nút chức năng phụ (VD: Clear/Reset)
    GUI::Button* btnReset = new GUI::Button(font, "Reset", {Theme::Style::IconButtonSize, Theme::Style::IconButtonSize});
    btnReset->applyPreset(GUI::ButtonPreset::Ghost);
    dock->addItem(btnReset);

    dock->updateLayout();


//std::cout << "OK!" << std::endl;
    // Nạp Commands từ cấu trúc dữ liệu
    if (currentDS)
    {
        std::cout << "[COMMANDS SIZE: ]" << (currentDS->getCommands()).size() << "\n";
        dock->setCommands(currentDS->getCommands(), ResourceManager::getInstance().getFont("assets/fonts/Phosphor.ttf"));

        dock->addItem(new GUI::Separator(Theme::Style::DockHeight));
        GUI::SpeedController* speedCtrl = new GUI::SpeedController(font, m_timeline);
        dock->addItem(speedCtrl);
    }

    // 3. Bơm (Inject) Timeline vào cấu trúc dữ liệu
    if(currentDS)
    {
        currentDS->setTimelineManager(m_timeline);
    }



    // NỐI DÂY (WIRING):
    dock->onCommandClicked = [this](sf::Vector2f btnPos, DS::Command cmd)
    {
        if (!popover->isOpen())
        {
            popover->setInputType(cmd.inputType);
            popover->open(btnPos, cmd);
        }
    };



//    if (currentDS)
//    {
//        currentDS->bindDynamicIsland(notch);
//    }

    // 3. Setup Layout ban đầu
    dock->snapToY(window.getSize().y + 100.0f);
    notch->snapToY(-100.0f);

    m_camera.setSize(Utils::System::DESIGN_WIDTH, Utils::System::DESIGN_HEIGHT);
    m_camera.setCenter(Utils::System::DESIGN_WIDTH / 2.f, Utils::System::DESIGN_HEIGHT / 2.f);

    testSlider = new GUI::Slider(200.f);
    testSlider->setRange(0.25f, 2.0f, 0.25f);
    testSlider->setValue(1.0f);
    testSlider->setPosition(sf::Vector2f(400.f, 300.f));
    testSlider->setIcons(ResourceManager::getInstance().getTexture("assets/textures/tortoise.png"),
                         ResourceManager::getInstance().getTexture("assets/textures/hare.png"));

    structurePanel = new GUI::StructurePanel();
    bool isCodeOpen = pseudoBox ? pseudoBox->isOpen() : false;
    structurePanel->initIntro(1920.f, 1080.f, isCodeOpen);

    m_bgOriginal = ResourceManager::getInstance().getTexture("assets/textures/macOS Big Sur.png");

    if (dock)
    {
        dock->setBlurTexture(ResourceManager::getInstance().getTexture("assets/textures/macOS Big Sur - Blur 25.png"), sf::Vector2f(1920.f, 1080.f));
    }

    GUI::NotchManager::getInstance().pushNotification(GUI::Scenario::Initial);
    onResize(window.getSize().x, window.getSize().y);

    structurePanel->syncTimeline(m_timeline);
    structurePanel->syncDataStructure(currentDS);

    std::cout << "5. End Constructor" << std::endl;



//    return;
}

VisualizerState::~VisualizerState()
{
    if (dock) delete dock;
    if (popover) delete popover;
    if (currentDS) delete currentDS;
    if (notch) delete notch;
    if (structurePanel) delete structurePanel;
    if (m_historyBoard) delete m_historyBoard;
    if (m_timeline) delete m_timeline;
    // Bổ sung các con trỏ bị thiếu gây memory leak
    if (pseudoBox) delete pseudoBox;
    if (testSlider) delete testSlider;
}

void VisualizerState::onEnter()
{
    GUI::NotchManager::getInstance().pushNotification(GUI::Scenario::Idle);
}

void VisualizerState::onExit()
{
    GUI::NotchManager::getInstance().pushNotification(GUI::Scenario::Idle);
}

void VisualizerState::onResize(unsigned int width, unsigned int height)
{
//    std::cout << "RESIZE TRIGGERED: " << width << "x" << height << std::endl;
    Utils::System::updateCustomView(m_camera, width, height);

    float centerX = Utils::System::DESIGN_WIDTH / 2.0f;
    float dockHeight = 70.0f;
    float targetY = Utils::System::DESIGN_HEIGHT - dockHeight - 40.0f;

    dock->setPosition(centerX, dock->getPosition().y);
    dock->setTargetY(targetY);

    if(notch)
    {
        notch->setX(centerX);
        notch->setTargetY(40.f);
    }

    if(pseudoBox)
    {
//        pseudoBox->onResize(Utils::System::DESIGN_WIDTH, Utils::System::DESIGN_HEIGHT);
    }
}

void VisualizerState::updateNotchLogic()
{
    static int lastIdx = -1;
    int currentIdx = m_timeline->getCurrentIdx();

    // Nếu bước hiện tại của Timeline thay đổi
    if (currentIdx != lastIdx) {
        auto snap = m_timeline->getSnapshot(currentIdx);
        if (snap) {
            // Bắn log tiếng Anh lên Notch Manager
            GUI::NotchManager::getInstance().pushNotification(
                snap->scenario,
                snap->logMessage,
                "",
                ""
            );
        }
        lastIdx = currentIdx;
    }
}

void VisualizerState::handleInput(sf::Event& event)
{
    if(event.type == sf::Event::Resized)
    {
        Utils::System::updateCustomView(m_camera, event.size.width, event.size.height);
        // Resize thường là sự kiện hệ thống, không nên chặn
    }

    // 1. Ưu tiên cao nhất: Các cửa sổ nổi (Popover/Dialog)
    if(popover && popover->isOpen())
    {
        if(popover->handleEvent(event, window)) return;
    }

    // 2. Tiếp theo: History Board (Vì nó nằm đè lên Panel)
    // Giả sử em sửa handleEvent của HistoryBoard trả về bool
    int targetIdx = m_historyBoard->handleEvent(event, window);
    if(targetIdx != -1)
    {
        // Nếu click trúng một node trong lịch sử, ta dừng luôn
        m_timeline->seek(targetIdx);
        m_timeline->pause();
        return;
    }

    // 3. Các thành phần UI cố định (Dock, PseudoBox)
    if(dock && dock->handleEvent(event, window)) return;

    if(pseudoBox && pseudoBox->handleEvent(event, window)) return;

    // 4. Phím tắt toàn cục (Global Shortcuts)
    if(event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::C)
    {
        if(pseudoBox)
        {
            pseudoBox->toggleState();
            structurePanel->updateLayout(1920.f, 1080.f, pseudoBox->isOpen());
        }
        return;
    }

    // 5. Lớp dưới cùng: StructurePanel (Nơi vẽ đồ thị)
    // Panel chỉ nhận sự kiện khi tất cả UI phía trên đã từ chối
    if(structurePanel)
    {
        structurePanel->handleEvent(event, window);
    }
}

//void VisualizerState::handleInput(sf::Event& event)
//{
////    return;
//
//    if(event.type == sf::Event::Resized)
//    {
//        Utils::System::updateCustomView(m_camera, event.size.width, event.size.height);
//    }
////
//    int targetIdx = m_historyBoard->handleEvent(event, window);
//    if (targetIdx != -1)
//    {
//        m_timeline->seek(targetIdx);
//        m_timeline->pause();
//    }
////
//    if(popover && popover->isOpen())
//    {
//        popover->handleEvent(event, window);
//    }
//    else
//    {
//        if(dock) dock->handleEvent(event, window);
//    }
//
//    if(pseudoBox)
//    {
//        pseudoBox->handleEvent(event, window);
//    }
//
//    if(event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::C)
//    {
//        if(pseudoBox)
//        {
//            pseudoBox->toggleState();
//            bool isCodeOpen = pseudoBox ? pseudoBox->isOpen() : false;
//            structurePanel->updateLayout(1920.f, 1080.f, isCodeOpen);
//        }
//
////        this->createMockTest();
//    }
//
////    static int testLine = 0;
////    if(event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Down)
////    {
////        testLine++;
////        if(pseudoBox)
////        {
////            pseudoBox->updateStep(testLine, {
////                {"cur", "Node(" + std::to_string(testLine * 10) + ")"},
////                {"min_idx", std::to_string(testLine)}
////            });
////        }
////    }
////
////    if(event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Up)
////    {
////        testLine = std::max(0, testLine - 1);
////        if(pseudoBox) pseudoBox->updateStep(testLine, {});
////    }
//
////    testSlider->handleEvent(event, window);
//    if(structurePanel) structurePanel->handleEvent(event, window);
//}

void VisualizerState::update(float dt)
{
//    return;


    m_timeline->update(dt);

    int currentIdx = m_timeline->getCurrentIdx();
    if (currentIdx != m_lastSyncIdx)
    {
        auto currentSnap = m_timeline->getSnapshot(currentIdx);
        if (currentSnap)
        {
            GUI::NotchManager::getInstance().pushNotification(
                currentSnap->scenario,
                currentSnap->operationName,
                currentSnap->logMessage,
                ""
            );

            GUI::NotchManager::getInstance().updateStep(
                m_timeline->getCurrentIdx(),
                m_timeline->getCount(),
                1.f / m_timeline->getPlaybackSpeed()
            );
        }
        m_lastSyncIdx = currentIdx;
    }

    if (m_historyBoard)
    {
        m_historyBoard->update(dt, window, m_timeline->getCurrentIdx());
    }

    if (currentDS && structurePanel)
    {
        // 1. Lấy frame hiện tại
        Core::RenderFrame currentFrame = m_timeline->getCurrentFrame();

        // 2. Cập nhật lò xo camera của Panel
        structurePanel->update(dt, window);

        // 3. ĐỒNG BỘ LOGIC: Tính toán vị trí, vận tốc, lò xo của Node/Edge
        structurePanel->syncGraphObjects(currentFrame, dt);
    }

    if (dock) dock->update(dt, window);
    if (popover) popover->update(dt, window);
    if (pseudoBox) pseudoBox->update(dt);

    GUI::NotchManager::getInstance().update(dt);
    GUI::NotchManager::getInstance().updateMousePos(sf::Mouse::getPosition(window), window);
}

void VisualizerState::draw()
{
//    return;

    window.clear();
    window.draw(sf::Sprite(m_bgOriginal));

    if (m_timeline->getCount() > 0)
    {
        Core::RenderFrame frame = m_timeline->getCurrentFrame();

        if (structurePanel)
        {
            // CHỈ VẼ: Áp những trạng thái đã tính toán lên buffer
            structurePanel->renderContent();
        }
    }

    if (structurePanel) window.draw(*structurePanel);
    if (m_historyBoard) window.draw(*m_historyBoard);
    if (pseudoBox) pseudoBox->draw(window);
    if (dock) dock -> draw(window);
    if (popover) popover -> draw(window);
    window.draw(GUI::NotchManager::getInstance());

//    window.display();
}

//void VisualizerState::draw()
//{
//    // 1. Clear một màu cố định (VD: Màu xám đậm)
//    window.clear(sf::Color(30, 30, 30));
//
//    // 2. Vẽ duy nhất cái background (Tạm thời ẩn hết bọn khác)
////    window.draw(sf::Sprite(m_bgOriginal));
//
//    // 3. Display một lần duy nhất
////    window.display();
//}

void VisualizerState::renderMock(sf::RenderTarget& target, const Core::RenderFrame& frame)
{
//    return;

    for (const auto& edge : frame.edges)
    {
        auto itStart = std::find_if(frame.nodes.begin(), frame.nodes.end(), [&](const auto& n){ return n.id == edge.startNodeId; });
        auto itEnd = std::find_if(frame.nodes.begin(), frame.nodes.end(), [&](const auto& n){ return n.id == edge.endNodeId; });

        if (itStart != frame.nodes.end() && itEnd != frame.nodes.end())
        {
            sf::Vertex line[] = {
                sf::Vertex(itStart->position, edge.fillColor),
                sf::Vertex(itEnd->position, edge.fillColor)
            };
            line[0].color.a = static_cast<sf::Uint8>(edge.opacity);
            line[1].color.a = static_cast<sf::Uint8>(edge.opacity);

            target.draw(line, 2, sf::Lines);
        }
    }

    for (const auto& node : frame.nodes)
    {
        float radius = 30.f * node.scale;
        sf::CircleShape shape(radius);
        shape.setOrigin(radius, radius);
        shape.setPosition(node.position);

        sf::Color fColor = node.fillColor;
        fColor.a = static_cast<sf::Uint8>(node.opacity);
        shape.setFillColor(fColor);

        shape.setOutlineThickness(2.f * node.scale);
        sf::Color oColor = node.outlineColor;
        oColor.a = static_cast<sf::Uint8>(node.opacity);
        shape.setOutlineColor(oColor);

        target.draw(shape);

        sf::Text text;
        text.setFont(font);
        text.setString(node.value);
        text.setCharacterSize(static_cast<unsigned int>(24 * node.scale));
        text.setFillColor(sf::Color::Black);

        sf::FloatRect textRect = text.getLocalBounds();
        text.setOrigin(textRect.left + textRect.width/2.0f, textRect.top + textRect.height/2.0f);
        text.setPosition(node.position);

        target.draw(text);
    }
}

void VisualizerState::createMockTest()
{
    m_timeline->clear();

    auto s1 = std::make_shared<Core::ISnapshot>();
    s1->operationName = "Heap Test";
    s1->logMessage = "Initial state: Node 1 and 2 are far apart.";
    s1->scenario = GUI::Scenario::Processing;

    Core::NodeState n1, n2;
    n1.id = 1; n1.position = {200, 300}; n1.value = "10";
    n2.id = 2; n2.position = {800, 300}; n2.value = "20";
    s1->nodeStates = {n1, n2};

    Core::EdgeState e1;
    e1.startNodeId = 1; e1.endNodeId = 2; e1.fillColor = sf::Color::White;
    s1->edgeStates = {e1};
    m_timeline->addSnapshot(s1);

    auto s2 = std::make_shared<Core::ISnapshot>();
    s2->operationName = "Heap Test";
    s2->logMessage = "Nodes are moving closer and changing color!";
    s2->scenario = GUI::Scenario::Success;

    n1.position = {450, 300}; n1.fillColor = sf::Color::Green;
    n2.position = {550, 300}; n2.fillColor = sf::Color::Cyan;
    s2->nodeStates = {n1, n2};
    s2->edgeStates = {e1};

    m_timeline->addSnapshot(s2);

    if (m_timeline != nullptr)
    {
        m_historyBoard->syncWithManager(*m_timeline);
    }
    m_timeline->seek(0); // Quay về bước 1
    // 2. ÉP Notch hiện log của Snapshot 0 ngay lập tức
    auto firstSnap = m_timeline->getSnapshot(0);
    if (firstSnap)
    {
        // Giả sử NotchManager của ông có hàm pushNotification nhận string và scenario
        GUI::NotchManager::getInstance().pushNotification(firstSnap->scenario, firstSnap->logMessage, "", "");
    }
    m_timeline->play();
}

void VisualizerState::init() {}
