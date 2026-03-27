#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "GUI/Squircle.h"
#include "Spring.h"

namespace GUI
{
    class Waveform : public sf::Drawable, public sf::Transformable
    {
    public:
        enum class State
        {
            Idle,       // Nhịp thở nhẹ (Sine wave)
            Processing, // Nhảy ngẫu nhiên (Thuật toán đang chạy)
            Action      // Bùng nổ (Spike - Khi có sự kiện quan trọng)
        };

    public:
        Waveform();

        void setState(State state);
        void update(float dt);
        void setAlpha(float alpha); // Để mờ dần khi Cross-fade

    protected:
        virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    private:
        State m_state;
        std::vector<GUI::Squircle> m_bars;
        std::vector<Utils::Physics::Spring> m_springs;

        float m_timer;    // Dùng để tạo nhịp nhảy ngẫu nhiên
        float m_totalTime; // Dùng cho hàm Sin (Idle)
        float m_alpha;
    };
}
