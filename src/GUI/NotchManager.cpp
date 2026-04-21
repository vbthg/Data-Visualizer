#include "GUI/NotchManager.h"
#include "GUI/NotchContent.h"
#include "FileTrayContent.h"
#include "ViewHandler.h"
#include "InputContent.h"
#include "FileDropManager.h"
#include "ResourceManager.h"
#include "ProgressBar.h"
#include <iostream>

namespace GUI
{
    NotchManager::NotchManager()
        : m_currentScenario(Scenario::Idle),
          m_currentSizeState(NotchSize::Compact),
          m_dismissTimer(0.f),
          m_dismissTargetTime(0.f),
          m_isAutoDismissing(false)
    {
    }

    void NotchManager::init(sf::Vector2f screenSize)
    {
        m_screenSize = screenSize;
        m_notch = std::make_unique<GUI::Notch>(SIZE_COMPACT);
        m_notch->setRadii(RADII_COMPACT.x, RADII_COMPACT.y);
        m_notch->setFillColor(sf::Color::Black);
        m_notch->setShadow(sf::Color(0, 0, 0, 150), 20.f, sf::Vector2f(0.f, 10.f));
        m_notch->setPosition(m_screenSize.x / 2.f, 0.f);

        applySize(NotchSize::Compact);
    }

    sf::FloatRect GUI::NotchManager::getBounds() const
    {
        if(m_notch)
        {
            sf::FloatRect bounds = m_notch->getGlobalBounds();
            bounds.left -= 30.f;
            bounds.width += 60.f;
            bounds.height += 30.f;
            return bounds;
        }
        return sf::FloatRect();
    }

    void GUI::NotchManager::updateMousePos(sf::Vector2i mousePos, const sf::RenderWindow& window)
    {
        if(!m_notch) return;

        bool isHovered = Utils::ViewHandler::isMouseInFrame(mousePos, window, getBounds());

//        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
//        { // Nhấn Space để in log tránh spam
//            std::cout << "Mouse UI Coords: " << mousePos.x << ", " << mousePos.y << std::endl;
//            std::cout << "Notch Bounds: L:" << getBounds().left << " T:" << getBounds().top
//                      << " W:" << getBounds().width << " H:" << getBounds().height << std::endl;
//        }

        if(m_currentScenario == Scenario::Initial || m_currentScenario == Scenario::FileTray)
        {
            if(isHovered)
            {
                 m_notch->setScaleTarget(1.03f);
            }
            else
            {
                 m_notch->setScaleTarget(1.0f);
            }
        }
    }

