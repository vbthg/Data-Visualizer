#include "Waveform.h"
#include <cmath>

namespace GUI
{
    Waveform::Waveform()
        : m_state(State::Idle), m_timer(0.0f), m_totalTime(0.0f), m_alpha(1.0f)
    {
        for(int i = 0; i < 4; i++)
        {
            // Khởi tạo 4 cột Squircle (power = 2 thành capsule)
            m_bars.emplace_back(sf::Vector2f(3.f, 15.f));
            m_bars.back().setPower(2.0f);
            m_bars.back().setFillColor(sf::Color::White);
            m_bars.back().setRadius(1.5f);

            // Khởi tạo Spring cho chiều cao
            Utils::Physics::Spring s;
            s.stiffness = 400.f; // Nhảy nhanh nhưng mượt
            s.damping = 25.f;
            s.snapTo(4.0f); // Chiều cao tối thiểu ban đầu
            m_springs.push_back(s);
        }
    }

    void Waveform::update(float dt)
    {
        m_totalTime += dt;
        m_timer += dt;

        for(size_t i = 0; i < m_springs.size(); i++)
        {
            if(m_state == State::Idle)
            {
                // Nhịp thở Sine: nhô lên nhẹ nhàng 4px -> 10px
                m_springs[i].target = 16.0f + std::sin(m_totalTime * 3.0f + i) * 3.0f;
            }
            else if(m_state == State::Processing)
            {
                // Cứ mỗi 0.1s đổi target ngẫu nhiên
                if(m_timer > 0.1f)
                {
                    m_springs[i].target = 4.0f + (std::rand() % 16);
                }
            }
            else if(m_state == State::Action)
            {
                // Spike mạnh rồi tự về Idle/Processing
                m_springs[i].target = (i == 1 || i == 2) ? 18.f : 10.f;
            }

            m_springs[i].update(dt);
            m_bars[i].setSize(sf::Vector2f(3.f, m_springs[i].position));

            // Căn lề các cột: nằm ngang cạnh nhau
            m_bars[i].setPosition(i * 7.0f, 0.f);
            // Căn giữa theo trục Y của cột (origin ở giữa đáy)
            m_bars[i].setOrigin(1.5f, m_springs[i].position / 2.0f);
        }

        if(m_timer > 0.1f)
        {
            m_timer = 0.0f;
        }
    }

    void Waveform::setAlpha(float alpha)
    {
        m_alpha = alpha;
        sf::Color c = sf::Color::White;
        c.a = (sf::Uint8)(alpha * 255.f);
        for(auto& bar : m_bars)
        {
            bar.setFillColor(c);
        }
    }

    void Waveform::setState(State state)
    {
        m_state = state;
    }

    void Waveform::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        states.transform *= getTransform();
        for(const auto& bar : m_bars)
        {
            target.draw(bar, states);
        }
    }
}
