#include "VisualizerState.h"
#include "ResourceManager.h"
#include "Theme.h"
#include "SpeedController.h"
#include "ViewHandler.h"
#include "WindowConfig.h"
#include "NotchManager.h"
#include "NotchEnums.h"
#include <iostream>

namespace Theme = Utils::Graphics::Theme;

VisualizerState::VisualizerState(sf::RenderWindow& win, std::stack<State*>& st, std::unique_ptr<DS::DataStructure> dataStructure)
    : window(win), states(st), currentDS(std::move(dataStructure))
//      font(ResourceManager::getInstance().getFont("assets/fonts/SFProtext-Regular.ttf"))
{
    // Constructor chỉ nên để trống hoặc set các giá trị cơ bản
    // Tránh gọi các hàm logic phức tạp ở đây
}

VisualizerState::~VisualizerState()
{
    // Không cần delete bất kỳ component UI nào!
    // std::unique_ptr sẽ tự lo hết khi VisualizerState bị hủy.
}

void VisualizerState::init()
{
//    std::cerr << "[VisualizerState] Safe Init Started..." << std::endl;

    // 1. Lấy tài nguyên Font (Bắt buộc phải có)
    m_iconFont = &ResourceManager::getInstance().getFont("assets/fonts/Phosphor.ttf");
    m_textFont = &ResourceManager::getInstance().getFont("assets/fonts/SFProText-Regular.ttf");
    m_codeFont = &ResourceManager::getInstance().getFont("assets/fonts/consolas-regular.ttf");


    m_camera.setSize(Utils::System::DESIGN_WIDTH, Utils::System::DESIGN_HEIGHT);
    m_camera.setCenter(Utils::System::DESIGN_WIDTH / 2.f, Utils::System::DESIGN_HEIGHT / 2.f);


    // 4. KHỞI TẠO NÚT BACK RIÊNG BIỆT
    m_btnBack = std::make_unique<GUI::Button>(*m_iconFont, L"\uE138", sf::Vector2f(55, 55));
    m_btnBack->applyPreset(GUI::ButtonPreset::Ghost);
    m_btnBack->setCornerRadius(20.f);
    m_btnBack->setCharacterSize(24.f);
    m_btnBack->onClick = [this]()
    {
        this->states.pop();
    };

    // Đặt vị trí nút Back (Ví dụ: Góc trên bên trái, cách lề 30px)
    m_btnBack->setPosition({60.f, 1005.f});


    // 2. Khởi tạo UI (Dùng unique_ptr)
    m_timeline = std::make_unique<Core::TimelineManager>();
    dock = std::make_unique<GUI::FloatingDock>();
    popover = std::make_unique<GUI::InputPopover>(*m_textFont);
    notch = std::make_unique<GUI::DynamicIsland>(*m_textFont);
    m_historyBoard = std::make_unique<GUI::HistoryBoard>();
    structurePanel = std::make_unique<GUI::StructurePanel>();
    pseudoBox = std::make_unique<GUI::PseudoCodeBox>(*m_codeFont, 1920.f, 1080.f);

    bool isCodeOpen = pseudoBox ? pseudoBox->isOpen() : false;
    structurePanel->initIntro(Utils::System::DESIGN_WIDTH, Utils::System::DESIGN_HEIGHT, isCodeOpen);

    // 3. WIRING (Kết nối logic)
    if(m_timeline && pseudoBox) m_timeline->syncCodeBox(pseudoBox.get());
    if(structurePanel && m_timeline) structurePanel->syncTimeline(m_timeline.get());


    dock->onCommandClicked = [this](sf::Vector2f btnPos, DS::Command cmd)
    {
        if(!popover->isOpen())
        {
            popover->setInputType(cmd.inputType);
            popover->open(btnPos, cmd);
        }
    };

    // Nút Lịch sử
    auto* btnHistory = new GUI::Button(*m_iconFont, L"\uE19E", {Theme::Style::IconButtonSize, Theme::Style::IconButtonSize});
    btnHistory->applyPreset(GUI::ButtonPreset::Ghost);
    btnHistory->setCharacterSize(25);
    btnHistory->onClick = [this](){ if(m_historyBoard) m_historyBoard->toggle(); };
    dock->addItem(btnHistory, "History");

    // Các nút Macro
    auto* btnMPrev = new GUI::Button(*m_iconFont, L"\uE5A4", {Theme::Style::IconButtonSize, Theme::Style::IconButtonSize});
    btnMPrev->applyPreset(GUI::ButtonPreset::Ghost);
    btnMPrev->setCharacterSize(25);
    btnMPrev->onClick = [this](){ if(m_timeline) m_timeline->jumpToPreviousMacro(); };
    dock->addItem(btnMPrev, "Backward");

    // Tạo nút Play/Pause
    m_btnPlayPause = new GUI::Button(*m_iconFont, L"\uE6DE", {Theme::Style::IconButtonSize, Theme::Style::IconButtonSize});
    m_btnPlayPause->setCharacterSize(25);
    m_btnPlayPause->applyPreset(GUI::ButtonPreset::Ghost);

    m_btnPlayPause->onClick = [this]()
    {
        if(m_timeline->isPlaying())
        {
            m_timeline->pause();
        }
        else
        {
            m_timeline->play();
        }
    };

    dock->addItem(m_btnPlayPause, "Resume");

    auto* btnMNext = new GUI::Button(*m_iconFont, L"\uE5A6", {Theme::Style::IconButtonSize, Theme::Style::IconButtonSize});
    btnMNext->applyPreset(GUI::ButtonPreset::Ghost);
    btnMNext->setCharacterSize(25);
    btnMNext->onClick = [this](){ if(m_timeline) m_timeline->jumpToNextMacro(); };
    dock->addItem(btnMNext, "Forward");


    // Sau đó mới thêm các nút điều hướng "bất tử" vào Dock
    dock->addItem(new GUI::Separator(Theme::Style::DockHeight));

    // 4. SETUP DOCK - QUAN TRỌNG: Gọi setCommands trước tiên
    if(currentDS)
    {
        currentDS->setTimelineManager(m_timeline.get());
        structurePanel->syncDataStructure(currentDS.get());

        // Load các nút từ thuật toán trước
        dock->setCommands(currentDS->getCommands(), *m_iconFont);
    }


    // 5. Cấu hình HistoryBoard
    if(m_historyBoard && m_timeline)
    {
        m_historyBoard->setOnJumpCallback([this](int targetIdx){
            if(m_timeline){ m_timeline->seek(targetIdx); m_timeline->pause(); }
        });
        m_historyBoard->syncWithManager(*m_timeline);
        m_historyBoard->setFonts(*m_iconFont, *m_textFont);
    }

    // 6. THÊM SPEED CONTROLLER VÀO DOCK (Cái này cực kỳ quan trọng)
    if(m_timeline&&dock)
    {
        // Dùng font text bình thường cho các con số/chữ của speed
        auto* speedCtrl = new GUI::SpeedController(*m_textFont, m_timeline.get());

        // Thêm một vạch ngăn trước khi cho SpeedController vào cho đẹp
        dock->addItem(new GUI::Separator(Theme::Style::DockHeight));

        // Add vào dock để nó quản lý memory luôn
        dock->addItem(speedCtrl);
    }

    // Đừng quên cập nhật lại layout cho dock để nó tính toán lại vị trí các con
    if(dock)
    {
        dock->updateLayout();
    }

    // Đăng ký callback cho Notch
    auto& notch = GUI::NotchManager::getInstance();

    notch.setOnFileDroppedCallback([this](const std::string& path)
    {
        // Khi Notch báo có file, ta bảo Structure nạp nó
        if(this->currentDS)
        {
            bool success = this->currentDS->loadFromFile(path);

            // Tùy chọn: Nếu nạp file thất bại hoàn toàn (không mở được file)
            if(!success)
            {
                GUI::NotchManager::getInstance().pushNotification(
                    GUI::Scenario::Error, "Load Failed", "Invalid file format", "\xef\x81\xb1"
                );
            }
        }
    });

    notch.pushNotification(GUI::Scenario::Idle, "Import Data", "Drop file or double click here.");

    // Background
    m_bgOriginal = ResourceManager::getInstance().getTexture("assets/textures/macOS Big Sur.png");
    onResize(window.getSize().x, window.getSize().y);
//    std::cerr << "[VisualizerState] Safe Init Completed." << std::endl;
}

