#include "VisualizerState.h"
#include "ResourceManager.h"
#include "Theme.h"
#include "SpeedController.h"
#include "ViewHandler.h"
#include "drawGlassPane.h"
#include <iostream>

namespace Theme = Utils::Graphics::Theme;

VisualizerState::VisualizerState(sf::RenderWindow& win, std::stack<State*>& st, DataStructure* ds)
    : window(win), states(st), currentDS(ds),
      font(ResourceManager::getInstance().getFont("assets/fonts/font.ttf")) // Sửa lại đường dẫn theo ý bạn
{
    // 1. Khởi tạo Dock
    dock = new GUI::FloatingDock();
    popover = new GUI::InputPopover(font); // Khởi tạo Popover
    notch = new GUI::DynamicIsland(font);
    pseudoBox = new GUI::PseudoCodeBox(ResourceManager::getInstance().getFont("assets/fonts/consolas-regular.ttf"), window.getSize().x, window.getSize().y);

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
    GUI::SpeedController* speedCtrl = new GUI::SpeedController(font);
    // Đừng quên gán callback để thuật toán nhận tốc độ mới
    // speedCtrl->onSpeedChanged = [this](float s) { currentDS->setSpeed(s); };
    dock->addItem(speedCtrl);

    // 5. Thêm vạch ngăn cuối
    dock->addItem(new GUI::Separator(Theme::Style::DockHeight));

    // 6. Nút chức năng phụ (VD: Clear/Reset)
    GUI::Button* btnReset = new GUI::Button(font, "Reset", {Theme::Style::IconButtonSize, Theme::Style::IconButtonSize});
    btnReset->applyPreset(GUI::ButtonPreset::Ghost);
    dock->addItem(btnReset);

    dock -> updateLayout();

    // 2. Nạp Commands từ cấu trúc dữ liệu (Dock sẽ tự động sinh nút và tính size)
    if (currentDS)
    {
//        dock->setCommands(currentDS->getCommands(), font);


    }

    // NỐI DÂY (WIRING): Khi Dock báo có nút bị click -> Mở Popover!
    dock->onCommandClicked = [this](sf::Vector2f btnPos, DS::Command cmd)
    {
        // Chỉ mở Popover nếu nó đang đóng để tránh giật lag
        if (!popover->isOpen())
        {
            popover->open(btnPos, cmd);
        }
    };

    if (currentDS)
    {
        currentDS->bindDynamicIsland(notch);
    }

    // 3. Setup Layout ban đầu
    // Đặt Dock ẩn sâu dưới màn hình 100px để chuẩn bị cho hiệu ứng trượt lên
    dock->snapToY(window.getSize().y + 100.0f);

    // Kéo viên thuốc lên cách viền trên -100px (Giấu đi)
    notch->snapToY(-100.0f);


    m_isDragging = false;
    m_camera.setSize(window.getSize().x, window.getSize().y);
    m_camera.setCenter(window.getSize().x / 2.0f, window.getSize().y / 2.0f);


    // Giả sử bạn đã có sẵn đối tượng ResourceManager tên là 'resMgr'
    // Khởi tạo slider với độ dài track là 200px
    testSlider = new GUI::Slider(200.f);

    // Thiết lập giới hạn: [0.25x -> 2.0x], bước nhảy 0.25
    testSlider -> setRange(0.25f, 2.0f, 0.25f);

    // Đặt giá trị mặc định ban đầu (VD: 1.0x)
    testSlider -> setValue(1.0f);

    // Đặt vị trí ra giữa màn hình để dễ nhìn
    testSlider -> setPosition(sf::Vector2f(400.f, 300.f));

    testSlider -> setIcons(ResourceManager::getInstance().getTexture("assets/textures/tortoise.png"),
                        ResourceManager::getInstance().getTexture("assets/textures/hare.png"));


    // Thêm vào trong VisualizerState::VisualizerState
    m_structuresRenderTexture.create(1400, 1032);
    m_structuresRenderTexture.setSmooth(true); // Giúp node không bị răng cưa khi zoom

    m_structuresFrame = new GUI::Squircle({1400.f, 1032.f}, 30.f, 4.0f);
    m_structuresFrame->setPosition(24.f, 24.f);
    m_structuresFrame->setOutlineThickness(1.5f);
    m_structuresFrame->setOutlineColor(sf::Color(255, 255, 255, 40)); // Viền kính trắng mờ

    // Load tài nguyên mờ (Giả sử ID texture bạn đặt trong Figma)
    m_bgOriginal = ResourceManager::getInstance().getTexture("assets/textures/macOS Big Sur.png");
    m_bgBlurLevel1 = ResourceManager::getInstance().getTexture("assets/textures/macOS Big Sur - Blur 10.png");
    m_bgBlurLevel2 = ResourceManager::getInstance().getTexture("assets/textures/macOS Big Sur - Blur 25.png");
    m_bgBlurLevel3 = ResourceManager::getInstance().getTexture("assets/textures/macOS Big Sur - Blur 50.png");

    // Nạp ảnh mờ cho Dock
    if (dock)
    {
        dock->setBlurTexture(m_bgBlurLevel2);
    }


    // Ra lệnh bay lên đúng vị trí
    onResize(window.getSize().x, window.getSize().y);
}