    void GUI::NotchManager::handleEvent(const sf::Event& event, const sf::RenderWindow& window)
    {
        if(event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
        {
            sf::Vector2i mousePos(event.mouseButton.x, event.mouseButton.y);
            if(Utils::ViewHandler::isMouseInFrame(mousePos, window, getBounds()))
            {
                if(m_currentScenario == Scenario::Initial)
                {
                    pushNotification(Scenario::Input, "", "", "");
                    return;
                }
            }
        }

        // 1. Logic TỰ ĐỘNG GIÃN NOTCH (Khi đang dragging file)
        if (FileDropManager::isDragging())
        {
            if (m_currentScenario != Scenario::FileTray)
            {
                // Ép Notch hiện FileTray ngay khi file vừa chạm vào cửa sổ
                pushNotification(Scenario::FileTray, "Import Data", "Drop to load file", "\xef\x84\x9e");
            }

            // Cập nhật hiệu ứng Hover Cyan nếu chuột nằm trong Notch
            auto tray = dynamic_cast<FileTrayContent*>(m_notch->getContent());
            if (tray) {
                sf::Vector2f mousePos = FileDropManager::getHoverPosition();
                // Nếu chuột đang đè lên Notch
                tray->setHoverState(getBounds().contains(mousePos));
            }
        }
        // Nếu hết dragging mà không thả file (Drag Leave) -> Quay về trạng thái chờ
        else if (m_currentScenario == Scenario::FileTray && !FileDropManager::hasDroppedFiles())
        {
            // pushNotification(Scenario::Initial, "Import Data", "Click to load file", "\xea\x8a");
        }

        // 2. Logic XỬ LÝ FILE KHI THẢ (Drop)
        if (FileDropManager::hasDroppedFiles())
        {
            auto files = FileDropManager::popDroppedFiles();
            if (!files.empty())
            {
                // Lấy file đầu tiên để xử lý
                std::string path = files[0];

                // Kích hoạt chuỗi hiệu ứng: Processing -> Success
                pushNotification(Scenario::Processing, "Importing...", path, "\xef\x84\x9e");
                m_notch->getProgressBar().setStep(1, 1, 1.5f);
            }
        }

        // 2. QUAN TRỌNG: Chuyển tiếp TẤT CẢ sự kiện vào Content hiện tại
        // (Bao gồm TextEntered, KeyPressed, MouseMove...)
        if (m_notch && m_notch->getContent())
        {
            m_notch->getContent()->handleEvent(event, window);
        }
    }

    void NotchManager::applySize(NotchSize targetSize)
    {
        m_currentSizeState = targetSize;
        switch(targetSize)
        {
            case NotchSize::Compact:
                m_notch->setSize(SIZE_COMPACT);
                m_notch->setRadii(RADII_COMPACT.x, RADII_COMPACT.y);
                break;
            case NotchSize::Standard:
                m_notch->setSize(SIZE_STANDARD);
                m_notch->setRadii(RADII_STANDARD.x, RADII_STANDARD.y);
                break;
            case NotchSize::Expanded:
                m_notch->setSize(SIZE_EXPANDED);
                m_notch->setRadii(RADII_EXPANDED.x, RADII_EXPANDED.y);
                break;
            case NotchSize::Tray:
                m_notch->setSize(SIZE_TRAY);
                m_notch->setRadii(RADII_TRAY.x, RADII_TRAY.y);
                break;
            case NotchSize::Input:
                m_notch->setSize(SIZE_INPUT);
                m_notch->setRadii(RADII_INPUT.x, RADII_INPUT.y);
        }
    }

    void NotchManager::pushNotification(Scenario type, const sf::String& title, const sf::String& subtitle, const sf::String& iconCode)
    {
//        std::cout << "[DEBUG] Push: " << (int)type << " | Title: " << title << " " << title.size() << "\n";

    // Dòng debug cực kỳ quan trọng
    std::cout << "[NOTCH DEBUG] Pushing Scenario: " << (int)type << " | Title: " << title.toAnsiString() << std::endl;

//        if(m_currentScenario == Scenario::FileTray && type != Scenario::FileTray)
//        {
//            GUI::FileTrayContent* currentTray = dynamic_cast<GUI::FileTrayContent*>(m_notch->getContent());
//            if(currentTray)
//            {
//                currentTray->setContentScale(0.f);
//            }
//            // Cần thêm logic delay việc changeContent() ở hàm update() cho đến khi scaleSpring chạm 0
////            return;
//        }
        m_currentScenario = type;

//        if(type == Scenario::FileTray) std::cout << "start PushNoti: title = " << title << "\n";

        NotchSize targetSize = NotchSize::Compact;
        m_isAutoDismissing = true;
        m_dismissTimer = 0.f;

        switch(type)
        {
            case Scenario::Initial:
            case Scenario::Idle:
            case Scenario::FileTray:
            case Scenario::Input:
            case Scenario::AwaitingInput:
                m_isAutoDismissing = false; // CÁC TRẠNG THÁI NÀY KHÔNG ĐƯỢC TỰ ẨN
                break;

            case Scenario::Success:
                m_dismissTargetTime = 2.0f; // Hiện 2 giây rồi ẩn
                break;

            case Scenario::Error:
                m_dismissTargetTime = 4.0f; // Lỗi thì hiện lâu hơn để đọc
                break;

            default:
                m_dismissTargetTime = 3.0f;
                break;
        }


        switch(type)
        {
            case Scenario::Initial:
                targetSize = NotchSize::Standard;
                break;
            case Scenario::Idle:
                targetSize = NotchSize::Compact;
                break;
            case Scenario::Processing:
                targetSize = NotchSize::Standard;
                break;
            case Scenario::Success:
                targetSize = NotchSize::Expanded;
                m_isAutoDismissing = true;
                m_dismissTargetTime = 2.5f;
                break;
            case Scenario::Error:
            case Scenario::Warning:
                targetSize = NotchSize::Expanded;
                m_isAutoDismissing = true;
                m_dismissTargetTime = 3.5f;
                break;
            case Scenario::AwaitingInput:
                targetSize = NotchSize::Expanded;
                break;
            case Scenario::FileTray:
                targetSize = NotchSize::Tray;
                break;
            case Scenario::Input:
                targetSize = NotchSize::Input;
                break;
        }

        applySize(targetSize);

        // --- ĐIỀU KHIỂN NHỊP ĐẬP WAVEFORM ---
        if(type == Scenario::Processing)
        {
            m_notch->setWaveformState(GUI::Waveform::State::Processing);
        }
        else if(type == Scenario::Success || type == Scenario::Error)
        {
            m_notch->setWaveformState(GUI::Waveform::State::Action);
        }
        else
        {
            m_notch->setWaveformState(GUI::Waveform::State::Idle);
        }
        // ------------------------------------

        m_notch -> setScenario(type);

        if (type == Scenario::Input)
        {
            auto content = std::make_unique<GUI::InputContent>(ResourceManager::getInstance().getFont("assets/fonts/SFProText-Regular.ttf"));

            // Khi nhấn Enter
            content->setOnSubmit([this](std::string path) {
                // Kiểm tra file tồn tại ở đây hoặc báo Processing
                this->pushNotification(Scenario::Processing, "Loading...", path, "\xef\x84\x9e");
                m_notch->getProgressBar().setStep(1, 1, 0.8f);
            });

            // Khi nhấn Escape
            content->setOnCancel([this]() {
                this->pushNotification(Scenario::Initial, "Import Data", "Click to load file", "\xea\x8a");
            });

            m_notch->changeContent(std::move(content));
        }
        else if(type == Scenario::FileTray)
        {
            // 1. Tạo Content mới (Dùng font từ ResourceManager)
            auto content = std::make_unique<GUI::FileTrayContent>(ResourceManager::getInstance().getFont("assets/fonts/SFProText-Regular.ttf"));

            // 2. Cài đặt Callback: Khi có file thả vào -> Chuyển sang Processing
            content->setOnFileDropped([this](std::string path) {
                // Kiểm tra xem file có đúng định dạng không nếu muốn
                this->pushNotification(Scenario::Processing, "Importing Data", path, "\xef\x84\x9e");

                // Chạy thanh bar ảo (Cách 1 đã thống nhất)
                this->m_notch->getProgressBar().setStep(1, 1, 1.5f);
            });

            // 3. Đẩy vào Notch để bắt đầu Morphing
            m_notch->changeContent(std::move(content));
            return; // Kết thúc hàm ở đây
        }
        else if(type == Scenario::Initial)
        {
            // Dùng icon dấu + (mã L"\xea\x8a" nếu đổi sang sf::String)
//            std::cout << "Made a Initial noti here!\n";
            auto content = std::make_unique<GUI::NotchContent>("", "Import Data", "Click to load file");
            m_notch->changeContent(std::move(content));
        }
        else if(type != Scenario::Idle)
        {
            auto content = std::make_unique<GUI::NotchContent>(iconCode, title, subtitle);
            m_notch->changeContent(std::move(content));
        }
        else
        {
            auto content = std::make_unique<GUI::NotchContent>("", "", "");
            m_notch->changeContent(std::move(content));
        }
    }

    void NotchManager::update(float dt)
    {
        if(FileDropManager::isDragging() && m_currentScenario != Scenario::FileTray)
        {
            pushNotification(Scenario::FileTray, "Drop File", "Release to import", "\xef\x84\x9e");
        }

        if(m_isAutoDismissing)
        {
            m_dismissTimer += dt;
//            if(m_dismissTimer >= m_dismissTargetTime)
            if(m_dismissTimer >= m_dismissTargetTime)
            {
                pushNotification(Scenario::Idle);
            }
        }

        if(m_notch)
        {
            m_notch->update(dt);

            // Tự động chuyển cảnh khi nạp xong dữ liệu
            if (m_currentScenario == Scenario::Processing)
            {
                if (m_notch->getProgressBar().isFinished())
                {
                    // Đổi sang Success với Tick xanh (mã icon \xef\x80\x8c)
                    pushNotification(Scenario::Success, "Imported", "Ready to visualize", "\xef\x80\x8c");
                }
            }
        }
    }

    void NotchManager::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        if(m_notch)
        {
            target.draw(*m_notch, states);
        }
    }
}
