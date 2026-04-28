#include "ProgressBar.h"
#include "Theme.h"
#include <iostream>
#include <algorithm>

namespace GUI
{
    namespace Theme = Utils::Graphics::Theme;

    ProgressBar::ProgressBar(sf::Vector2f size)
        : m_size(size), m_mainColor(sf::Color(100, 255, 100))
    {}

    void ProgressBar::setStep(int current, int total, float duration)
    {
        m_currentStep = current;
        m_totalSteps = total;
        m_stepDuration = duration;
        m_stepTimer = 0.f;
    }

    void ProgressBar::setPercent(float p)
    {
        if(m_totalSteps <= 0)
        {
            m_currentStep = 1;
            m_stepTimer = (p >= 1.0f) ? m_stepDuration : 0.f;
            return;
        }

        float clampedP = std::max(0.0f, std::min(1.0f, p));

        // globalPosition chạy từ 0.0 đến m_totalSteps (tổng số đoạn segment)
        float globalPosition = clampedP * static_cast<float>(m_totalSteps);

        // Xác định chúng ta đang ở segment thứ mấy (1-indexed)
        m_currentStep = static_cast<int>(globalPosition) + 1;

        // Xử lý biên: Nếu p = 1.0, m_currentStep sẽ là m_totalSteps + 1 -> phải ép về m_totalSteps
        if(m_currentStep > m_totalSteps)
        {
            m_currentStep = m_totalSteps;
        }

        // Tính toán tiến trình nội bộ của segment hiện tại
        float localAlpha = 0.f;
        if(clampedP >= 1.0f)
        {
            localAlpha = 1.0f; // Full đoạn cuối
        }
        else
        {
            localAlpha = globalPosition - static_cast<float>(m_currentStep - 1);
        }

        m_stepTimer = localAlpha * m_stepDuration;
    }

    void ProgressBar::update(float dt)
    {
        if(m_currentStep > 0 && m_stepTimer < m_stepDuration)
        {
            // Update logic if needed
        }
    }

    void ProgressBar::setSize(sf::Vector2f size)
    {
        m_size = size;
    }

    bool ProgressBar::isFinished() const
    {
        return (m_currentStep >= m_totalSteps) && (m_stepTimer >= m_stepDuration);
    }

    void ProgressBar::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        if(m_totalSteps <= 0)
        {
            return;
        }

        float segmentWidth = m_size.x / m_totalSteps;
        float h = 2.f;

        // 1. Draw Background
        m_rect.setSize({m_size.x, h});
        m_rect.setFillColor(sf::Color(255, 255, 255, 30));
        m_rect.setPosition(0, 0);
        target.draw(m_rect, states);

        // 2. Draw Completed Segments
        if(m_currentStep > 1)
        {
            float completedWidth = (m_currentStep - 1) * segmentWidth;
            m_rect.setSize({completedWidth, h});
            m_rect.setFillColor(Theme::Color::CompletedBarColor);
            m_rect.setPosition(0, 0);
            target.draw(m_rect, states);
        }

        // 3. Draw Active Segment
        float progress = std::min(1.0f, m_stepTimer / m_stepDuration);
        float activeStartX = (m_currentStep - 1) * segmentWidth;
        float activeWidth = progress * segmentWidth;

        m_rect.setPosition(activeStartX, 0);
        m_rect.setSize({activeWidth, h});
        m_rect.setFillColor(Theme::Color::ActiveBarColor);
        target.draw(m_rect, states);

        // 4. Draw Dividers
        m_rect.setFillColor(Theme::Color::DividerColor);
        m_rect.setSize({1.0f, h});
        for(int i = 1; i < m_totalSteps; ++i)
        {
            m_rect.setPosition(i * segmentWidth, 0);
            target.draw(m_rect, states);
        }
    }
}