VisualizerState::~VisualizerState()
{
    if (dock) delete dock;
    if (popover) delete popover; // Nhớ xóa vùng nhớ
    if (currentDS) delete currentDS;
    if (notch) delete notch;
}

void VisualizerState::onResize(unsigned int width, unsigned int height)
{
    // Do bên trong FloatingDock, ta đã setOrigin ở giữa cạnh trên (Top-Center)
    // Nên tọa độ X của Dock chính là Tâm của màn hình
    float centerX = width / 2.0f;

    // Tọa độ Y đích: Cách đáy màn hình 40px
    // Giả sử dockHeight = 70.f (như đã set trong FloatingDock.cpp)
    float dockHeight = 70.0f;
    float targetY = height - dockHeight - 40.0f;

    // Cập nhật vị trí X ngay lập tức
//    dock->setPosition(centerX, dock->getPosition().y);
    dock->setPosition(1400, height / 2.f - 150);

    // Đặt Target Y để lò xo tự trượt đến (Hoặc trượt lên lúc mở, hoặc trượt xuống lúc resize)
//    dock->setTargetY(targetY);
    dock->setTargetY(height / 2.f - 150);

    if (notch)
    {
        notch->setX(centerX);
        notch->setTargetY(40.f);
    }

    if (pseudoBox)
    {
        pseudoBox->onResize(width, height);
    }
}

void VisualizerState::handleInput(sf::Event& event)
{
    if (event.type == sf::Event::Resized)
    {
        sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
        window.setView(sf::View(visibleArea));

        onResize(event.size.width, event.size.height);
    }



    // 1. ZOOM: Phóng to / Thu nhỏ tại tâm chuột
    if(event.type == sf::Event::MouseWheelScrolled)
    {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        sf::FloatRect frameBounds = m_structuresFrame->getGlobalBounds();

        if(Utils::ViewHandler::isMouseInFrame(mousePos, window, frameBounds))
        {
            // Lưu tọa độ World của chuột TRƯỚC khi zoom
            sf::Vector2f mouseWorldBefore = Utils::ViewHandler::mapPixelToWorld(mousePos, window, frameBounds, m_camera);

            float zoomFactor = (event.mouseWheelScroll.delta > 0) ? 0.9f : 1.1f;
            m_camera.zoom(zoomFactor);

            // Lưu tọa độ World của chuột SAU khi zoom
            sf::Vector2f mouseWorldAfter = Utils::ViewHandler::mapPixelToWorld(mousePos, window, frameBounds, m_camera);

            // Dịch chuyển camera để bù đắp sai lệch, giữ điểm dưới chuột đứng yên
            m_camera.move(mouseWorldBefore - mouseWorldAfter);
        }
    }

    // 2. PAN: Kéo thả khung nhìn (Dùng chuột phải/giữa hoặc Space + Chuột trái)
    if(event.type == sf::Event::MouseButtonPressed)
    {
        bool isPanKey = (event.mouseButton.button == sf::Mouse::Right ||
                         event.mouseButton.button == sf::Mouse::Middle ||
                        (event.mouseButton.button == sf::Mouse::Left && sf::Keyboard::isKeyPressed(sf::Keyboard::Space)));

        if(isPanKey)
        {
            m_isDragging = true;
            m_lastMousePos = sf::Mouse::getPosition(window);
        }
    }

    // 3. PAN - Đang di chuyển chuột
    if(event.type == sf::Event::MouseMoved && m_isDragging)
    {
        sf::Vector2i currentMousePos = sf::Mouse::getPosition(window);

        // Tính Delta dựa trên tọa độ World để việc kéo mượt mà bất kể độ Zoom
        sf::Vector2f worldLastPos = window.mapPixelToCoords(m_lastMousePos, m_camera);
        sf::Vector2f worldCurrentPos = window.mapPixelToCoords(currentMousePos, m_camera);

        m_camera.move(worldLastPos - worldCurrentPos);
        m_lastMousePos = currentMousePos;
    }

    // 4. PAN - Kết thúc kéo thả
    if(event.type == sf::Event::MouseButtonReleased)
    {
        m_isDragging = false;
    }




    // Truyền event cho Dock (Dock sẽ tự chia cho các nút bên trong)
    // ƯU TIÊN SỰ KIỆN:
    // Nếu Popover đang mở, nó sẽ chặn luồng sự kiện (Người dùng không bấm được Dock nữa)
    if (popover && popover->isOpen())
    {
        popover->handleEvent(event, window);
    }
    else
    {
        if (dock) dock->handleEvent(event, window);
    }

    // 1. Cho bảng Code tự xử lý sự kiện click chuột của nó (bấm mấu kéo)
    if (pseudoBox)
    {
        pseudoBox->handleEvent(event, window);
    }

    // 2. TEST TẠM: Bấm phím 'C' để gọi toggleState() trượt bảng ra/vào
    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::C)
    {
        if (pseudoBox)
        {
            pseudoBox->toggleState();
        }
    }



    // TEST TẠM: Phím Xuống để chạy code tới dòng tiếp theo

    static int testLine = 0;
    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Down)
    {

        testLine++;
        if (pseudoBox)
        {
            pseudoBox->updateStep(testLine, {
                {"cur", "Node(" + std::to_string(testLine * 10) + ")"},
                {"min_idx", std::to_string(testLine)}
            });
        }
    }

    // TEST TẠM: Phím Lên để lùi code lại
    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Up)
    {
//        static int testLine = 0;
        testLine = std::max(0, testLine - 1);
        if (pseudoBox) pseudoBox->updateStep(testLine, {});
    }


    // Đặt bên trong vòng lặp while (window.pollEvent(event))
    testSlider -> handleEvent(event, window);
}

