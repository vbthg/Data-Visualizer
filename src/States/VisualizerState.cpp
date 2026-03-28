#include "VisualizerState.h"
#include "ResourceManager.h"
#include "Theme.h"
#include "SpeedController.h"
#include "ViewHandler.h"
#include "WindowConfig.h"
#include "NotchManager.h"
#include "FileDropManager.h"
//#include "drawGlassPane.h"
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
        dock->setCommands(currentDS->getCommands(), font);


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

    speedCtrl = new GUI::SpeedController(font);
    // Đừng quên gán callback để thuật toán nhận tốc độ mới
    // speedCtrl->onSpeedChanged = [this](float s) { currentDS->setSpeed(s); };
    dock->addItem(speedCtrl);

    // 5. Thêm vạch ngăn cuối
    dock->addItem(new GUI::Separator(Theme::Style::DockHeight));

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
//    m_camera.setSize(window.getSize().x, window.getSize().y);
//    m_camera.setCenter(window.getSize().x / 2.0f, window.getSize().y / 2.0f);

    m_camera.setSize(Utils::System::DESIGN_WIDTH, Utils::System::DESIGN_HEIGHT);
    m_camera.setCenter(Utils::System::DESIGN_WIDTH / 2.f, Utils::System::DESIGN_HEIGHT / 2.f);


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


    structurePanel = new GUI::StructurePanel();
    bool isCodeOpen = pseudoBox ? pseudoBox->isOpen() : false;
    structurePanel->initIntro(1920.f, 1080.f, isCodeOpen);



    // Load tài nguyên mờ (Giả sử ID texture bạn đặt trong Figma)
    m_bgOriginal = ResourceManager::getInstance().getTexture("assets/textures/macOS Big Sur.png");

    // Nạp ảnh mờ cho Dock
    if (dock)
    {
        dock->setBlurTexture(ResourceManager::getInstance().getTexture("assets/textures/macOS Big Sur - Blur 25.png"), sf::Vector2f(1920.f, 1080.f));
    }



    GUI::NotchManager::getInstance().pushNotification(GUI::Scenario::Initial);


    // Ra lệnh bay lên đúng vị trí
    onResize(window.getSize().x, window.getSize().y);
}

VisualizerState::~VisualizerState()
{
    if (dock) delete dock;
    if (popover) delete popover; // Nhớ xóa vùng nhớ
    if (currentDS) delete currentDS;
    if (notch) delete notch;
    if(structurePanel) delete structurePanel;
}

void VisualizerState::onEnter()
{
    GUI::NotchManager::getInstance().pushNotification(GUI::Scenario::Idle);
//    m_wasDragging = false;
}

void VisualizerState::onExit()
{
    GUI::NotchManager::getInstance().pushNotification(GUI::Scenario::Idle);
}

//void VisualizerState::onResize(unsigned int width, unsigned int height)
//{
//    Utils::System::updateCustomView(m_camera, width, height);
//
//    // Do bên trong FloatingDock, ta đã setOrigin ở giữa cạnh trên (Top-Center)
//    // Nên tọa độ X của Dock chính là Tâm của màn hình
//    float centerX = width / 2.0f;
//
//    // Tọa độ Y đích: Cách đáy màn hình 40px
//    // Giả sử dockHeight = 70.f (như đã set trong FloatingDock.cpp)
//    float dockHeight = 70.0f;
//    float targetY = height - dockHeight - 40.0f;
//
//    // Cập nhật vị trí X ngay lập tức
//    dock->setPosition(centerX, dock->getPosition().y);
////    dock->setPosition(1400, height / 2.f - 150);
//
//    // Đặt Target Y để lò xo tự trượt đến (Hoặc trượt lên lúc mở, hoặc trượt xuống lúc resize)
//    dock->setTargetY(targetY);
////    dock->setTargetY(height / 2.f - 150);
//
//    if (notch)
//    {
//        notch->setX(centerX);
//        notch->setTargetY(40.f);
//    }
//
//    if (pseudoBox)
//    {
//        pseudoBox->onResize(width, height);
//    }
//
////    updateLayoutTargets(event.size.width, event.size.height);
//}

void VisualizerState::onResize(unsigned int width, unsigned int height)
{
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
        pseudoBox->onResize(Utils::System::DESIGN_WIDTH, Utils::System::DESIGN_HEIGHT);
    }
}

