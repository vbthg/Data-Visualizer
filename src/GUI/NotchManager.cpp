#include "GUI/NotchManager.h"
#include "GUI/NotchContent.h"
#include "FileTray.h"
#include "ViewHandler.h"
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
                    pushNotification(Scenario::FileTray, "Import Data", "Drop a .txt file", "\xea\x8a");
                }
            }
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
        }
    }

    void NotchManager::pushNotification(Scenario type, const sf::String& title, const sf::String& subtitle, const sf::String& iconCode)
    {
//        std::cout << "[DEBUG] Push: " << (int)type << " | Title: " << title << " " << title.size() << "\n";

    // Dòng debug cực kỳ quan trọng
    std::cout << "[NOTCH DEBUG] Pushing Scenario: " << (int)type << " | Title: " << title.toAnsiString() << std::endl;

        if(m_currentScenario == Scenario::FileTray && type != Scenario::FileTray)
        {
            GUI::FileTray* currentTray = dynamic_cast<GUI::FileTray*>(m_notch->getContent());
            if(currentTray)
            {
                currentTray->setScale(0.f);
            }
            // Cần thêm logic delay việc changeContent() ở hàm update() cho đến khi scaleSpring chạm 0
//            return;
        }
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

        if(type == Scenario::FileTray)
        {
//            iconCode = "\uEB32";
//            title = "abc";
//            std::cout << "title = " << title << "\n";
            auto content = std::make_unique<GUI::FileTray>(iconCode, title, subtitle);
//            auto content = std::make_unique<GUI::FileTray>("\uEB32", "abc");
            m_notch->changeContent(std::move(content));
            return;
        }
        if(type == Scenario::Initial)
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
