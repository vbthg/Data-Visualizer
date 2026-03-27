#include "ProgressBar.h"
#include "Theme.h"
#include <iostream>

namespace GUI
{
    namespace Theme = Utils::Graphics::Theme;

    ProgressBar::ProgressBar(sf::Vector2f size)
        : m_size(size), m_mainColor(sf::Color(100, 255, 100)) {}

    void ProgressBar::setStep(int current, int total, float duration) {
        m_currentStep = current;
        m_totalSteps = total;
        m_stepDuration = duration;
        m_stepTimer = 0.f; // Reset mỗi khi sang step mới
    }

    void ProgressBar::update(float dt) {
        if (m_currentStep > 0 && m_stepTimer < m_stepDuration) {
            m_stepTimer += dt;
        }
    }

    void ProgressBar::setSize(sf::Vector2f size) {
        m_size = size;
    }

    void ProgressBar::draw(sf::RenderTarget& target, sf::RenderStates states) const {
        if(m_totalSteps <= 0)
        {
//            std::cout << "[Debug] ProgressBar: m_totalSteps <= 0\n";
            return;
        }

//        std::cout << "[Debug] ProgressBar: Drawing Step " << m_currentStep << "/" << m_totalSteps << " | Timer: " << m_stepTimer << "\n";

//        if (m_totalSteps <= 0) return;

        float segmentWidth = m_size.x / m_totalSteps;
        float h = 2.f;//m_size.y;

        // 1. Vẽ nền (Background segments)
        m_rect.setSize({m_size.x, h});
        m_rect.setFillColor(sf::Color(255, 255, 255, 30));
//        m_rect.setFillColor(sf::Color::Red);
        m_rect.setPosition(0, 0);
        target.draw(m_rect, states);
//        return;

        // 2. Vẽ các đoạn đã xong (Completed)
        if (m_currentStep > 1) {
            float completedWidth = (m_currentStep - 1) * segmentWidth;
            m_rect.setSize({completedWidth, h});
            m_rect.setFillColor(m_mainColor);
            m_rect.setFillColor(Theme::Color::CompletedBarColor);
            target.draw(m_rect, states);
        }

        // 3. Vẽ đoạn hiện tại (Active)
        float progress = std::min(1.0f, m_stepTimer / m_stepDuration);
        float activeStartX = (m_currentStep - 1) * segmentWidth;
        float activeWidth = progress * segmentWidth;

        m_rect.setPosition(activeStartX, 0);
        m_rect.setSize({activeWidth, h});
        m_rect.setFillColor(Theme::Color::ActiveBarColor);
        target.draw(m_rect, states);

        // 4. Vẽ các vạch chia đoạn (Divider) - Tạo hiệu ứng Segmented xịn xò
        m_rect.setFillColor(Theme::Color::DividerColor); // Vạch đen mờ
        m_rect.setSize({1.0f, h});
        for (int i = 1; i < m_totalSteps; ++i) {
            m_rect.setPosition(i * segmentWidth, 0);
            target.draw(m_rect, states);
        }
    }
}