void VisualizerState::update(float dt)
{
    if (currentDS)
    {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        sf::FloatRect frameBounds = m_structuresFrame->getGlobalBounds();

        // Chuyển đổi tọa độ Screen -> World Space của CTDL
        sf::Vector2f worldMouse = Utils::ViewHandler::mapPixelToWorld(mousePos, window, frameBounds, m_camera);

        currentDS->setMousePosition(worldMouse);
        currentDS->update(dt);
    }

    // Cập nhật vật lý cho Dock
    if (dock) dock->update(dt, window);
    if (popover) popover->update(dt, window); // Update animation của Popover
    if (notch) notch->update(dt);
    if (pseudoBox) pseudoBox->update(dt);

    // Đặt trong hàm update chạy mỗi frame, truyền thêm delta time (dt)
    testSlider -> update(window, dt);
}

void VisualizerState::draw()
{
    window.clear();

    // LỚP 1: Wallpaper gốc
    window.draw(sf::Sprite(m_bgOriginal));

    // LỚP 2: Chuẩn bị nội dung động cho Structures
    m_structuresRenderTexture.clear(sf::Color::Transparent);
    m_structuresRenderTexture.setView(m_camera);
    if(currentDS)
    {
        currentDS->draw(m_structuresRenderTexture);
    }
    m_structuresRenderTexture.display();

    // LỚP 3: Vẽ các thành phần bằng hàm tiện ích

    // Vẽ Khung Structures (Blur nhẹ - Level 1)
    Utils::Graphics::drawGlassPane(window, *m_structuresFrame, m_bgBlurLevel1,
                                  sf::Color(255, 255, 255, 15), // Trắng 6%
//                                  sf::Color(0, 0, 0, 255), // Trắng 6%
                                  &m_structuresRenderTexture.getTexture());

    // Vẽ Khung Code (Blur mạnh - Level 3)
    // Giả sử bạn đã có codeBoxFrame và codeRenderTexture
    if(pseudoBox)
    {
        // Bạn có thể tùy biến màu overlay tối hơn cho Code (Đen 40%)
        // Utils::Graphics::drawGlassPane(window, *codeFrame, m_bgBlurLevel3, sf::Color(0, 0, 0, 100), ...);
        pseudoBox->draw(window);
    }

    // Vẽ Dock (Blur vừa - Level 2)
    if(dock)
    {
        // Dock có thể tự gọi hàm tiện ích này bên trong hàm draw của nó
        dock->draw(window);
    }

    // Vẽ các thành phần nổi trên cùng
    if(notch) notch->draw(window);
    if(popover) popover->draw(window);

//    window.display();
}

void VisualizerState::init() {}