void VisualizerState::handleInput(sf::Event& event)
{
    // F11: Fullscreen Toggle
    if(event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F11)
    {
//        titleBar->toggleFullscreen();
        sf::Vector2u size = window.getSize();
        onResize(size.x, size.y);
//        Utils::System::applyLetterboxView(window, event.size.width, event.size.height);
    }

    if(event.type == sf::Event::Resized)
    {
        onResize(event.size.width, event.size.height);
    }

    if(m_btnBack && m_btnBack->handleEvent(event, window)) return;

    if(popover && popover->isOpen())
    {
        if(popover->handleEvent(event, window)) return;
    }

    if(m_historyBoard)
    {
        int targetIdx = m_historyBoard->handleEvent(event, window);

        // TH 1: Nếu click trúng dòng để nhảy Snapshot -> Nuốt event
        if(targetIdx != -1)
        {
            m_timeline->seek(targetIdx);
            m_timeline->pause();
            return;
        }

        // TH 2: NUỐT EVENT CUỘN CHUỘT (Fix lỗi zoom nhầm)
        // Nếu sự kiện là cuộn chuột hoặc nhấn chuột, và chuột đang nằm trên Board
        // thì ta return ngay lập tức, không cho sự kiện đi xuống Dock hay StructurePanel
        if(event.type == sf::Event::MouseWheelScrolled || event.type == sf::Event::MouseButtonPressed)
        {
            if(m_historyBoard->containsMouse(window))
            {
                return;
            }
        }
    }

    if(dock && dock->handleEvent(event, window)) return;
    if(pseudoBox && pseudoBox->handleEvent(event, window))
    {
        if(structurePanel) structurePanel->updateLayout(Utils::System::DESIGN_WIDTH, Utils::System::DESIGN_HEIGHT, pseudoBox->isOpen());
        return;
    }

    // Chỉ khi thoát qua được tất cả các "lớp lọc" ở trên,
    // structurePanel mới nhận được event để Zoom/Pan
    if(structurePanel) structurePanel->handleEvent(event, window);
    GUI::NotchManager::getInstance().handleEvent(event, window);
}