void VisualizerState::handleInput(sf::Event& event)
{
    if(event.type == sf::Event::Resized)
    {
        // KHÔNG set lại window.setView() ở đây nữa vì Application.cpp đã lo việc giữ tỉ lệ (letterbox)
        // Chỉ gọi onResize để cập nhật lại các thành phần UI (nếu cần)
        Utils::System::updateCustomView(m_camera, event.size.width, event.size.height);
//        onResize(event.size.width, event.size.height);
    }

    // 1. ZOOM: Phóng to / Thu nhỏ tại tâm chuột
    if(event.type == sf::Event::MouseWheelScrolled)
    {
        sf::Vector2i mousePos(event.mouseWheelScroll.x, event.mouseWheelScroll.y);
        sf::FloatRect frameBounds = structurePanel->getGlobalBounds();

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
            m_lastMousePos = sf::Vector2i(event.mouseButton.x, event.mouseButton.y);
        }
    }

    // 3. PAN - Đang di chuyển chuột
    if(event.type == sf::Event::MouseMoved && m_isDragging)
    {
        sf::Vector2i currentMousePos(event.mouseMove.x, event.mouseMove.y);

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

    // ƯU TIÊN SỰ KIỆN CỦA UI
    if(popover && popover->isOpen())
    {
        popover->handleEvent(event, window);
    }
    else
    {
        if(dock) dock->handleEvent(event, window);
    }

    if(pseudoBox)
    {
        pseudoBox->handleEvent(event, window);
    }

    // TEST TẠM: Bấm phím 'C'
    if(event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::C)
    {
        if(pseudoBox)
        {
            pseudoBox->toggleState();
            bool isCodeOpen = pseudoBox ? pseudoBox->isOpen() : false;
            structurePanel->updateLayout(1920.f, 1080.f, isCodeOpen);
        }
    }

    // TEST TẠM: Phím Xuống
    static int testLine = 0;
    if(event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Down)
    {
        testLine++;
        if(pseudoBox)
        {
            pseudoBox->updateStep(testLine, {
                {"cur", "Node(" + std::to_string(testLine * 10) + ")"},
                {"min_idx", std::to_string(testLine)}
            });
        }
    }

    // TEST TẠM: Phím Lên
    if(event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Up)
    {
        testLine = std::max(0, testLine - 1);
        if(pseudoBox) pseudoBox->updateStep(testLine, {});
    }

    testSlider->handleEvent(event, window);
    structurePanel->handleEvent(event, window);
}

/*

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
        sf::FloatRect frameBounds = structurePanel->getGlobalBounds();

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
            // Cập nhật lại Target để lò xo giãn ra / co vào
            bool isCodeOpen = pseudoBox ? pseudoBox->isOpen() : false;
            structurePanel->updateLayout(1920.f, 1080.f, isCodeOpen);
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

    structurePanel->handleEvent(event, window);
}

*/

void VisualizerState::update(float dt)
{
    static bool wasDragging = false;
    bool isDragging = FileDropManager::isDragging();

    std::cout << wasDragging << " " << isDragging << "\n";

    if (isDragging && !wasDragging)
    {
        GUI::NotchManager::getInstance().pushNotification(GUI::Scenario::FileTray, "Drop file to load", "", "\xef\x84\x9e");
    }
    else if (!isDragging && wasDragging)
    {
        if (FileDropManager::hasDroppedFiles())
        {
            std::vector<std::string> files = FileDropManager::popDroppedFiles();
            if (!files.empty())
            {
                GUI::NotchManager::getInstance().pushNotification(GUI::Scenario::Processing, "Loading Data...", files[0], "\xef\x84\x9e");
                // loadData(files[0]);
            }
        }
        else
        {
            GUI::NotchManager::getInstance().pushNotification(GUI::Scenario::Idle);
        }
    }
    wasDragging = isDragging;

    GUI::NotchManager::getInstance().update(dt);
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    // PHẢI GỌI DÒNG NÀY
    GUI::NotchManager::getInstance().updateMousePos(mousePos, window);


    if (currentDS)
    {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        sf::FloatRect frameBounds = structurePanel->getGlobalBounds();;

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
    if(structurePanel)
    {
        structurePanel->update(dt, window),
        // 3. THỰC HIỆN RENDER NỘI DUNG VÀO BUFFER
        // Hàm này sẽ gọi ds->draw(m_contentBuffer) bên trong StructurePanel
        structurePanel->renderContent(currentDS);
    }

    // Đặt trong hàm update chạy mỗi frame, truyền thêm delta time (dt)
    testSlider -> update(window, dt);
}

void VisualizerState::draw()
{
    window.clear();

    // LỚP 1: Wallpaper gốc
    window.draw(sf::Sprite(m_bgOriginal));


    if(structurePanel) window.draw(*structurePanel);

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
//    if(notch) notch->draw(window);
    if(popover) popover->draw(window);
    window.draw(GUI::NotchManager::getInstance());


//    window.display();
}

void VisualizerState::init() {}
