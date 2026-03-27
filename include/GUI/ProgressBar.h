#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

namespace GUI {
    class ProgressBar : public sf::Drawable {
    public:
        // Trong ProgressBar.h
        ProgressBar() = default;
        ProgressBar(sf::Vector2f size);

        void setStep(int current, int total, float duration);
        void update(float dt);
        void setSize(sf::Vector2f size);
        void setFillColor(sf::Color color);

        sf::Vector2f getSize() const { return m_size; }

    protected:
        virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    private:
        sf::Vector2f m_size;
        int m_currentStep = 0;
        int m_totalSteps = 0;
        float m_stepTimer = 0.f;
        float m_stepDuration = 1.0f;

        sf::Color m_mainColor;
        mutable sf::RectangleShape m_rect; // Dùng chung để tiết kiệm bộ nhớ
    };
}