void VisualizerState::update(float dt)
{
//    return;

    if(m_btnBack)
    {
        m_btnBack->update(window, dt);
    }

    if(m_timeline) m_timeline->update(dt);

    if(m_historyBoard&&m_timeline)
    {
        m_historyBoard->update(dt, window, m_timeline->getCurrentIdx());

        int currentCount = m_timeline->getCount();
        if(currentCount != m_lastSnapshotCount)
        {
            m_historyBoard->syncWithManager(*m_timeline);
            m_lastSnapshotCount = currentCount;
        }
    }

    if(currentDS&&structurePanel&&m_timeline)
    {
        structurePanel->update(dt, window);
        structurePanel->syncGraphObjects(m_timeline->getCurrentFrame(), dt);
    }

    // Cập nhật icon Play/Pause dựa trên trạng thái của Timeline
    if(m_btnPlayPause && m_timeline)
    {
        if(m_timeline->isPlaying())
        {
            // Icon đang phát (Play) - bấm vào sẽ gọi pause() ở onClick
            m_btnPlayPause->setText(L"\uE39E");
        }
        else
        {
            // Icon đang dừng (Pause) - bấm vào sẽ gọi play() ở onClick
            m_btnPlayPause->setText(L"\uE3D0");
        }
    }

    if(dock) dock->update(dt, window);
    if(popover) popover->update(dt, window);
    if(pseudoBox) pseudoBox->update(dt);

    GUI::NotchManager::getInstance().updateMousePos(sf::Mouse::getPosition(window), window);
    GUI::NotchManager::getInstance().update(dt, window);
}

void VisualizerState::draw()
{
//    return;
    // Kiểm tra window trước khi vẽ
    if (!window.isOpen()) return;

    window.clear();
//        if (m_historyBoard) window.draw(*m_historyBoard);
//
//        return;

    // Vẽ Background và Panel với Camera View
//    window.setView(m_camera);
    window.draw(sf::Sprite(m_bgOriginal));

    if(m_timeline && m_timeline->getCount() > 0)
    {
        if(structurePanel)
        {
            // QUAN TRỌNG: Render content để chuẩn bị VertexBuffer cho Panel
            structurePanel->renderContent();
        }
    }

    // Chuyển về View mặc định để vẽ UI cố định
//    window.setView(window.getDefaultView());

    if (structurePanel) window.draw(*structurePanel);


    // Kiểm tra nullptr cho từng UI trước khi Draw
    if (m_btnBack)      m_btnBack->draw(window); // Vẽ nút Back riêng
    if (m_historyBoard) window.draw(*m_historyBoard);
    if (pseudoBox)      pseudoBox->draw(window);
    if (dock)           dock->draw(window);
    if (popover)        popover->draw(window);

    window.draw(GUI::NotchManager::getInstance());
}

void VisualizerState::onResize(unsigned int width, unsigned int height)
{
//    Utils::System::updateCustomView(m_camera, width, height);

    float centerX = Utils::System::DESIGN_WIDTH / 2.0f;
    float targetY = Utils::System::DESIGN_HEIGHT - 70.0f - 40.0f;

    if(dock)
    {
        dock->setPosition(centerX, dock->getPosition().y);
        dock->setTargetY(targetY);
    }

    if(notch)
    {
        notch->setX(centerX);
        notch->setTargetY(40.f);
    }

    if(pseudoBox)
    {
        pseudoBox->onResize(width, height);
    }

//    if(structurePanel)
//    {
//        bool isCodeOpen = pseudoBox && pseudoBox->isOpen();
//        structurePanel->updateLayout((float)width, (float)height, isCodeOpen);
//    }

    if(m_historyBoard)
    {
        // Ví dụ: Đặt Board ở góc dưới bên phải, cách lề 30px
        // Vì Origin là (width, height) nên ta đặt pos ở đúng góc đó luôn
        m_historyBoard->setPosition(640.f, targetY + 70.f);
    }
}

void VisualizerState::onEnter() {}
void VisualizerState::onExit() {}
