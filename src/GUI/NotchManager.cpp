#include "GUI/NotchManager.h"
#include "GUI/NotchContent.h"
#include "FileTray.h"
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

    void NotchManager::pushNotification(Scenario type, const std::string& title, const std::string& subtitle, const std::string& iconCode)
    {
        std::cout << "[DEBUG] Push: " << (int)type << " | Title: " << title << std::endl;

        m_currentScenario = type;

        NotchSize targetSize = NotchSize::Compact;
        m_isAutoDismissing = false;
        m_dismissTimer = 0.f;

        switch(type)
        {
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
            auto content = std::make_unique<GUI::FileTray>(iconCode, title);
            m_notch->changeContent(std::move(content));
            return;
        }
        if(type != Scenario::Idle)
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
